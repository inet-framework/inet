/*
 * Copyright (c) 2016 Robert W. Rose
 * Copyright (c) 2018 Paul Maevskikh
 *
 * MIT License, see LICENSE file.
 */
#pragma once

#include "activation.h"
namespace keras2cpp{
    namespace layers{
        class Conv1D final : public Layer<Conv1D> {
            Tensor weights_;
            Tensor biases_;
            Activation activation_;

        public:
            Conv1D(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}
