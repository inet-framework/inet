#include "batchNormalization.h"
namespace keras2cpp{
    namespace layers{
        BatchNormalization::BatchNormalization(Stream& file)
        : weights_(file), biases_(file) {}
        Tensor BatchNormalization::operator()(const Tensor& in) const noexcept {
            kassert(in.ndim());
            return in.fma(weights_, biases_);
        }
    }
}