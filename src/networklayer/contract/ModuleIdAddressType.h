//
// Copyright (C) 2012 Opensim Ltd.
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

#ifndef __INET_MODULEIDADDRESSTYPE_H
#define __INET_MODULEIDADDRESSTYPE_H

#include "INETDefs.h"
#include "IAddressType.h"
#include "ModuleIdAddress.h"
#include "GenericNetworkProtocolControlInfo.h"

class INET_API ModuleIdAddressType : public IAddressType
{
    public:
        static ModuleIdAddressType INSTANCE;

    public:
        ModuleIdAddressType() { }
        virtual ~ModuleIdAddressType() { }

        virtual int getMaxPrefixLength() const { return 0; }
        virtual Address getUnspecifiedAddress() const { return ModuleIdAddress(); } // TODO: constant
        virtual Address getLinkLocalManetRoutersMulticastAddress() const { return ModuleIdAddress(-109); } // TODO: constant
        virtual Address getLinkLocalRIPRoutersMulticastAddress() const { return ModuleIdAddress(-9); } // TODO: constant
        virtual INetworkProtocolControlInfo * createNetworkProtocolControlInfo() const { return new GenericNetworkProtocolControlInfo(); }
        virtual Address getLinkLocalAddress(const InterfaceEntry *ie) const { return ModuleIdAddress(); } // TODO constant
};

#endif
