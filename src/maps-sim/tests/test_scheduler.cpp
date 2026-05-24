#include <cassert>
#include <iostream>
#include <vector>

#include "npu/descriptor.h"
#include "npu/schedule.h"

using namespace maps_sim;

static void test_scheduler_even_tiles() {
    Descriptor desc;
    desc.set_matrix_mnk(8, 8, 8);
    desc.set_matrix_base(4, 8, 12);
    desc.set_tile(4, 4, 4);

    Scheduler sched;
    std::vector<GemmTile> plan;

    assert(sched.build_plan(desc, plan));
    // (8/4) * (8/4) * (8/4) = 8
    assert(plan.size() == 8);

    for (const auto& t : plan) {
        assert(t.m_len == 4);
        assert(t.n_len == 4);
        assert(t.k_len == 4);
    }
}

static void test_scheduler_tail_tiles() {
    Descriptor desc;
    desc.set_matrix_mnk(10, 9, 7);
    desc.set_matrix_base(4, 8, 12);
    desc.set_tile(4, 4, 4);

    Scheduler sched;
    std::vector<GemmTile> plan;

    assert(sched.build_plan(desc, plan));
    // ceil(10/4) * ceil(9/4) * ceil(7/4) = 3 * 3 * 2 = 18
    assert(plan.size() == 18);

    const GemmTile& last = plan.back();
    // Last tile starts at (8,8,4), so tail sizes are (2,1,3)
    assert(last.m0 == 8);
    assert(last.n0 == 8);
    assert(last.k0 == 4);
    assert(last.m_len == 2);
    assert(last.n_len == 1);
    assert(last.k_len == 3);
}

static void test_scheduler_invalid_desc() {
    Descriptor invalid_desc; // defaults invalid
    Scheduler sched;
    std::vector<GemmTile> plan;

    assert(!sched.build_plan(invalid_desc, plan));
    assert(plan.empty());
}

int main() {
    test_scheduler_even_tiles();
    test_scheduler_tail_tiles();
    test_scheduler_invalid_desc();

    std::cout << "test_scheduler: PASS\n";
    return 0;
}
