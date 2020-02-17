#include "conv2d.h"
namespace keras2cpp{
    namespace layers{
        Conv2D::Conv2D(Stream& file)
        : weights_(file, 4), biases_(file), activation_(file) {}

        Tensor Conv2D::operator()(const Tensor& in) const noexcept {
            kassert(in.dims_[2] == weights_.dims_[3]);

            auto& ww = weights_.dims_;

            size_t offset_y = ww[1] - 1;
            size_t offset_x = ww[2] - 1;
            auto tmp
                = Tensor::empty(in.dims_[0] - offset_y, in.dims_[1] - offset_x, ww[0]);

            auto ws_ = cast(ww[3] * ww[2] * ww[1] * ww[0]);
            auto ws0 = cast(ww[3] * ww[2] * ww[1]);
            auto ws1 = cast(ww[3] * ww[2]);
            auto ws2 = cast(ww[3]);
            auto is0 = cast(ww[3] * in.dims_[1]);

            auto ty = cast(tmp.dims_[0]);
            auto tx = cast(tmp.dims_[1]);

            auto w_ptr = weights_.begin();
            auto b_ptr = biases_.begin();
            auto t_ptr = std::back_inserter(tmp.data_);
            auto i_ptr = in.begin();

            for (ptrdiff_t y = 0; y < ty; ++y)
                for (ptrdiff_t x = 0; x < tx; ++x) {
                    auto b_ = b_ptr;
                    auto i_ = i_ptr + y * is0 + x * ws2;
                    for (auto w0 = w_ptr; w0 < w_ptr + ws_; w0 += ws0) {
                        auto tmp_ = 0.f;
                        auto i0 = i_;
                        for (auto w1 = w0; w1 < w0 + ws0; w1 += ws1, i0 += is0)
                            tmp_ = std::inner_product(w1, w1 + ws1, i0, tmp_);
                        *(++t_ptr) = *(b_++) + tmp_;
                    }
                }
            return activation_(tmp);
        }
    }
}