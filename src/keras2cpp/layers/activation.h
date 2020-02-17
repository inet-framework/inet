#pragma once
#include "../baseLayer.h"
namespace keras2cpp{
    namespace layers{
        class Activation final : public Layer<Activation> {
            enum _Type : unsigned {
                Linear = 1,
                Relu = 2,
                Elu = 3,
                SoftPlus = 4,
                SoftSign = 5,
                Sigmoid = 6,
                Tanh = 7,
                HardSigmoid = 8,
                SoftMax = 9
            };
            _Type type_ {Linear};
        
        public:
            Activation(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}