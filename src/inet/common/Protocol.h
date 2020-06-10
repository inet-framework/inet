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
    const std::string name;
    const std::string descriptiveName;
    const Layer layer;

  public:
    Protocol(const char *name, const char *descriptiveName, Layer layer = UnspecifiedLayer);

    bool operator==(const Protocol& other) const { return id == other.id; }
    bool operator!=(const Protocol& other) const { return id != other.id; }

    int getId() const { return id; }
    const char *getName() const { return name.c_str(); }
    const char *getDescriptiveName() const { return descriptiveName.c_str(); }
    Layer getLayer() const { return layer; }

    std::string str() const;

    static const Protocol *findProtocol(int id);
    static const Protocol *getProtocol(int id);

    static const Protocol *findProtocol(const char *name);
    static const Protocol *getProtocol(const char *name);

  public:
    // Standard protocol identifiers (in alphanumeric order)
    static const Protocol aodv;
    static const Protocol arp;
    static const Protocol babel;
    static const Protocol bgp;
    static const Protocol bmac;
    static const Protocol cdp;
    static const Protocol clns;
    static const Protocol dsdv2;
    static const Protocol dsr;
    static const Protocol dymo;
    static const Protocol egp;
    static const Protocol ethernetFlowCtrl;
    static const Protocol ethernetMac;
    static const Protocol ethernetPhy;
    static const Protocol ftp;
    static const Protocol gpsr;
    static const Protocol http;
    static const Protocol icmpv4;
    static const Protocol icmpv6;
    static const Protocol ieee80211EtherType;
    static const Protocol ieee80211Mac;
    static const Protocol ieee80211Mgmt;
    static const Protocol ieee80211FhssPhy;
    static const Protocol ieee80211IrPhy;
    static const Protocol ieee80211DsssPhy;
    static const Protocol ieee80211HrDsssPhy;
    static const Protocol ieee80211OfdmPhy;
    static const Protocol ieee80211ErpOfdmPhy;
    static const Protocol ieee80211HtPhy;
    static const Protocol ieee80211VhtPhy;
    static const Protocol ieee802154;
    static const Protocol ieee8021QCtag;
    static const Protocol ieee8021QStag;
    static const Protocol ieee8022;
    static const Protocol igmp;
    static const Protocol igp;
    static const Protocol ipv4;
    static const Protocol ipv6;
    static const Protocol isis;
    static const Protocol l2isis;
    static const Protocol lldp;
    static const Protocol lmac;
    static const Protocol manet;
    static const Protocol mobileipv6;
    static const Protocol mpls;
    static const Protocol ospf;
    static const Protocol pim;
    static const Protocol ppp;
    static const Protocol rip;
    static const Protocol rsvpTe;
    static const Protocol rtsp;
    static const Protocol sctp;
    static const Protocol srp;
    static const Protocol ssh;
    static const Protocol stp;
    static const Protocol tcp;
    static const Protocol telnet;
    static const Protocol trill;
    static const Protocol tsn;  // ieee1722, AVB, TSN
    static const Protocol tteth;  // SAE AS6802, TTEthernet
    static const Protocol udp;
    static const Protocol xmac;
    static const Protocol xtp;

    // INET specific conceptual protocol identifiers (in alphanumeric order)
    static const Protocol ackingMac;
    static const Protocol apskPhy;
    static const Protocol csmaCaMac;
    static const Protocol echo;
    static const Protocol flooding;
    static const Protocol nextHopForwarding;
    static const Protocol linkStateRouting;
    static const Protocol probabilistic;
    static const Protocol shortcutMac;
    static const Protocol shortcutPhy;
    static const Protocol unitDisk;
    static const Protocol wiseRoute;
};

inline std::ostream& operator << (std::ostream& o, const Protocol& t) { o << t.str(); return o; }

inline void doParsimPacking(cCommBuffer *buffer, const Protocol* protocol) { buffer->pack(protocol->getId()); }
inline void doParsimUnpacking(cCommBuffer *buffer, const Protocol*& protocol) { int id; buffer->unpack(id); protocol = Protocol::getProtocol(id); }

} // namespace inet

#endif // ifndef __INET_PROTOCOL_H

