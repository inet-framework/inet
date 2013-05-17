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

#ifndef INETWORKPROTOCOLCONTROLINFO_H_
#define INETWORKPROTOCOLCONTROLINFO_H_

#include "Address.h"

class INET_API INetworkProtocolControlInfo {
  public:
    virtual ~INetworkProtocolControlInfo() { }
    virtual short getProtocol() const = 0;
    virtual void setProtocol(short protocol) = 0;
    virtual Address getSourceAddress() const = 0;
    virtual void setSourceAddress(const Address & address) = 0;
    virtual Address getDestinationAddress() const = 0;
    virtual void setDestinationAddress(const Address & address) = 0;
    virtual int getInterfaceId() const = 0;
    virtual void setInterfaceId(int interfaceId) = 0;
    virtual short getHopLimit() const = 0;
    virtual void setHopLimit(short hopLimit) = 0;
};

#endif /* INETWORKPROTOCOLCONTROLINFO_H_ */
