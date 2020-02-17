#include "lstm.h"
#include <tuple>
namespace keras2cpp{
    namespace layers{
        LSTM::LSTM(Stream& file)
        : Wi_(file, 2)
        , Ui_(file, 2)
        , bi_(file, 2) // Input
        , Wf_(file, 2)
        , Uf_(file, 2)
        , bf_(file, 2) // Forget
        , Wc_(file, 2)
        , Uc_(file, 2)
        , bc_(file, 2) // State
        , Wo_(file, 2)
        , Uo_(file, 2)
        , bo_(file, 2) // Output
        , inner_activation_(file)
        , activation_(file)
        , return_sequences_(static_cast<unsigned>(file)) {}

        Tensor LSTM::operator()(const Tensor& in) const noexcept {
            // Assume 'bo_' always keeps the output shape and we will always
            // receive one single sample.
            size_t out_dim = bo_.dims_[1];
            size_t steps = in.dims_[0];

            Tensor c_tm1 {1, out_dim};

            if (!return_sequences_) {
                Tensor out {1, out_dim};
                for (size_t s = 0; s < steps; ++s)
                    std::tie(out, c_tm1) = step(in.select(s), out, c_tm1);
                return out.flatten();
            }

            auto out = Tensor::empty(steps, out_dim);
            Tensor last {1, out_dim};

            for (size_t s = 0; s < steps; ++s) {
                std::tie(last, c_tm1) = step(in.select(s), last, c_tm1);
                out.data_.insert(out.end(), last.begin(), last.end());
            }
            return out;
        }

        std::tuple<Tensor, Tensor>
        LSTM::step(const Tensor& x, const Tensor& h_tm1, const Tensor& c_tm1) const
            noexcept {
            auto i_ = x.dot(Wi_) + h_tm1.dot(Ui_) + bi_;
            auto f_ = x.dot(Wf_) + h_tm1.dot(Uf_) + bf_;
            auto c_ = x.dot(Wc_) + h_tm1.dot(Uc_) + bc_;
            auto o_ = x.dot(Wo_) + h_tm1.dot(Uo_) + bo_;

            auto cc = inner_activation_(f_) * c_tm1
                + inner_activation_(i_) * activation_(c_);
            auto out = inner_activation_(o_) * activation_(cc);
            return std::make_tuple(out, cc);
        }
    }
}