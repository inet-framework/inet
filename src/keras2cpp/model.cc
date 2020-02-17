#include "model.h"
#include "layers/conv1d.h"
#include "layers/conv2d.h"
#include "layers/dense.h"
#include "layers/elu.h"
#include "layers/embedding.h"
#include "layers/flatten.h"
#include "layers/locally1d.h"
#include "layers/locally2d.h"
#include "layers/lstm.h"
#include "layers/maxPooling2d.h"
#include "layers/batchNormalization.h"

namespace keras2cpp {
    std::unique_ptr<BaseLayer> Model::make_layer(Stream& file) {
        switch (static_cast<unsigned>(file)) {
            case InputLayer:
                return layers::InputLayer::make(file);
            case Dense:
                return layers::Dense::make(file);
            case Conv1D:
                return layers::Conv1D::make(file);
            case Conv2D:
                return layers::Conv2D::make(file);
            case LocallyConnected1D:
                return layers::LocallyConnected1D::make(file);
            case LocallyConnected2D:
                return layers::LocallyConnected2D::make(file);
            case Flatten:
                return layers::Flatten::make(file);
            case ELU:
                return layers::ELU::make(file);
            case Activation:
                return layers::Activation::make(file);
            case MaxPooling2D:
                return layers::MaxPooling2D::make(file);
            case LSTM:
                return layers::LSTM::make(file);
            case Embedding:
                return layers::Embedding::make(file);
            case BatchNormalization:
                return layers::BatchNormalization::make(file);
        }
        return nullptr;
    }

    Model::Model(Stream& file) {
        auto count = static_cast<unsigned>(file);
        layers_.reserve(count);
        for (size_t i = 0; i != count; ++i)
            layers_.push_back(make_layer(file));
    }

    Tensor Model::operator()(const Tensor& in) const noexcept {
        Tensor out = in;
        for (auto&& layer : layers_)
            out = (*layer)(out);
        return out;
    }
}
