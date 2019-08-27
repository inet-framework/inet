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
class INET_API EitherInterpolator : public InterpolatorBase<X, Y>
{
  public:
    static EitherInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        ASSERT(y1 == y2);
        return y1;
    }

    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        ASSERT(y1 == y2);
        return y1;
    }
};

template<typename X, typename Y>
EitherInterpolator<X, Y> EitherInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API LeftInterpolator : public InterpolatorBase<X, Y>
{
  public:
    static LeftInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return y1;
    }

    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        return y1;
    }
};

template<typename X, typename Y>
LeftInterpolator<X, Y> LeftInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API RightInterpolator : public InterpolatorBase<X, Y>
{
  public:
    static RightInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return y2;
    }

    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        return y2;
    }
};

template<typename X, typename Y>
RightInterpolator<X, Y> RightInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API CenterInterpolator : public InterpolatorBase<X, Y>
{
  public:
    static CenterInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return (y1 + y2) / 2;
    }

    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        return (y1 + y2) / 2;
    }
};

template<typename X, typename Y>
CenterInterpolator<X, Y> CenterInterpolator<X, Y>::singleton;

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
class INET_API SmallerInterpolator : public InterpolatorBase<X, Y>
{
  public:
    static SmallerInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return std::min(y1, y2);
    }

    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        return std::min(y1, y2);
    }
};

template<typename X, typename Y>
SmallerInterpolator<X, Y> SmallerInterpolator<X, Y>::singleton;

template<typename X, typename Y>
class INET_API GreaterInterpolator : public InterpolatorBase<X, Y>
{
  public:
    static GreaterInterpolator<X, Y> singleton;

  public:
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return std::max(y1, y2);
    }

    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const override {
        ASSERT(x1 <= x2);
        return std::max(y1, y2);
    }
};

template<typename X, typename Y>
GreaterInterpolator<X, Y> GreaterInterpolator<X, Y>::singleton;

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
const IInterpolator<X, Y> *createInterpolator(const char *text) {
    if (!strcmp("either", text))
        return &EitherInterpolator<X, Y>::singleton;
    else if (!strcmp("left", text))
        return &LeftInterpolator<X, Y>::singleton;
    else if (!strcmp("right", text))
        return &RightInterpolator<X, Y>::singleton;
    else if (!strcmp("center", text))
        return &CenterInterpolator<X, Y>::singleton;
    else if (!strcmp("closer", text))
        return &CloserInterpolator<X, Y>::singleton;
    else if (!strcmp("smaller", text))
        return &SmallerInterpolator<X, Y>::singleton;
    else if (!strcmp("greater", text))
        return &GreaterInterpolator<X, Y>::singleton;
    else if (!strcmp("linear", text))
        return &LinearInterpolator<X, Y>::singleton;
    else
        throw cRuntimeError("Unknown interpolator: '%s'", text);
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_INTERPOLATORS_H_

