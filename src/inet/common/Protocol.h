//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_PROTOCOL_H
#define __INET_PROTOCOL_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API Protocol
{
  protected:
    static int nextId;
    static std::map<int, const Protocol *> idToProtocol;
    static std::map<std::string, const Protocol *> nameToProtocol;

  public:
    enum Layer { PhysicalLayer, LinkLayer, NetworkLayer, TransportLayer, UnspecifiedLayer };

  protected:
    const int id;
    const char *name;
    const char *descriptiveName;
    Layer layer;

  public:
    Protocol(const char *name, const char *descriptiveName, Layer layer = UnspecifiedLayer);

    bool operator==(const Protocol& other) const { return id == other.id; }
    bool operator!=(const Protocol& other) const { return id != other.id; }

    int getId() const { return id; }
    const char *getName() const { return name; }
    const char *getDescriptiveName() const { return descriptiveName; }
    Layer getLayer() const { return layer; }

    std::string str() const;

    static const Protocol *findProtocol(int id);
    static const Protocol *getProtocol(int id);

    static const Protocol *findProtocol(const char *name);
    static const Protocol *getProtocol(const char *name);

  public:
    // Standard protocols: (in alphanumeric order)
    static const Protocol aodv;
    static const Protocol arp;
    static const Protocol bgp;
    static const Protocol bmac;
    static const Protocol dsdv2;
    static const Protocol dsr;
    static const Protocol dymo;
    static const Protocol egp;
    static const Protocol ethernetMac;
    static const Protocol ethernetPhy;
    static const Protocol gpsr;
    static const Protocol icmpv4;
    static const Protocol icmpv6;
    static const Protocol ieee80211Mac;
    static const Protocol ieee80211Mgmt;
    static const Protocol ieee80211Phy;
    static const Protocol ieee802154;
    static const Protocol ieee8022;
    static const Protocol igmp;
    static const Protocol igp;
    static const Protocol ipv4;
    static const Protocol ipv6;
    static const Protocol lmac;
    static const Protocol manet;
    static const Protocol mobileipv6;
    static const Protocol mpls;
    static const Protocol ospf;
    static const Protocol pim;
    static const Protocol ppp;
    static const Protocol rsvp;
    static const Protocol sctp;
    static const Protocol stp;
    static const Protocol tcp;
    static const Protocol udp;
    static const Protocol xmac;
    static const Protocol xtp;

    // INET specific protocols: (in alphanumeric order)
    static const Protocol ackingmac;
    static const Protocol apskPhy;
    static const Protocol csmacamac;
    static const Protocol echo;
    static const Protocol flood;
    static const Protocol gnp;
    static const Protocol linkstaterouting;
    static const Protocol probabilistic;
    static const Protocol shortcutMac;
    static const Protocol shortcutPhy;
    static const Protocol unitdisk;
    static const Protocol wiseroute;
};

} // namespace inet

#endif // ifndef __INET_PROTOCOL_H

