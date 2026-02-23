//
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"

namespace inet {

const Ipv6NdOption *Ipv6NdOptions::findOption(Ipv6NdOptionTypes t) const
{
    for (size_t i = 0; i < option_arraysize; i++) {
        if (option[i]->getType() == t)
            return option[i];
    }
    return nullptr;
}

Ipv6NdOption *Ipv6NdOptions::findOptionForUpdate(Ipv6NdOptionTypes t)
{
    for (size_t i = 0; i < option_arraysize; i++) {
        if (option[i]->getType() == t)
            return getOptionForUpdate(i);
    }
    return nullptr;
}

void Ipv6NdOptions::insertUniqueOption(size_t k, Ipv6NdOption *option)
{
    if (findOption(option->getType()))
        throw cRuntimeError("Option %i already exists", (int)option->getType());
    appendOption(option);
}

} // namespace inet

