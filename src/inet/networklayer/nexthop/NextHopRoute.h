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

#ifndef __INET_NEXTHOPROUTE_H
#define __INET_NEXTHOPROUTE_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IRoute.h"

namespace inet {

class InterfaceEntry;
class IRoutingTable;
class NextHopRoutingTable;

/**
 * A next hop route that uses generic addresses as destination and next hop.
 */
class INET_API NextHopRoute : public cObject, public IRoute
{
  private:
    NextHopRoutingTable *owner;
    int prefixLength;
    L3Address destination;
    L3Address nextHop;
    InterfaceEntry *interface;
    SourceType sourceType;
    cObject *source;
    cObject *protocolData;
    int metric;

  protected:
    void changed(int fieldCode);

  public:
    NextHopRoute() : owner(nullptr), prefixLength(0), interface(nullptr), sourceType(IRoute::MANUAL),
        source(nullptr), protocolData(nullptr), metric(0) {}
    virtual ~NextHopRoute() { delete protocolData; }

    virtual std::string str() const override;
    virtual std::string detailedInfo() const OMNETPP5_CODE(override);

    bool equals(const IRoute& route) const;

    virtual void setRoutingTable(NextHopRoutingTable *owner) { this->owner = owner; }
    virtual void setDestination(const L3Address& dest) override { if (destination != dest) { this->destination = dest; changed(F_DESTINATION); } }
    virtual void setPrefixLength(int l) override { if (prefixLength != l) { this->prefixLength = l; changed(F_PREFIX_LENGTH); } }
    virtual void setNextHop(const L3Address& nextHop) override { if (this->nextHop != nextHop) { this->nextHop = nextHop; changed(F_NEXTHOP); } }
    virtual void setInterface(InterfaceEntry *ie) override { if (interface != ie) { this->interface = ie; changed(F_IFACE); } }
    virtual void setSourceType(SourceType sourceType) override { if (this->sourceType != sourceType) { this->sourceType = sourceType; changed(F_TYPE); } }
    virtual void setSource(cObject *source) override { if (this->source != source) { this->source = source; changed(F_SOURCE); } }
    virtual void setMetric(int metric) override { if (this->metric != metric) { this->metric = metric; changed(F_METRIC); } }
    virtual void setAdminDist(unsigned int adminDist) override {   }
    virtual void setProtocolData(cObject *protocolData) override { this->protocolData = protocolData; }

    /** The routing table in which this route is inserted, or nullptr. */
    virtual IRoutingTable *getRoutingTableAsGeneric() const override;

    /** Destination address prefix to match */
    virtual L3Address getDestinationAsGeneric() const override { return destination; }

    /** Represents length of prefix to match */
    virtual int getPrefixLength() const override { return prefixLength; }

    /** Next hop address */
    virtual L3Address getNextHopAsGeneric() const override { return nextHop; }

    /** Next hop interface */
    virtual InterfaceEntry *getInterface() const override { return interface; }

    /** Source type of the route */
    SourceType getSourceType() const override { return sourceType; }

    /** Source of route */
    virtual cObject *getSource() const override { return source; }

    /** Cost to reach the destination */
    virtual int getMetric() const override { return metric; }

    virtual cObject *getProtocolData() const override { return protocolData; }
};

class INET_API NextHopMulticastRoute : public cObject
{
};

#if 0    /*FIXME TODO!!!! */
/**
 * TODO
 */
class INET_API NextHopMulticastRoute : public cObject, public INextHopMulticastRoute
{
  private:
    struct Child { InterfaceEntry *ie; bool isLeaf; };

  private:
    IRoutingTable *owner;
    bool enabled;
    int prefixLength;
    L3Address origin;
    L3Address multicastGroup;
    InterfaceEntry *parent;
    std::vector<Child> children;
    cObject *source;
    //XXX cObject *protocolData;
    int metric;

  public:
    NextHopMulticastRoute() {}    //TODO
    virtual ~NextHopMulticastRoute() {}

    virtual std::string str() const;
    virtual std::string detailedInfo() const;

    virtual void setEnabled(bool enabled) { this->enabled = enabled; }
    virtual void setOrigin(const L3Address& origin) { this->origin = origin; }
    virtual void setPrefixLength(int len) { this->prefixLength = len; }
    virtual void setMulticastGroup(const L3Address& group) { this->multicastGroup = group; }
    virtual void setParent(InterfaceEntry *ie) { this->parent = ie; }
    virtual bool addChild(InterfaceEntry *ie, bool isLeaf);
    virtual bool removeChild(InterfaceEntry *ie);
    virtual void setSource(cObject *source) { this->source = source; }
    virtual void setMetric(int metric) { this->metric = metric; }

    /** The routing table in which this route is inserted, or nullptr. */
    virtual IRoutingTable *getRoutingTableAsGeneric() { return owner; }

    /** Disabled entries are ignored by routing until the became enabled again. */
    virtual bool isEnabled() const { return enabled; }

    /** Expired entries are ignored by routing, and may be periodically purged. */
    virtual bool isExpired() const { return false; }    //XXX

    /** Source address prefix to match */
    virtual L3Address getOrigin() const { return origin; }

    /** Prefix length to match */
    virtual int getPrefixLength() const { return prefixLength; }

    /** Multicast group address */
    virtual L3Address getMulticastGroup() const { return multicastGroup; }

    /** Parent interface */
    virtual InterfaceEntry *getParent() const { return parent; }

    /** Child interfaces */
    virtual int getNumChildren() const { return children.size(); }

    /** Returns the ith child interface */
    virtual InterfaceEntry *getChild(int i) const { return X; }    //TODO

    /** Returns true if the ith child interface is a leaf */
    virtual bool getChildIsLeaf(int i) const { return X; }    //TODO

    /** Source of route */
    virtual cObject *getSource() const { return source; }

    /** Cost to reach the destination */
    virtual int getMetric() const { return metric; }
};
#endif /*0*/

} // namespace inet

#endif // ifndef __INET_NEXTHOPROUTE_H

