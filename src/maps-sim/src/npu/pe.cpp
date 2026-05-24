#include "npu/pe.h"


namespace maps_sim {

    PE::PE()
        : input0_(0),
          input1_(0),
          output0_(0),
          output1_(0),
          weight_reg_(0),
          input_reg_(0),
          psum_reg_(0),
          current_mode_(dataflow::OS) {}

    void PE::reset() {
        input0_ = 0;
        input1_ = 0;
        output0_ = 0;
        output1_ = 0;
        weight_reg_ = 0;
        input_reg_ = 0;
        psum_reg_ = 0;
    }

    void PE::set_mode(dataflow mode) {
        current_mode_ = mode;
    }

    void PE::load_weight(INT32 weight) {
        weight_reg_ = weight;
    }

    void PE::load_input(INT32 input) {
        input_reg_ = input;
    }

    void PE::load_psum(INT32 psum) {
        psum_reg_ = psum;
    }

    void PE::set_inputs(INT32 lhs, INT32 rhs) {
        input0_ = lhs;
        input1_ = rhs;
    }

    INT32 PE::output0() const {
        return output0_;
    }

    INT32 PE::output1() const {
        return output1_;
    }

    INT32 PE::psum() const {
        return psum_reg_;
    }

    void PE::tick() {
        switch(current_mode_) {
            case dataflow::OS:
                psum_reg_ += input0_ * input1_;
                output0_ = input0_;
                output1_ = input1_;
                break;

            case dataflow::WS:
                psum_reg_ = input0_ * weight_reg_ + input1_;
                output0_ = input0_;
                output1_ = psum_reg_;
                break;

            case dataflow::IS:
                psum_reg_ = input0_ + input_reg_ * input1_;
                output0_ = psum_reg_;
                output1_ = input1_;
                break;

            default:
                break;
        }
    }

} // namespace maps_sim
