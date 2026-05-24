#include <cassert>
#include <iostream>

#include "common/memory.h"
#include "npu/dma.h"
#include "npu/buffer.h"

using namespace maps_sim;

static void test_dram_to_buffer_ok() {
    Memory mem(16);
    INT32 init[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    Buffer buf(16);
    DmaEngine dma;

    assert(mem.load(0, init, 16));
    dma.bind_memory(&mem);
    dma.bind_buffer(&buf);

    DmaRequest req{DmaDirection::DRAM_TO_BUFFER, 4 * sizeof(INT32), 2, 5, 1, 5, 5};
    assert(dma.submit(req));
    assert(dma.run_sync());
    assert(dma.state() == DmaState::DONE);
    assert(dma.error() == DmaError::NONE);

    INT32 out = 0;
    assert(buf.read(2, out) && out == 4);
    assert(buf.read(3, out) && out == 5);
    assert(buf.read(4, out) && out == 6);
    assert(buf.read(5, out) && out == 7);
    assert(buf.read(6, out) && out == 8);

    const DmaStats st = dma.stats();
    assert(st.transfer_count == 1);
    assert(st.bytes_moved == 5 * sizeof(INT32));
    assert(st.cycle_count == 5);
}

static void test_buffer_to_dram_ok() {
    Memory mem(16);
    Buffer buf(16);
    DmaEngine dma;

    assert(buf.write(1, 21));
    assert(buf.write(2, 22));
    assert(buf.write(3, 23));

    dma.bind_memory(&mem);
    dma.bind_buffer(&buf);

    DmaRequest req{DmaDirection::BUFFER_TO_DRAM, 8 * sizeof(INT32), 1, 3, 1, 3, 3};
    assert(dma.submit(req));
    assert(dma.run_sync());
    assert(dma.state() == DmaState::DONE);
    assert(dma.error() == DmaError::NONE);

    INT32 out = 0;
    assert(mem.read(8, out) && out == 21);
    assert(mem.read(9, out) && out == 22);
    assert(mem.read(10, out) && out == 23);
}

static void test_out_of_range_fail() {
    Memory mem(8);
    Buffer buf(8);
    DmaEngine dma;

    dma.bind_memory(&mem);
    dma.bind_buffer(&buf);

    DmaRequest req{DmaDirection::DRAM_TO_BUFFER, 6 * sizeof(INT32), 0, 4, 1, 4, 4}; // 6+4 > 8
    assert(!dma.submit(req));
    assert(dma.state() == DmaState::ERROR);
    assert(dma.error() == DmaError::OUT_OF_RANGE);
}

static void test_null_ptr_fail() {
    Memory mem(8);
    Buffer buf(8);
    DmaEngine dma1;

    // only bind buffer, not DRAM
    dma1.bind_buffer(&buf);
    DmaRequest req{DmaDirection::DRAM_TO_BUFFER, 0, 0, 1, 1, 1, 1};
    assert(!dma1.submit(req));
    assert(dma1.state() == DmaState::ERROR);
    assert(dma1.error() == DmaError::NULL_PTR);

    DmaEngine dma2;
    dma2.bind_memory(&mem);
    // no buffer bind
    DmaRequest req2{DmaDirection::DRAM_TO_BUFFER, 0, 0, 1, 1, 1, 1};
    assert(!dma2.submit(req2));
    assert(dma2.state() == DmaState::ERROR);
    assert(dma2.error() == DmaError::NULL_PTR);
}

static void test_zero_length_fail() {
    Memory mem(8);
    Buffer buf(8);
    DmaEngine dma;

    dma.bind_memory(&mem);
    dma.bind_buffer(&buf);

    DmaRequest req{DmaDirection::DRAM_TO_BUFFER, 0, 0, 0, 1, 1, 1};
    assert(!dma.submit(req));
    assert(dma.state() == DmaState::ERROR);
    assert(dma.error() == DmaError::INVALID_PARAM);
}

static void test_2d_stride_transfer() {
    Memory mem(32);
    Buffer buf(32);
    DmaEngine dma;
    dma.bind_memory(&mem);
    dma.bind_buffer(&buf);

    // Build a 4x4 row-major matrix in memory starting at base 0.
    // Row r, col c => 10*r + c
    for (UINT32 r = 0; r < 4; ++r) {
        for (UINT32 c = 0; c < 4; ++c) {
            assert(mem.write(r * 4 + c, static_cast<INT32>(10 * r + c)));
        }
    }

    // Copy a 2x3 rectangle from memory rows [1..2], cols [1..3]
    // src base = 1*4 + 1 = 5, src stride = 4, dst stride = 3
    DmaRequest req_in{DmaDirection::DRAM_TO_BUFFER, 5 * sizeof(INT32), 0, 3, 2, 4, 3};
    assert(dma.submit(req_in));
    assert(dma.run_sync());

    INT32 v = 0;
    assert(buf.read(0, v) && v == 11);
    assert(buf.read(1, v) && v == 12);
    assert(buf.read(2, v) && v == 13);
    assert(buf.read(3, v) && v == 21);
    assert(buf.read(4, v) && v == 22);
    assert(buf.read(5, v) && v == 23);
}

int main() {
    test_dram_to_buffer_ok();
    test_buffer_to_dram_ok();
    test_out_of_range_fail();
    test_null_ptr_fail();
    test_zero_length_fail();
    test_2d_stride_transfer();

    std::cout << "test_dma: PASS\n";
    return 0;
}
