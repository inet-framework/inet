#pragma once
#include "../baseLayer.h"
namespace keras2cpp{
    namespace layers{
        class Embedding final : public Layer<Embedding> {
            Tensor weights_;

        public:
            Embedding(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}