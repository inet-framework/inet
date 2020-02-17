#pragma once
#include "baseLayer.h"
namespace keras2cpp {
    class Model : public Layer<Model> {
        enum _LayerType : unsigned {
            InputLayer = 1,
            Dense = 2,
            Conv1D = 3,
            Conv2D = 4,
            LocallyConnected1D = 5,
            LocallyConnected2D = 6,
            Flatten = 7,
            ELU = 8,
            Activation = 9,
            MaxPooling2D = 10,
            LSTM = 11,
            Embedding = 12,
            BatchNormalization = 13,
        };
        std::vector<std::unique_ptr<BaseLayer>> layers_;

        static std::unique_ptr<BaseLayer> make_layer(Stream&);

    public:
        Model(Stream& file);
        Tensor operator()(const Tensor& in) const noexcept override;
    };
}