//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SIMPLEMODULE_H
#define __INET_SIMPLEMODULE_H

#include "inet/common/INETDefs.h"
#include "inet/common/StringFormat.h"

namespace inet {

  /**
   * @brief This simple module throws an error on instantiation.
   *
   * This module is used for the @class() property of inet.common.SimpleModule
   * to throw an error on start if a user extends SimpleModule in NED
   * but forgot to add @class(MyModule) to the module definition.
   *
   * This would cause hard to detect issues as the MyModule would inherit
   * the @class() parameter from inet.common.SimpleModule instantiating
   * an empty inet::SimpleModule.
   */
class INET_API ForbiddenSimpleModule : public cSimpleModule
{
  public:
    virtual void initialize() override;
};

/**
 * A base module for all INET simple modules that implements
 * behavior common to all modules.
 *
 * Derived classes can implement resolveDirective() to support
 * %c style directives that can be used in displayStringTextFormat.
 *
 * refreshDisplay() implements displaying the displayStringTextFormat
 * parameter above the module in the t tag.
 *
 */
class INET_API SimpleModule : public cSimpleModule, public StringFormat::IDirectiveResolver
{
  protected:
    virtual void refreshDisplay() const override;

  public:
    // Implementation of StringFormat::IDirectiveResolver interface
    virtual std::string resolveDirective(char directive) const override;
    virtual std::string resolveExpression(const char *expression) const override;
};

} // namespace inet

#endif // __INET_SIMPLEMODULE_H
