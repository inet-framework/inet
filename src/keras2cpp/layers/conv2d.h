#pragma once
#include "activation.h"
namespace keras2cpp{
    namespace layers{
        class Conv2D final : public Layer<Conv2D> {
            Tensor weights_;
            Tensor biases_;
            Activation activation_;
        public:
            Conv2D(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}