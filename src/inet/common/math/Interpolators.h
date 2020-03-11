//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_MATH_INTERPOLATORS_H_
#define __INET_MATH_INTERPOLATORS_H_

#include "inet/common/math/IInterpolator.h"
#include "inet/common/math/Point.h"

namespace inet {

namespace math {

template<typename X, typename Y>
class INET_API InterpolatorBase : public IInterpolator<X, Y>
{
  public:
    virtual Y getMin(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        return std::min(y1, y2);
    }

    virtual Y getMax(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        return std::max(y1, y2);
    }
};

template<typename X, typename Y>
class INET_API ConstantInterpolatorBase : public InterpolatorBase<X, Y>
{
    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        auto v1 = this->getValue(x1, y1, x2, y2, x1);
        auto v2 = this->getValue(x1, y1, x2, y2, x2);
        ASSERT(v1 == v2);
        return v1;
    }
};

/**
 * Interpolation that can only be used if y1 == y2.
 */
template<typename X, typename Y>
class INET_API EitherInterpolator : public ConstantInterpolatorBase<X, Y>
{
  public:
    static EitherInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        ASSERT(y1 == y2);
        return y1;
    }
};

template<typename X, typename Y>
EitherInterpolator<X, Y> EitherInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API LeftInterpolator : public ConstantInterpolatorBase<X, Y>
{
  public:
    static LeftInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return y1;
    }
};

template<typename X, typename Y>
LeftInterpolator<X, Y> LeftInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API RightInterpolator : public ConstantInterpolatorBase<X, Y>
{
  public:
    static RightInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return y2;
    }
};

template<typename X, typename Y>
RightInterpolator<X, Y> RightInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API AverageInterpolator : public ConstantInterpolatorBase<X, Y>
{
  public:
    static AverageInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return (y1 + y2) / 2;
    }
};

template<typename X, typename Y>
AverageInterpolator<X, Y> AverageInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API MinimumInterpolator : public ConstantInterpolatorBase<X, Y>
{
  public:
    static MinimumInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return std::min(y1, y2);
    }
};

template<typename X, typename Y>
MinimumInterpolator<X, Y> MinimumInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API MaximumInterpolator : public ConstantInterpolatorBase<X, Y>
{
  public:
    static MaximumInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return std::max(y1, y2);
    }
};

template<typename X, typename Y>
MaximumInterpolator<X, Y> MaximumInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API CloserInterpolator : public InterpolatorBase<X, Y>
{
  public:
    static CloserInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return x - x1 < x2 - x ? y1 : y2;
    }

    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        return (y1 + y2) / 2;
    }
};

template<typename X, typename Y>
CloserInterpolator<X, Y> CloserInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API LinearInterpolator : public InterpolatorBase<X, Y>
{
  public:
    static LinearInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        if (x1 == x)
            return y1;
        else if (x2 == x)
            return y2;
        else {
            auto a = toDouble(x - x1) / toDouble(x2 - x1);
            return y1 * (1 - a) + y2 * a;
        }
    }

    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        return (y1 + y2) / 2;
    }
};

template<typename X, typename Y>
LinearInterpolator<X, Y> LinearInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API LineardbInterpolator : public InterpolatorBase<X, Y>
{
  public:
    static LineardbInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        if (x1 == x)
            return y1;
        else if (x2 == x)
            return y2;
        else {
            auto a = toDouble(x - x1) / toDouble(x2 - x1);
            auto y1dB = math::fraction2dB(y1);
            auto y2dB = math::fraction2dB(y2);
            return math::dB2fraction(y1dB * (1 - a) + y2dB * a);
        }
    }

    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        auto y1dB = math::fraction2dB(y1);
        auto y2dB = math::fraction2dB(y2);
        return math::dB2fraction((y1dB + y2dB) / 2);
    }
};

template<typename X, typename Y>
LineardbInterpolator<X, Y> LineardbInterpolator<X, Y>::singleton;

template<typename X, typename Y>
const IInterpolator<X, Y> *createInterpolator(const char *text) {
    if (!strcmp("either", text))
        return &EitherInterpolator<X, Y>::singleton;
    else if (!strcmp("left", text))
        return &LeftInterpolator<X, Y>::singleton;
    else if (!strcmp("right", text))
        return &RightInterpolator<X, Y>::singleton;
    else if (!strcmp("average", text))
        return &AverageInterpolator<X, Y>::singleton;
    else if (!strcmp("closer", text))
        return &CloserInterpolator<X, Y>::singleton;
    else if (!strcmp("minimum", text))
        return &MinimumInterpolator<X, Y>::singleton;
    else if (!strcmp("maximum", text))
        return &MaximumInterpolator<X, Y>::singleton;
    else if (!strcmp("linear", text))
        return &LinearInterpolator<X, Y>::singleton;
    else if (!strcmp("lineardb", text))
        return &LineardbInterpolator<X, Y>::singleton;
    else
        throw cRuntimeError("Unknown interpolator: '%s'", text);
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_INTERPOLATORS_H_

