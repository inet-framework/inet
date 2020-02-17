#include "locally2d.h"
namespace keras2cpp{
    namespace layers{
        LocallyConnected2D::LocallyConnected2D(Stream& file)
        : weights_(file, 4), biases_(file, 3), activation_(file) {}

        Tensor LocallyConnected2D::operator()(const Tensor& in) const noexcept {
            /*
            // 'in' have shape (x, y, features)
            // 'tmp' have shape (new_x, new_y, outputs)
            // 'weights' have shape (new_x*new_y, outputs, kernel*features)
            // 'biases' have shape (new_x*new_y, outputs)
            auto& ww = weights_.dims_;

            size_t ksize = ww[2] / in.dims_[1];
            size_t offset = ksize - 1;
            kassert(in.dims_[0] - offset == ww[0]);

            auto tmp = Tensor::empty(ww[0], ww[1]);

            auto is0 = cast(in.dims_[1]);
            auto ts0 = cast(ww[1]);
            auto ws0 = cast(ww[2] * ww[1]);
            auto ws1 = cast(ww[2]);

            auto b_ptr = biases_.begin();
            auto t_ptr = tmp.begin();
            auto i_ptr = in.begin();

            for (auto w_ = weights_.begin(); w_ < weights_.end();
                 w_ += ws0, b_ptr += ts0, t_ptr += ts0, i_ptr += is0) {
                auto b_ = b_ptr;
                auto t_ = t_ptr;
                auto i_ = i_ptr;
                for (auto w0 = w_; w0 < w_ + ws0; w0 += ws1)
                    *(t_++) = std::inner_product(w0, w0 + ws1, i_, *(b_++));
            }
            return activation_(tmp);
            */
            return activation_(in);
        }
    }
}
