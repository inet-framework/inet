#pragma once
#include "activation.h"
namespace keras2cpp{
    namespace layers{
        class Dense final : public Layer<Dense> {
            Tensor weights_;
            Tensor biases_;
            Activation activation_;
        public:
            Dense(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };

        class InputLayer final : public Layer<InputLayer> {
        public:
            InputLayer(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}
