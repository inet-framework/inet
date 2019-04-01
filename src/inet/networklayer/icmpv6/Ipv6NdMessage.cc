/**
 * Copyright (C) 2019 OpenSim Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * @author Zoltan Bojthe
 */

#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"

namespace inet {

const Ipv6NdOption *Ipv6NdOptions::findOption(Ipv6NdOptionTypes t) const
{
    for (size_t i=0; i < option_arraysize; i++) {
        if (option[i]->getType() == t)
            return option[i];
    }
    return nullptr;
}

Ipv6NdOption *Ipv6NdOptions::findOptionForUpdate(Ipv6NdOptionTypes t)
{
    for (size_t i=0; i < option_arraysize; i++) {
        if (option[i]->getType() == t)
            return getOptionForUpdate(i);
    }
    return nullptr;
}

void Ipv6NdOptions::insertUniqueOption(size_t k, Ipv6NdOption * option)
{
    if (findOption(option->getType()))
        throw cRuntimeError("Option %i already exists", (int)option->getType());
    insertOption(option);
}

} // namespace inet

