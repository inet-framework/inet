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

#ifndef __INET_MATH_ALGEBRAICOPERATIONS_H_
#define __INET_MATH_ALGEBRAICOPERATIONS_H_

#include "inet/common/math/PrimitiveFunctions.h"

namespace inet {

namespace math {

template<typename R, typename D>
class INET_API AddedFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function1;
    const Ptr<const IFunction<R, D>> function2;

  public:
    AddedFunction(const Ptr<const IFunction<R, D>>& function1, const Ptr<const IFunction<R, D>>& function2) :
        function1(function1), function2(function2)
    { }

    virtual typename D::I getDomain() const override {
        return function1->getDomain().getIntersected(function2->getDomain());
    }

    virtual R getValue(const typename D::P& p) const override {
        return function1->getValue(p) + function2->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        function1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            // NOTE: optimization for 0 + x
            if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                if (toDouble(f1c->getConstantValue()) == 0) {
                    function2->partition(i1, callback);
                    return;
                }
            }
            function2->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *f2) {
                // TODO: use template specialization for compile time optimization
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        ConstantFunction<R, D> g(f1c->getConstantValue() + f2c->getConstantValue());
                        callback(i2, &g);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f2l->getValue(i2.getLower()) + f1c->getConstantValue(), f2l->getValue(i2.getUpper()) + f1c->getConstantValue(), f2l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) + f2c->getConstantValue(), f1l->getValue(i2.getUpper()) + f2c->getConstantValue(), f1l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        if (f1l->getDimension() == f2l->getDimension()) {
                            UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) + f2l->getValue(i2.getLower()), f1l->getValue(i2.getUpper()) + f2l->getValue(i2.getUpper()), f1l->getDimension());
                            simplifyAndCall(i2, &g, callback);
                        }
                        else {
                            typename D::P lowerLower = D::P::getZero();
                            typename D::P lowerUpper = D::P::getZero();
                            typename D::P upperLower = D::P::getZero();
                            typename D::P upperUpper = D::P::getZero();
                            lowerLower.set(f1l->getDimension(), i1.getLower().get(f1l->getDimension()));
                            lowerUpper.set(f1l->getDimension(), i1.getLower().get(f1l->getDimension()));
                            upperLower.set(f1l->getDimension(), i1.getUpper().get(f1l->getDimension()));
                            upperUpper.set(f1l->getDimension(), i1.getUpper().get(f1l->getDimension()));
                            lowerLower.set(f2l->getDimension(), i2.getLower().get(f2l->getDimension()));
                            lowerUpper.set(f2l->getDimension(), i2.getUpper().get(f2l->getDimension()));
                            upperLower.set(f2l->getDimension(), i2.getLower().get(f2l->getDimension()));
                            upperUpper.set(f2l->getDimension(), i2.getUpper().get(f2l->getDimension()));
                            R rLowerLower = f1l->getValue(lowerLower) + f2l->getValue(lowerLower);
                            R rLowerUpper = f1l->getValue(lowerUpper) + f2l->getValue(lowerUpper);
                            R rUpperLower = f1l->getValue(upperLower) + f2l->getValue(upperLower);
                            R rUpperUpper = f1l->getValue(upperUpper) + f2l->getValue(upperUpper);
                            BilinearFunction<R, D> g(lowerLower, lowerUpper, upperLower, upperUpper, rLowerLower, rLowerUpper, rUpperLower, rUpperUpper, f1l->getDimension(), f2l->getDimension());
                            simplifyAndCall(i2, &g, callback);
                        }
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return function1->isFinite(i) & function2->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(+ ";
        function1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        function2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API SubtractedFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function1;
    const Ptr<const IFunction<R, D>> function2;

  public:
    SubtractedFunction(const Ptr<const IFunction<R, D>>& function1, const Ptr<const IFunction<R, D>>& function2) :
        function1(function1), function2(function2)
    { }

    virtual typename D::I getDomain() const override {
        return function1->getDomain().getIntersected(function2->getDomain());
    }

    virtual R getValue(const typename D::P& p) const override {
        return function1->getValue(p) - function2->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        function1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            function2->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *f2) {
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        ConstantFunction<R, D> g(f1c->getConstantValue() - f2c->getConstantValue());
                        callback(i2, &g);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f2l->getValue(i2.getLower()) - f1c->getConstantValue(), f2l->getValue(i2.getUpper()) - f1c->getConstantValue(), f2l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) - f2c->getConstantValue(), f1l->getValue(i2.getUpper()) - f2c->getConstantValue(), f1l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        if (f1l->getDimension() == f2l->getDimension()) {
                            UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) - f2l->getValue(i2.getLower()), f1l->getValue(i2.getUpper()) - f2l->getValue(i2.getUpper()), f1l->getDimension());
                            simplifyAndCall(i2, &g, callback);
                        }
                        else
                            throw cRuntimeError("TODO");
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return function1->isFinite(i) & function2->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(- ";
        function1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        function2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API MultipliedFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function1;
    const Ptr<const IFunction<double, D>> function2;

  public:
    MultipliedFunction(const Ptr<const IFunction<R, D>>& function1, const Ptr<const IFunction<double, D>>& function2) :
        function1(function1), function2(function2)
    { }

    virtual typename D::I getDomain() const override {
        return function1->getDomain().getIntersected(function2->getDomain());
    }

    virtual const Ptr<const IFunction<R, D>>& getF1() const { return function1; }

    virtual const Ptr<const IFunction<double, D>>& getF2() const { return function2; }

    virtual R getValue(const typename D::P& p) const override {
        return function1->getValue(p) * function2->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        function1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            // NOTE: optimization for 0 * x
            if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                if (toDouble(f1c->getConstantValue()) == 0 && function2->isFinite(i1)) {
                    callback(i1, f1);
                    return;
                }
            }
            function2->partition(i1, [&] (const typename D::I& i2, const IFunction<double, D> *f2) {
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<double, D> *>(f2)) {
                        ConstantFunction<R, D> g(f1c->getConstantValue() * f2c->getConstantValue());
                        callback(i2, &g);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<double, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f2l->getValue(i2.getLower()) * f1c->getConstantValue(), f2l->getValue(i2.getUpper()) * f1c->getConstantValue(), f2l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<double, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) * f2c->getConstantValue(), f1l->getValue(i2.getUpper()) * f2c->getConstantValue(), f1l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<double, D> *>(f2)) {
                        // QuadraticFunction<double, D> g();
                        throw cRuntimeError("TODO");
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return function1->isFinite(i) & function2->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(* ";
        function1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        function2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API DividedFunction : public FunctionBase<double, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function1;
    const Ptr<const IFunction<R, D>> function2;

  public:
    DividedFunction(const Ptr<const IFunction<R, D>>& function1, const Ptr<const IFunction<R, D>>& function2) : function1(function1), function2(function2) { }

    virtual typename D::I getDomain() const override {
        return function1->getDomain().getIntersected(function2->getDomain());
    }

    virtual double getValue(const typename D::P& p) const override {
        return unit(function1->getValue(p) / function2->getValue(p)).get();
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<double, D> *)> callback) const override {
        function1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            // NOTE: optimization for 0 / x
            if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                if (toDouble(f1c->getConstantValue()) == 0 && function2->isNonZero(i1)) {
                    ConstantFunction<double, D> g(0);
                    callback(i1, &g);
                    return;
                }
            }
            function2->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *f2) {
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        ConstantFunction<double, D> g(unit(f1c->getConstantValue() / f2c->getConstantValue()).get());
                        callback(i2, &g);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        UnireciprocalFunction<double, D> g(0, toDouble(f1c->getConstantValue()), f2l->getA(), f2l->getB(), f2l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        UnilinearFunction<double, D> g(i2.getLower(), i2.getUpper(), unit(f1l->getValue(i2.getLower()) / f2c->getConstantValue()).get(), unit(f1l->getValue(i2.getUpper()) / f2c->getConstantValue()).get(), f1l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        if (f1l->getDimension() == f2l->getDimension()) {
                            UnireciprocalFunction<double, D> g(f1l->getA(), f1l->getB(), f2l->getA(), f2l->getB(), f2l->getDimension());
                            simplifyAndCall(i2, &g, callback);
                        }
                        else {
                            throw cRuntimeError("TODO");
                            // BireciprocalFunction<double, D> g(...);
                            // simplifyAndCall(i2, &g, f);
                        }
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }

    virtual bool isFinite(const typename D::I& i) const override { return false; }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(/ ";
        function1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        function2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API SummedFunction : public FunctionBase<R, D>
{
  protected:
    std::vector<Ptr<const IFunction<R, D>>> functions;

  public:
    SummedFunction() { }
    SummedFunction(const std::vector<Ptr<const IFunction<R, D>>>& functions) : functions(functions) { }

    const std::vector<Ptr<const IFunction<R, D>>>& getElements() const { return functions; }

    virtual void addElement(const Ptr<const IFunction<R, D>>& f) {
        functions.push_back(f);
    }

    virtual void removeElement(const Ptr<const IFunction<R, D>>& f) {
        functions.erase(std::remove(functions.begin(), functions.end(), f), functions.end());
    }

    virtual typename D::I getDomain() const override {
        typename D::I domain(D::P::getLowerBounds(), D::P::getUpperBounds(), 0, 0, 0);
        for (const auto& f : functions)
            domain = domain.getIntersected(f->getDomain());
        return domain;
    }

    virtual R getValue(const typename D::P& p) const override {
        R sum = R(0);
        for (const auto& f : functions)
            sum += f->getValue(p);
        return sum;
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        ConstantFunction<R, D> g(R(0));
        partition(0, i, callback, &g);
    }

    virtual void partition(int index, const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback, const IFunction<R, D> *f) const {
        if (index == (int)functions.size())
            simplifyAndCall(i, f, callback);
        else {
            functions[index]->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
                if (auto fc = dynamic_cast<const ConstantFunction<R, D> *>(f)) {
                    if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                        ConstantFunction<R, D> g(fc->getConstantValue() + f1c->getConstantValue());
                        partition(index + 1, i1, callback, &g);
                    }
                    else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                        UnilinearFunction<R, D> g(i1.getLower(), i1.getUpper(), f1l->getValue(i1.getLower()) + fc->getConstantValue(), f1l->getValue(i1.getUpper()) + fc->getConstantValue(), f1l->getDimension());
                        partition(index + 1, i1, callback, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto fl = dynamic_cast<const UnilinearFunction<R, D> *>(f)) {
                    if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                        UnilinearFunction<R, D> g(i1.getLower(), i1.getUpper(), fl->getValue(i1.getLower()) + f1c->getConstantValue(), fl->getValue(i1.getUpper()) + f1c->getConstantValue(), fl->getDimension());
                        partition(index + 1, i1, callback, &g);
                    }
                    else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                        if (fl->getDimension() == f1l->getDimension()) {
                            UnilinearFunction<R, D> g(i1.getLower(), i1.getUpper(), fl->getValue(i1.getLower()) + f1l->getValue(i1.getLower()), fl->getValue(i1.getUpper()) + f1l->getValue(i1.getUpper()), fl->getDimension());
                            partition(index + 1, i1, callback, &g);
                        }
                        else
                            throw cRuntimeError("TODO");
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        }
    }

    virtual bool isFinite(const typename D::I& i) const override {
        for (const auto& f : functions)
            if (!f->isFinite(i))
                return false;
        return true;
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Σ ";
        bool first = true;
        for (const auto& f : functions) {
            if (first)
                first = false;
            else
                os << "\n" << std::string(level + 3, ' ');
            f->printStructure(os, level + 3);
        }
        os << ")";
    }
};

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_ALGEBRAICOPERATIONS_H_

