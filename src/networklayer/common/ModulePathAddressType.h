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

#ifndef __INET_MODULEPATHADDRESSTYPE_H
#define __INET_MODULEPATHADDRESSTYPE_H

#include "INETDefs.h"
#include "IAddressType.h"
#include "ModulePathAddress.h"
#include "GenericNetworkProtocolControlInfo.h"

class INET_API ModulePathAddressType : public IAddressType
{
    public:
        static ModulePathAddressType INSTANCE;

    public:
        ModulePathAddressType() { }
        virtual ~ModulePathAddressType() { }

        virtual int getMaxPrefixLength() const { return 0; } // TODO: support address prefixes
        virtual Address getUnspecifiedAddress() const { return ModulePathAddress(); }
        virtual Address getLinkLocalManetRoutersMulticastAddress() const { return ModulePathAddress(-109); } // TODO: constant
        virtual Address getLinkLocalRIPRoutersMulticastAddress() const { return ModulePathAddress(-9); } // TODO: constant
        virtual INetworkProtocolControlInfo * createNetworkProtocolControlInfo() const { return new GenericNetworkProtocolControlInfo(); }
        virtual Address getLinkLocalAddress(const InterfaceEntry *ie) const { return ModulePathAddress(); } // TODO constant
};

#endif
