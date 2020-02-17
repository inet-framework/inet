#include "conv1d.h"
namespace keras2cpp{
    namespace layers{
        Conv1D::Conv1D(Stream& file)
        : weights_(file, 3), biases_(file), activation_(file) {}

        Tensor Conv1D::operator()(const Tensor& in) const noexcept {
            kassert(in.dims_[1] == weights_.dims_[2]);

            auto& ww = weights_.dims_;

            size_t offset = ww[1] - 1;
            auto tmp = Tensor::empty(in.dims_[0] - offset, ww[0]);

            auto ws0 = cast(ww[2] * ww[1]);
            auto ws1 = cast(ww[2]);

            auto tx = cast(tmp.dims_[0]);

            auto i_ptr = in.begin();
            auto b_ptr = biases_.begin();
            auto t_ptr = std::back_inserter(tmp.data_);

            for (ptrdiff_t x = 0; x < tx; ++x) {
                auto b_ = b_ptr;
                auto i_ = i_ptr + x * ws1;
                for (auto w0 = weights_.begin(); w0 < weights_.end(); w0 += ws0)
                    *(t_ptr++) = std::inner_product(w0, w0 + ws0, i_, *(b_++));
            }
            return activation_(tmp);
        }
    }
}
