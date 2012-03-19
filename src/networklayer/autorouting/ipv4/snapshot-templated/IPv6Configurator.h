//
// Copyright (C) 2011 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IPV6CONFIGURATOR_H
#define __INET_IPV6CONFIGURATOR_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "uint128.h"
#include "IPv6Address.h"
#include "IPConfigurator.h"


/**
 * Configures IPv6 addresses for a network.
 *
 * For more info please see the NED file.
 */
class INET_API IPv6Configurator : public IPConfigurator<Uint128>
{
    protected:
        // main functionality
        virtual void initialize(int stage);
        virtual void assignAddress(InterfaceEntry *interfaceEntry, Uint128 address, Uint128 netmask);
        virtual std::string toString(Uint128 value) { return ((IPv6Address)value).str(); }
};

#endif
