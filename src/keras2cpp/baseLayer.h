#pragma once

#include "tensor.h"
#include <memory>

namespace keras2cpp {
    class BaseLayer {
    public:
        BaseLayer() = default;
        BaseLayer(Stream&) : BaseLayer() {}
        BaseLayer(BaseLayer&&) = default;
        BaseLayer& operator=(BaseLayer&&) = default;
        virtual ~BaseLayer();
        virtual Tensor operator()(const Tensor& in) const noexcept = 0;
    };
    template <typename Derived>
    class Layer : public BaseLayer {
    public:
        using BaseLayer::BaseLayer;
        static Derived *load(const std::string& filename) {
            Stream file(filename);
            return new Derived(file);
        }

        static std::unique_ptr<BaseLayer> make(Stream& file) {
            return std::make_unique<Derived>(file);
        }
    };
}
