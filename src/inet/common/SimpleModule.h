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
