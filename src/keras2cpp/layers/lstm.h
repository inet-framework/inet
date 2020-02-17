#pragma once
#include "activation.h"
namespace keras2cpp{
    namespace layers{
        class LSTM final : public Layer<LSTM> {
            Tensor Wi_;
            Tensor Ui_;
            Tensor bi_;
            Tensor Wf_;
            Tensor Uf_;
            Tensor bf_;
            Tensor Wc_;
            Tensor Uc_;
            Tensor bc_;
            Tensor Wo_;
            Tensor Uo_;
            Tensor bo_;
        
            Activation inner_activation_;
            Activation activation_;
            bool return_sequences_{false};
        
            std::tuple<Tensor, Tensor>
            step(const Tensor& x, const Tensor& ht_1, const Tensor& ct_1)
                 const noexcept;
        
        public:
            LSTM(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}
