#include "maxPooling2d.h"
namespace keras2cpp{
    namespace layers{
        MaxPooling2D::MaxPooling2D(Stream& file)
        : pool_size_y_(file), pool_size_x_(file) {}

        Tensor MaxPooling2D::operator()(const Tensor& in) const noexcept {
            kassert(in.ndim() == 3);

            const auto& iw = in.dims_;

            Tensor out {iw[0] / pool_size_y_, iw[1] / pool_size_x_, iw[2]};
            out.fill(-std::numeric_limits<float>::infinity());

            auto is0p = cast(iw[2] * iw[1] * pool_size_y_);
            auto is0 = cast(iw[2] * iw[1]);
            auto is1p = cast(iw[2] * pool_size_x_);
            auto is1 = cast(iw[2]);
            auto os_ = cast(iw[2] * out.dims_[1] * out.dims_[0]);
            auto os0 = cast(iw[2] * out.dims_[1]);

            auto o_ptr = out.begin();
            auto i_ptr = in.begin();
            for (auto o0 = o_ptr; o0 < o_ptr + os_; o0 += os0, i_ptr += is0p) {
                auto i_ = i_ptr;
                for (auto o1 = o0; o1 < o0 + os0; o1 += is1, i_ += is1p)
                    for (auto i0 = i_; i0 < i_ + is0p; i0 += is0)
                        for (auto i1 = i0; i1 < i0 + is1p; i1 += is1)
                            std::transform(i1, i1 + is1, o1, o1, [](float x, float y) {
                                return std::max(x, y);
                            });
            }
            return out;
        }


        AveragePooling1D::AveragePooling1D(Stream& file)
        : pool_size_(file) {}

        Tensor AveragePooling1D::operator()(const Tensor& in) const noexcept {
            kassert(in.ndim() == 2);  // steps, features
            kassert(in.dims_[0] % pool_size_ == 0);

            const auto& iw = in.dims_;

            int num_steps = in.dims_[0];
            int num_pools = num_steps / pool_size_;
            int num_features = in.dims_[1];

            Tensor out {num_pools, num_features};

            for (int feature = 0; feature < num_features; ++feature) {
                for (int pool = 0; pool < num_pools; ++pool) {
                    float sum = 0;
                    for (int step_of_pool = 0; step_of_pool < pool_size_; ++step_of_pool)
                        sum += in.data_[step_of_pool * pool_size * num_features + step * num_features + feature];
                    out.data_[pool * num_features + feature] = sum / pool_size_;
                }
            }

            return out;
        }




        GlobalAveragePooling1D::GlobalAveragePooling1D(Stream& file)
        : {}

        Tensor GlobalAveragePooling1D::operator()(const Tensor& in) const noexcept {
            kassert(in.ndim() == 2);  // steps, features

            const auto& iw = in.dims_;

            int num_steps = in.dims_[0];
            int num_features = in.dims_[1];

            Tensor out {num_features};

            for (int feature = 0; feature < num_features; ++feature) {
                float sum = 0;
                for (int step = 0; step < num_steps; ++step)
                    sum += in.data_[step * num_features + feature];
                out.data_[feature] = sum / num_steps;
            }

            return out;
        }

    }
}