#include <cassert>
#include <iostream>
#include <vector>

#include "common/memory.h"
#include "npu/controller.h"

using namespace maps_sim;

int main() {
    Controller ctrl;
    Memory mem;
    ctrl.bind_memory(&mem);

    Descriptor desc;
    desc.set_matrix_mnk(2, 2, 2);
    desc.set_matrix_base(256, 512, 768);
    desc.set_dataflow(dataflow::WS);
    desc.set_tile(2, 2, 2);
    assert(ctrl.submit(desc));

    INT32 a[4] = {1, 2, 3, 4};
    INT32 b[4] = {5, 6, 7, 8};
    assert(ctrl.load_input_data(a, 4));
    assert(ctrl.load_weight_data(b, 4));
    assert(ctrl.run());

    std::vector<INT32> out(4, 0);
    assert(ctrl.read_output_data(out.data(), static_cast<UINT32>(out.size())));
    assert(out[0] == 19);
    assert(out[1] == 22);
    assert(out[2] == 43);
    assert(out[3] == 50);

    std::cout << "test_controller_readback_dma: PASS\n";
    return 0;
}

