#ifndef MAPS_SIM_NPU_PE_H
#define MAPS_SIM_NPU_PE_H

#include <cstdint>
#include "common/types.h"
#include "common/config.h"


namespace maps_sim {

    class PE {
        public:
            PE(); 

            void reset();
            void set_mode(dataflow mode);

            void load_weight(INT32 weight);
            void load_input(INT32 input);
            void load_psum(INT32 psum);

            void set_inputs(INT32 lhs, INT32 rhs);
            void tick();

            INT32 output0() const;
            INT32 output1() const;
            INT32 psum() const;

        private:
            INT32 input0_;
            INT32 input1_;

            INT32 output0_;
            INT32 output1_;

            INT32 weight_reg_;
            INT32 input_reg_;
            INT32 psum_reg_;

            dataflow current_mode_;

    };


} // namespace maps_sim

#endif // MAPS_SIM_NPU_PE_H
