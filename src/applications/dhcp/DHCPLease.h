//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
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
#ifndef __DHCPLEASE_H__
#define __DHCPLEASE_H__

#include "Byte.h"
#include "IPv4Address.h"
#include "MACAddress.h"
#include "ARP.h"

class DHCPLease
{
    public:
        long xid;
        IPv4Address ip;
        MACAddress mac;
        IPv4Address gateway;
        IPv4Address network;
        IPv4Address netmask;
        IPv4Address dns;
        IPv4Address ntp;
        IPv4Address server_id;
        std::string host_name;
        simtime_t lease_time;
        simtime_t renewal_time;
        simtime_t rebind_time;
        Byte parameter_request_list;
        bool leased;

        friend std::ostream& operator <<(std::ostream& os, DHCPLease obj)
        {
            os << "xid:" << obj.xid << " ip:" << obj.ip << " network:" << obj.network << " netmask:" << obj.netmask
                    << " MAC:" << obj.mac << endl;
            return (os);
        }
};
#endif
