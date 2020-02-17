#include "tensor.h"

namespace keras2cpp {
    Tensor::Tensor(Stream& file, size_t rank) : Tensor() {
        kassert(rank);

        dims_.reserve(rank);
        std::generate_n(std::back_inserter(dims_), rank, [&file] {
            unsigned stride = file;
            kassert(stride > 0);
            return stride;
        });

        data_.resize(size());
        file.reads(reinterpret_cast<char*>(data_.data()), sizeof(float) * size());
    }

    Tensor Tensor::unpack(size_t row) const noexcept {
        kassert(ndim() >= 2);
        size_t pack_size = std::accumulate(dims_.begin() + 1, dims_.end(), 0u);

        auto base = row * pack_size;
        auto first = begin() + cast(base);
        auto last = begin() + cast(base + pack_size);

        Tensor x;
        x.dims_ = std::vector<size_t>(dims_.begin() + 1, dims_.end());
        x.data_ = std::vector<float>(first, last);
        return x;
    }

    Tensor Tensor::select(size_t row) const noexcept {
        auto x = unpack(row);
        x.dims_.insert(x.dims_.begin(), 1);
        return x;
    }

    Tensor& Tensor::operator+=(const Tensor& other) noexcept {
        kassert(dims_ == other.dims_);
        std::transform(begin(), end(), other.begin(), begin(), std::plus<>());
        return *this;
    }

    Tensor& Tensor::operator*=(const Tensor& other) noexcept {
        kassert(dims_ == other.dims_);
        std::transform(begin(), end(), other.begin(), begin(), std::multiplies<>());
        return *this;
    }

    Tensor Tensor::fma(const Tensor& scale, const Tensor& bias) const noexcept {
        kassert(dims_ == scale.dims_);
        kassert(dims_ == bias.dims_);

        Tensor result;
        result.dims_ = dims_;
        result.data_.resize(data_.size());

        auto k_ = scale.begin();
        auto b_ = bias.begin();
        auto r_ = result.begin();
        for (auto x_ = begin(); x_ != end();)
            *(r_++) = *(x_++) * *(k_++) + *(b_++);

        return result;
    }

    Tensor Tensor::dot(const Tensor& other) const noexcept {
        kassert(ndim() == 2);
        kassert(other.ndim() == 2);
        kassert(dims_[1] == other.dims_[1]);

        Tensor tmp {dims_[0], other.dims_[0]};

        auto ts = cast(tmp.dims_[1]);
        auto is = cast(dims_[1]);

        auto i_ = begin();
        for (auto t0 = tmp.begin(); t0 != tmp.end(); t0 += ts, i_ += is) {
            auto o_ = other.begin();
            for (auto t1 = t0; t1 != t0 + ts; ++t1, o_ += is)
                *t1 = std::inner_product(i_, i_ + is, o_, 0.f);
        }
        return tmp;
    }

    void Tensor::print() const noexcept {
        std::vector<size_t> steps(ndim());
        std::partial_sum(
            dims_.rbegin(), dims_.rend(), steps.rbegin(), std::multiplies<>());

        size_t count = 0;
        for (auto&& it : data_) {
            for (auto step : steps)
                if (count % step == 0)
                    printf("[");
            printf("%f", static_cast<double>(it));
            ++count;
            for (auto step : steps)
                if (count % step == 0)
                    printf("]");
            if (count != steps[0])
                printf(", ");
        }
        printf("\n");
    }

    void Tensor::print_shape() const noexcept {
        printf("(");
        size_t count = 0;
        for (auto&& dim : dims_) {
            printf("%zu", dim);
            if ((++count) != dims_.size())
                printf(", ");
        }
        printf(")\n");
    }
}