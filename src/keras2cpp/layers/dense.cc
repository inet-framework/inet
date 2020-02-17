#include "dense.h"
namespace keras2cpp{
    namespace layers{
        Dense::Dense(Stream& file)
        : weights_(file, 2), biases_(file), activation_(file) {}

        Tensor Dense::operator()(const Tensor& in) const noexcept {
            kassert(in.dims_.back() == weights_.dims_[1]);
            const auto ws = cast(weights_.dims_[1]);

            Tensor tmp;
            tmp.dims_ = in.dims_;
            tmp.dims_.back() = weights_.dims_[0];
            tmp.data_.reserve(tmp.size());

            auto tmp_ = std::back_inserter(tmp.data_);
            for (auto in_ = in.begin(); in_ < in.end(); in_ += ws) {
                auto bias_ = biases_.begin();
                for (auto w = weights_.begin(); w < weights_.end(); w += ws)
                    *(tmp_++) = std::inner_product(w, w + ws, in_, *(bias_++));
            }
            return activation_(tmp);
        }

         InputLayer::InputLayer(Stream& file)
         {}

        Tensor InputLayer::operator()(const Tensor& in) const noexcept {
            return in;
        }
    }
}