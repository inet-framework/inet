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

#ifndef __INET_IADDRESSTYPE_H
#define __INET_IADDRESSTYPE_H

#include "INETDefs.h"
#include "Address.h"
#include "INetworkProtocolControlInfo.h"

class INET_API IAddressType
{
    public:
        IAddressType() { }
        virtual ~IAddressType() { }

        virtual int getMaxPrefixLength() const = 0;
        virtual Address getLinkLocalManetRoutersMulticastAddress() const = 0;
        virtual Address getLinkLocalRIPRoutersMulticastAddress() const = 0;
        virtual INetworkProtocolControlInfo * createNetworkProtocolControlInfo() const = 0; // TODO: move, where?
};

#endif
