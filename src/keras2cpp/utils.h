#pragma once
#include <chrono>
#include <cmath>	
#include <fstream>
#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>

#define stringify(x) #x

#define cast(x) static_cast<ptrdiff_t>(x)

#ifndef NDEBUG
#define kassert_eq(x, y, eps) \
    { \
        auto x_ = static_cast<double>(x); \
        auto y_ = static_cast<double>(y); \
        if (std::abs(x_ - y_) > eps) { \
            printf( \
                "ASSERT [%s:%d] %f isn't equal to %f ('%s' != '%s')\n", \
                __FILE__, __LINE__, x_, y_, stringify(x), stringify(y)); \
            exit(-1); \
        } \
    }
#define kassert(x) \
    if (!(x)) { \
        printf( \
            "ASSERT [%s:%d] '%s' failed\n", __FILE__, __LINE__, stringify(x)); \
        exit(-1); \
    }
#else
#define kassert(x) ;
#define kassert_eq(x, y, eps) ;
#endif

namespace keras2cpp {
    template <typename Callable, typename... Args>
    auto timeit(Callable&& callable, Args&&... args) {
        using namespace std::chrono;
        auto begin = high_resolution_clock::now();
        if constexpr (std::is_void_v<std::invoke_result_t<Callable, Args...>>)
            return std::make_tuple(
                (std::invoke(callable, args...), nullptr),
                duration<double>(high_resolution_clock::now() - begin).count());
        else
            return std::make_tuple(
                std::invoke(callable, args...),
                duration<double>(high_resolution_clock::now() - begin).count());
    }
    class Stream {
        std::ifstream stream_;
    
    public:
        Stream(const std::string& filename);
        Stream& reads(char*, size_t);
    
        template <
            typename T,
            typename = std::enable_if_t<std::is_default_constructible_v<T>>>
        operator T() noexcept {
            T value;
            reads(reinterpret_cast<char*>(&value), sizeof(T));
            return value;
        }
    };
}
