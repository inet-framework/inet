//
// Copyright (C) 2005,2012 Andras Varga
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


#ifndef _UDPCONTROLINFO_H_
#define _UDPCONTROLINFO_H_

#include <vector>
#include "IPvXAddress.h"
#include "UDPControlInfo_m.h"

typedef std::vector<IPvXAddress> IPvXAddressList;

class INET_API UDPJoinMulticastGroupsCommand : public UDPJoinMulticastGroupsCommand_Base
{
  public:
    UDPJoinMulticastGroupsCommand() : UDPJoinMulticastGroupsCommand_Base() {}
    UDPJoinMulticastGroupsCommand(const UDPJoinMulticastGroupsCommand& other) : UDPJoinMulticastGroupsCommand_Base(other) {}
    UDPJoinMulticastGroupsCommand& operator=(const UDPJoinMulticastGroupsCommand& other) {if (this==&other) return *this; UDPJoinMulticastGroupsCommand_Base::operator=(other); return *this;}
    virtual UDPJoinMulticastGroupsCommand *dup() const {return new UDPJoinMulticastGroupsCommand(*this);}
    virtual const IPvXAddress *getMulticastAddrArrayPtr() const { return multicastAddr_var; }
    virtual const int *getInterfaceIdArrayPtr() const { return interfaceId_var; }
};


class INET_API UDPLeaveMulticastGroupsCommand : public UDPLeaveMulticastGroupsCommand_Base
{
  public:
    UDPLeaveMulticastGroupsCommand() : UDPLeaveMulticastGroupsCommand_Base() {}
    UDPLeaveMulticastGroupsCommand(const UDPLeaveMulticastGroupsCommand& other) : UDPLeaveMulticastGroupsCommand_Base(other) {}
    UDPLeaveMulticastGroupsCommand& operator=(const UDPLeaveMulticastGroupsCommand& other) {if (this==&other) return *this; UDPLeaveMulticastGroupsCommand_Base::operator=(other); return *this;}
    virtual UDPLeaveMulticastGroupsCommand *dup() const {return new UDPLeaveMulticastGroupsCommand(*this);}
    virtual const IPvXAddress *getMulticastAddrArrayPtr() const { return multicastAddr_var; }
};

#endif
