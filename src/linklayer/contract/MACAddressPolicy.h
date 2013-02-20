//
// Copyright (C) 2013 Andras Varga
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

#ifndef __INET_MACADDRESSPOLICY_H
#define __INET_MACADDRESSPOLICY_H

#include "INETDefs.h"
#include "IAddressPolicy.h"
#include "MACAddress.h"
#include "GenericNetworkProtocolControlInfo.h"

class INET_API MACAddressPolicy : public IAddressPolicy
{
    public:
        static MACAddressPolicy INSTANCE;

    public:
        MACAddressPolicy() { }
        virtual ~MACAddressPolicy() { }

        virtual int getMaxPrefixLength() const { return 0; }
        virtual Address getLinkLocalManetRoutersMulticastAddress() const { return MACAddress(-109); } // TODO: constant
        virtual Address getLinkLocalRIPRoutersMulticastAddress() const { return MACAddress(-9); } // TODO: constant
        virtual INetworkProtocolControlInfo * createNetworkProtocolControlInfo() const { return new GenericNetworkProtocolControlInfo(); }
};

#endif
