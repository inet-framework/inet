#pragma once
#include "activation.h"
namespace keras2cpp{
    namespace layers{
        class LocallyConnected2D final : public Layer<LocallyConnected2D> {
            Tensor weights_;
            Tensor biases_;
            Activation activation_;
        public:
            LocallyConnected2D(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };

    }
}