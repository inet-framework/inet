#pragma once
#include "../baseLayer.h"
namespace keras2cpp{
    namespace layers{
        class MaxPooling2D final : public Layer<MaxPooling2D> {
            unsigned pool_size_y_{0};
            unsigned pool_size_x_{0};

        public:
            MaxPooling2D(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };


        class AveragePooling1D final : public Layer<AveragePooling1D> {
            unsigned pool_size_{0};

        public:
            AveragePooling1D(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };


        class GlobalAveragePooling1D final : public Layer<GlobalAveragePooling1D> {
        public:
            GlobalAveragePooling1D(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}