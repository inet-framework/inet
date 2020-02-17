#include "activation.h"
namespace keras2cpp{
    namespace layers{
        Activation::Activation(Stream& file) : type_(file) {
            switch (type_) {
            case Linear:
            case Relu:
            case Elu:
            case SoftPlus:
            case SoftSign:
            case HardSigmoid:
            case Sigmoid:
            case Tanh:
            case SoftMax:
                return;
            }
            kassert(false);
        }

        Tensor Activation::operator()(const Tensor& in) const noexcept {
            Tensor out {in.size()};
            out.dims_ = in.dims_;

            switch (type_) {
            case Linear:
                std::copy(in.begin(), in.end(), out.begin());
                break;
            case Relu:
                std::transform(in.begin(), in.end(), out.begin(), [](float x) {
                    if (x < 0.f)
                        return 0.f;
                    return x;
                });
                break;
            case Elu:
                std::transform(in.begin(), in.end(), out.begin(), [](float x) {
                    if (x < 0.f)
                        return std::expm1(x);
                    return x;
                });
                break;
            case SoftPlus:
                std::transform(in.begin(), in.end(), out.begin(), [](float x) {
                    return std::log1p(std::exp(x));
                });
                break;
            case SoftSign:
                std::transform(in.begin(), in.end(), out.begin(), [](float x) {
                    return x / (1.f + std::abs(x));
                });
                break;
            case HardSigmoid:
                std::transform(in.begin(), in.end(), out.begin(), [](float x) {
                    if (x <= -2.5f)
                        return 0.f;
                    if (x >= 2.5f)
                        return 1.f;
                    return (x * .2f) + .5f;
                });
                break;
            case Sigmoid:
                std::transform(in.begin(), in.end(), out.begin(), [](float x) {
                    float z = std::exp(-std::abs(x));
                    if (x < 0)
                        return z / (1.f + z);
                    return 1.f / (1.f + z);
                });
                break;
            case Tanh:
                std::transform(in.begin(), in.end(), out.begin(), [](float x) {
                    return std::tanh(x);
                });
                break;
            case SoftMax: {
                auto channels = cast(in.dims_.back());
                kassert(channels > 1);

                Tensor tmp = in;
                std::transform(in.begin(), in.end(), tmp.begin(), [](float x) {
                    return std::exp(x);
                });

                auto out_ = out.begin();
                for (auto t_ = tmp.begin(); t_ != tmp.end(); t_ += channels) {
                    // why std::reduce not in libstdc++ yet?
                    auto norm = 1.f / std::accumulate(t_, t_ + channels, 0.f);
                    std::transform(
                        t_, t_ + channels, out_, [norm](float x) { return norm * x; });
                    out_ += channels;
                }
                break;
            }
            }
            return out;
        }
    }
}