#pragma once
#include "../baseLayer.h"
namespace keras2cpp{
    namespace layers{
        class Flatten final : public Layer<Flatten> {
        public:
            using Layer<Flatten>::Layer;
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}