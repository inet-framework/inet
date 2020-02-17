#pragma once
#include <algorithm>
#include <numeric>
#include "utils.h"
//#include "reader.h"

namespace keras2cpp {
    class Tensor {
        public:
            Tensor() = default;
            template <
                typename... Size,
                typename = std::enable_if_t<(... && std::is_integral_v<Size>)>>
            Tensor(Size... sizes) {
                resize(static_cast<size_t>(sizes)...);
            }

            Tensor(Stream& file, size_t rank = 1);

            template <typename... Size>
            static auto empty(Size... sizes);

            template <typename... Size>
            void resize(Size... sizes) noexcept;

            inline size_t size() const noexcept;
            inline size_t ndim() const noexcept;
            inline Tensor& flatten() noexcept;

            inline float& operator()(size_t) noexcept;
            inline float& operator()(size_t, size_t) noexcept;
            inline float& operator()(size_t, size_t, size_t) noexcept;
            inline float& operator()(size_t, size_t, size_t, size_t) noexcept;
            inline float operator()(size_t) const noexcept;
            inline float operator()(size_t, size_t) const noexcept;
            inline float operator()(size_t, size_t, size_t) const noexcept;
            inline float operator()(size_t, size_t, size_t, size_t) const noexcept;

            inline std::vector<float>::iterator begin() noexcept;
            inline std::vector<float>::const_iterator begin() const noexcept;
            inline std::vector<float>::iterator end() noexcept;
            inline std::vector<float>::const_iterator end() const noexcept;

            inline void fill(float value) noexcept;

            Tensor unpack(size_t row) const noexcept;
            Tensor select(size_t row) const noexcept;

            Tensor& operator+=(const Tensor& other) noexcept;
            Tensor& operator*=(const Tensor& other) noexcept;
            Tensor fma(const Tensor& scale, const Tensor& bias) const noexcept;
            Tensor dot(const Tensor& other) const noexcept;

            void print() const noexcept;
            void print_shape() const noexcept;

            std::vector<size_t> dims_;
            std::vector<float> data_;
    };

    template <typename... Size>
    auto Tensor::empty(Size... sizes) {
        Tensor tensor;
        tensor.dims_ = {static_cast<size_t>(sizes)...};
        tensor.data_.reserve(tensor.size());
        return tensor;
    }
    template <typename... Size>
    void Tensor::resize(Size... sizes) noexcept {
        dims_ = {static_cast<size_t>(sizes)...};
        data_.resize(size());
    }
    size_t Tensor::size() const noexcept {
        size_t elements = 1;
        for (const auto& it : dims_)
            elements *= it;
        return elements;
    }
    size_t Tensor::ndim() const noexcept {
        return dims_.size();
    }
    Tensor& Tensor::flatten() noexcept {
        kassert(ndim());
        dims_ = {size()};
        return *this;
    }
    float& Tensor::operator()(size_t i) noexcept {
        kassert(ndim() == 1);
        kassert(i < dims_[0]);
        return data_[i];
    }
    float Tensor::operator()(size_t i) const noexcept {
        kassert(ndim() == 1);
        kassert(i < dims_[0]);
        return data_[i];
    }
    float& Tensor::operator()(size_t i, size_t j) noexcept {
        kassert(ndim() == 2);
        kassert(i < dims_[0]);
        kassert(j < dims_[1]);
        return data_[dims_[1] * i + j];
    }
    float Tensor::operator()(size_t i, size_t j) const noexcept {
        kassert(ndim() == 2);
        kassert(i < dims_[0]);
        kassert(j < dims_[1]);
        return data_[dims_[1] * i + j];
    }
    float& Tensor::operator()(size_t i, size_t j, size_t k) noexcept {
        kassert(ndim() == 3);
        kassert(i < dims_[0]);
        kassert(j < dims_[1]);
        kassert(k < dims_[2]);
        return data_[dims_[2] * (dims_[1] * i + j) + k];
    }
    float Tensor::operator()(size_t i, size_t j, size_t k) const noexcept {
        kassert(ndim() == 3);
        kassert(i < dims_[0]);
        kassert(j < dims_[1]);
        kassert(k < dims_[2]);
        return data_[dims_[2] * (dims_[1] * i + j) + k];
    }
    float& Tensor::operator()(size_t i, size_t j, size_t k, size_t l) noexcept {
        kassert(ndim() == 4);
        kassert(i < dims_[0]);
        kassert(j < dims_[1]);
        kassert(k < dims_[2]);
        kassert(l < dims_[3]);
        return data_[dims_[3] * (dims_[2] * (dims_[1] * i + j) + k) + l];
    }
    float Tensor::operator()(size_t i, size_t j, size_t k, size_t l) const
        noexcept {
        kassert(ndim() == 4);
        kassert(i < dims_[0]);
        kassert(j < dims_[1]);
        kassert(k < dims_[2]);
        kassert(l < dims_[3]);
        return data_[dims_[3] * (dims_[2] * (dims_[1] * i + j) + k) + l];
    }
    void Tensor::fill(float value) noexcept {
        std::fill(begin(), end(), value);
    }
    std::vector<float>::iterator Tensor::begin() noexcept {
        return data_.begin();
    }
    std::vector<float>::const_iterator Tensor::begin() const noexcept {
        return data_.begin();
    }
    std::vector<float>::iterator Tensor::end() noexcept {
        return data_.end();
    }
    std::vector<float>::const_iterator Tensor::end() const noexcept {
        return data_.end();
    }
    inline Tensor operator+(Tensor lhs, const Tensor& rhs) noexcept {
        lhs += rhs;
        return lhs;
    }
    inline Tensor operator*(Tensor lhs, const Tensor& rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }
}