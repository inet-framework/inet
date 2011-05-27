// ***************************************************************************
// *   Copyright (C) 2009 Alfonso Ariza Quintana aarizaq@uma.es              *
// *                                                                         *
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// *   This program is distributed in the hope that it will be useful,       *
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
// *   GNU General Public License for more details.                          *
// *                                                                         *
// *   You should have received a copy of the GNU General Public License     *
// *   along with this program; if not, write to the                         *
// *   Free Software Foundation, Inc.,                                       *
// *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
// ***************************************************************************

cplusplus {{
#include "uint128.h"
#define DIRECTLINK 0x40
#define UNIDIRECTIONAL 0x80

#ifdef __USE_MINHOP__
#define BatPacketSize 17
#else
#define BatPacketSize 16
#endif

class BatmanHnaMsg
{
   public:
    Uint128 addr;
    unsigned char  netmask;
    BatmanHnaMsg& operator=(const BatmanHnaMsg& m)
    {
           addr=m.addr;
           netmask=m.netmask;
           return *this;
    }
    bool operator < (BatmanHnaMsg const &o) const
    {
        if (addr < o.addr)
            return true;
        else if (addr == o.addr)
        {
            if (netmask < o.netmask)
                return true;
            else
                return false;
        }
        else
            return false;
    }

    bool operator > (BatmanHnaMsg const &o) const
    {
        if (addr > o.addr)
            return true;
        else if (addr == o.addr)
        {
            if (netmask > o.netmask)
                return true;
            else
                return false;
        }
        else
            return false;
    }
    bool operator == (BatmanHnaMsg const &o) const
    {
        if (addr == o.addr && netmask == o.netmask)
            return true;
        else
            return false;
    }
    bool operator != (BatmanHnaMsg const &o) const
    {
        if (addr == o.addr && netmask == o.netmask)
            return false;
        else
            return true;
    }
    BatmanHnaMsg * dup()
    {
        BatmanHnaMsg *aux = new BatmanHnaMsg;
        *aux=*this;
        return aux;
    }
};

struct vis_data
{
    unsigned char type;
    unsigned char data;
    Uint128 ip;
};

}}


class noncobject IPv4Address;
class noncobject BatmanHnaMsg;
class noncobject Uint128;
class noncobject vis_data;

packet BatmanPacket
{
    byteLength = BatPacketSize;
    unsigned char  version;  // batman version field
    unsigned char  flags;    // 0x80: UNIDIRECTIONAL link, 0x40: DIRECTLINK flag, ...
    unsigned char  ttl;
    unsigned char  gatewayFlags;  // flags related to gateway functions: gateway class
    unsigned short seqNumber;
    unsigned short gatewayPort;
    Uint128        orig;
    Uint128        prevSender;
    unsigned char  tq;           // Quality
    unsigned char  hops;
    unsigned char  hnaLen;
    BatmanHnaMsg   hnaMsg[];    ///< Packet body.
}



packet visPacket {
    Uint128 senderIp;
    unsigned char version;
    unsigned char gwClass;
    unsigned char tqMax;
    vis_data visData[];
}


