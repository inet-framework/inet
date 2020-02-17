#include "elu.h"
namespace keras2cpp{
    namespace layers{
        ELU::ELU(Stream& file) : alpha_(file) {}    
        Tensor ELU::operator()(const Tensor& in) const noexcept {
            kassert(in.ndim());
            Tensor out;
            out.data_.resize(in.size());
            out.dims_ = in.dims_;

            std::transform(in.begin(), in.end(), out.begin(), [this](float x) {
                if (x >= 0.f)
                    return x;
                return alpha_ * std::expm1(x);
            });
            return out;
        }
    }
}