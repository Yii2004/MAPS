#include <cassert>
#include <iostream>

#include "system/loader.h"
#include "system/machine.h"

using namespace maps_sim;

namespace {
    constexpr UINT32 C_ADDR = 0x00002020;
}

int main() {
    Machine machine;

    UINT32 entry = 0;
    assert(ElfLoader::load(machine.bus(), MAPS_TENSORC_EXAMPLE_ELF, entry));

    machine.reset(entry);
    assert(machine.run(200000));
    assert(machine.cpu().halted());
    assert(!machine.cpu().faulted());
    assert(machine.exit_code() == 0);
    assert(machine.console_output() == "maps-os boot\ntensorc npu ok\nmaps-os done\n");

    UINT32 c[4] = {};
    for (UINT32 i = 0; i < 4; ++i) {
        assert(machine.bus().read32(C_ADDR + i * sizeof(UINT32), c[i]));
    }
    assert(c[0] == 19);
    assert(c[1] == 22);
    assert(c[2] == 43);
    assert(c[3] == 50);

    std::cout << "test_tensorc_os_example: PASS\n";
    return 0;
}
