#include "npu/schedule.h"

namespace maps_sim {

    bool Scheduler::build_plan(const Descriptor& desc, std::vector<GemmTile>& out_plan) const {
        out_plan.clear();

        if (!desc.validate()) {
            return false;
        }

        const UINT32 m = desc.matrix_m();
        const UINT32 n = desc.matrix_n();
        const UINT32 k = desc.matrix_k();
        const UINT32 tm = desc.tile_m();
        const UINT32 tn = desc.tile_n();
        const UINT32 tk = desc.tile_k();

        for (UINT32 m0 = 0; m0 < m; m0 += tm) {
            const UINT32 m_len = (m0 + tm <= m) ? tm : (m - m0);
            for (UINT32 n0 = 0; n0 < n; n0 += tn) {
                const UINT32 n_len = (n0 + tn <= n) ? tn : (n - n0);
                for (UINT32 k0 = 0; k0 < k; k0 += tk) {
                    const UINT32 k_len = (k0 + tk <= k) ? tk : (k - k0);
                    out_plan.push_back({m0, n0, k0, m_len, n_len, k_len});
                }
            }
        }

        return true;
    }
}

