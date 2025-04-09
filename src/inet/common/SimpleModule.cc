//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/SimpleModule.h"

namespace inet {

void SimpleModule::refreshDisplay() const
{
    auto displayStringTextFormat = par("displayStringTextFormat").stringValue();
    if (!opp_isempty(displayStringTextFormat)) {
        auto text = StringFormat::formatString(displayStringTextFormat, this);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
}

std::string SimpleModule::resolveDirective(char directive) const
{
    throw cRuntimeError("Unknown directive: %c", directive);
}

std::string SimpleModule::resolveExpression(const char *expression) const
{
    cModule *module = const_cast<SimpleModule*>(this);
    cObject *obj = module->findObject(expression, false);
    if (obj)
        return obj->str();
    else    
        throw cRuntimeError("Unknown expression: %s", expression);
}

} // namespace inet
