#pragma once
#include "../baseLayer.h"
namespace keras2cpp{
    namespace layers{
        class BatchNormalization final : public Layer<BatchNormalization> {
            Tensor weights_;
            Tensor biases_;
        public:
            BatchNormalization(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}