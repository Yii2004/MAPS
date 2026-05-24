#include <cassert>
#include <iostream>
#include "npu/buffer.h"

using namespace maps_sim;

static void test_read_write_and_bounds() {
    Buffer buf(8);
    INT32 out = 0;

    assert(buf.write(0, 11));
    assert(buf.write(7, 22));
    assert(!buf.write(8, 33));

    assert(buf.read(0, out));
    assert(out == 11);
    assert(buf.read(7, out));
    assert(out == 22);
    assert(!buf.read(8, out));
}

static void test_reset() {
    Buffer buf(4);
    INT32 out = 0;

    assert(buf.write(0, 5));
    assert(buf.write(1, 9));
    buf.reset();

    assert(buf.read(0, out) && out == 0);
    assert(buf.read(1, out) && out == 0);
    assert(buf.read_count() == 2);
    assert(buf.write_count() == 0);
}

static void test_load_dump_and_stats() {
    Buffer buf(6);
    INT32 src[4] = {1, 2, 3, 4};
    INT32 dst[4] = {0, 0, 0, 0};

    assert(buf.load(src, 4));
    assert(!buf.load(src, 7));
    assert(buf.dump(dst, 4));

    assert(dst[0] == 1);
    assert(dst[1] == 2);
    assert(dst[2] == 3);
    assert(dst[3] == 4);

    assert(buf.write_count() == 4);
    assert(buf.read_count() == 4);
}

int main() {
    test_read_write_and_bounds();
    test_reset();
    test_load_dump_and_stats();
    std::cout << "test_buffer: PASS\n";
    return 0;
}

