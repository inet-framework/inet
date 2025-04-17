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
 * Derived classes can implement resolveDisplayStringTextDirective() to support
 * %c style directives that can be used in displayStringTextFormat.
 *
 * refreshDisplay() implements displaying the displayStringTextFormat
 * parameter above the module in the t tag.
 *
 */
class INET_API SimpleModule : public cSimpleModule, public StringFormat::IDisplayStringTextResolver
{
  protected:
    virtual void refreshDisplay() const override;

  public:
    virtual void initialize() override;
    virtual void initialize(int stage) override;
  // Implementation of StringFormat::IDisplayStringTextResolver interface
    virtual std::string resolveDisplayStringTextDirective(char directive) const override;
    virtual std::string resolveDisplayStringTextExpression(const char *expression) const override;
};

} // namespace inet

#endif // __INET_SIMPLEMODULE_H
