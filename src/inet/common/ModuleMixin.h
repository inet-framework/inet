//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEMIXIN_H
#define __INET_MODULEMIXIN_H

#include "inet/common/StringFormat.h"

#include <algorithm>
#include <type_traits>
#include <vector>

namespace inet {

namespace internal {
void refreshDisplayString(cModule *thisModule, const StringFormat::IResolver *thisModuleAsResolver);
std::string doResolveExpression(cModule *targetModule, const char *expression);
}

/**
 * A base functionality for all INET modules that implements behavior common to all modules.
 *
 * Additional features compared to cSimpleModule / cModule:
 * - Support for displayStringTextFormat parameter
 * - Derived classes can implement resolveDirective() to support %c style directives that can
 *   be used in displayStringTextFormat.
 */
template<typename T>
class INET_API ModuleMixin : public T, public StringFormat::IResolver
{
  static_assert(std::is_base_of<cModule, T>::value, "Type parameter of ModuleMixin must be a subclass of cModule");

  protected:

  protected:
    virtual void initialize() override { T::initialize(); }
    virtual void initialize(int stage) override { T::initialize(stage); }

    virtual void refreshDisplay() const override
    {
        internal::refreshDisplayString(const_cast<ModuleMixin<T>*>(this), this);
        T::refreshDisplay();
    }

  public:
    // Implementation of StringFormat::IResolver interface
    virtual std::string resolveDirective(char directive) const override
    {
        throw cRuntimeError("Unknown directive: %c", directive);
    }

    virtual std::string resolveExpression(const char *expression) const override
    {
        return internal::doResolveExpression(const_cast<ModuleMixin<T>*>(this), expression);
    }
};

} // namespace inet

#endif
