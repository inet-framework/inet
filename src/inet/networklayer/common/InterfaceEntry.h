//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_INTERFACEENTRY_H
#define __INET_INTERFACEENTRY_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/tag/TagSet.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/InterfaceToken.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

// Forward declarations. Do NOT #include the corresponding header files
// since that would create dependence on Ipv4 and Ipv6 stuff!
class InterfaceEntry;
class IInterfaceTable;

enum McastSourceFilterMode { MCAST_INCLUDE_SOURCES, MCAST_EXCLUDE_SOURCES };

class INET_API MacEstimateCostProcess
{
  public:
    virtual ~MacEstimateCostProcess() {};
    virtual double getCost(int, MacAddress&) = 0;
    virtual double getNumCost() = 0;
    virtual int getNumNeighbors() = 0;
    virtual int getNeighbors(MacAddress[]) = 0;
};

/**
 * Base class for protocol-specific data on an interface.
 * Notable subclasses are Ipv4InterfaceData and Ipv6InterfaceData.
 */
class INET_API InterfaceProtocolData : public cObject
{
    friend class InterfaceEntry;    //only this guy is allowed to set ownerp

  protected:
    InterfaceEntry *ownerp = nullptr;    // the interface entry this object belongs to
    int id;

  protected:
    // fires notification with the given signalID, and the interface entry as obj
    virtual void changed(simsignal_t signalID, int fieldId);

  public:
    InterfaceProtocolData(int id) : id(id) { }

    /**
     * Returns the InterfaceEntry that contains this data object, or nullptr
     */
    InterfaceEntry *getInterfaceEntry() const { return ownerp; }
};

class INET_API InterfaceEntryChangeDetails : public cObject
{
    InterfaceEntry *ie;
    int field;

  public:
    InterfaceEntryChangeDetails(InterfaceEntry *ie, int field) : ie(ie), field(field) { ASSERT(ie); }
    InterfaceEntry *getInterfaceEntry() const { return ie; }
    int getFieldId() const { return field; }
    virtual std::string str() const override;
};

/**
 * Interface entry for the interface table in IInterfaceTable.
 *
 * @see IInterfaceTable
 */
class INET_API InterfaceEntry : public cModule
{
    friend class InterfaceProtocolData;    // to call protocolDataChanged()

  public:
    enum State { UP, DOWN, GOING_UP, GOING_DOWN };

  protected:
    IInterfaceTable *ownerp = nullptr;    ///< IInterfaceTable that contains this interface, or nullptr
    int interfaceId = -1;    ///< identifies the interface in the IInterfaceTable
    std::string interfaceName;
    int nodeOutputGateId = -1;    ///< id of the output gate of this host/router (or -1 if this is a virtual interface)
    int nodeInputGateId = -1;    ///< id of the input gate of this host/router (or -1 if this is a virtual interface)
    int mtu = 0;    ///< Maximum Transmission Unit (e.g. 1500 on Ethernet); 0 means infinite (i.e. never fragment)
    State state = DOWN;    ///< requested interface state, similar to Linux ifup/ifdown
    bool carrier = false;    ///< current state (up/down) of the physical layer, e.g. Ethernet cable
    bool broadcast = false;    ///< interface supports broadcast
    bool multicast = false;    ///< interface supports multicast
    bool pointToPoint = false;    ///< interface is point-to-point link
    bool loopback = false;    ///< interface is loopback interface
    double datarate = 0;    ///< data rate in bit/s
    MacAddress macAddr;    ///< link-layer address (for now, only IEEE 802 MAC addresses are supported)
    InterfaceToken token;    ///< for Ipv6 stateless autoconfig (RFC 1971), interface identifier (RFC 2462)

    TagSet protocolDataSet;
    std::vector<MacEstimateCostProcess *> estimateCostProcessArray;

  private:
    // copying not supported: following are private and also left undefined
    InterfaceEntry(const InterfaceEntry& obj);
    InterfaceEntry& operator=(const InterfaceEntry& obj);

  public:
    // field ids for change notifications
    enum {
        F_CARRIER, F_STATE,
        F_NAME, F_NODE_IN_GATEID, F_NODE_OUT_GATEID, F_NETW_GATEIDX,
        F_LOOPBACK, F_BROADCAST, F_MULTICAST, F_POINTTOPOINT,
        F_DATARATE, F_MTU, F_MACADDRESS, F_TOKEN,
        F_IPV4_DATA, F_IPV6_DATA, F_NEXTHOP_DATA, F_ISIS_DATA, F_TRILL_DATA, F_IEEE8021D_DATA, F_CLNS_DATA
    };

  protected:
    // change notifications
    virtual void configChanged(int fieldId) { changed(interfaceConfigChangedSignal, fieldId); }
    virtual void stateChanged(int fieldId) { changed(interfaceStateChangedSignal, fieldId); }
    virtual void changed(simsignal_t signalID, int fieldId);

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;

  public:
    // internal: to be invoked from InterfaceTable only!
    virtual void setInterfaceTable(IInterfaceTable *t) { ownerp = t; }
    virtual void setInterfaceId(int id) { interfaceId = id; }
    virtual void resetInterface();

  protected:
    virtual std::string getFullPath() const override { return cModule::getFullPath(); }
    virtual const char *getName() const override { return cModule::getName(); }

  public:
    InterfaceEntry();
    virtual ~InterfaceEntry();
    virtual std::string str() const override;
    virtual std::string getInterfaceFullPath() const;

    /**
     * Returns the IInterfaceTable this interface is in, or nullptr
     */
    IInterfaceTable *getInterfaceTable() const { return ownerp; }

    /**
     * Returns the requested state of this interface.
     */
    State getState() const { return state; }

    /**
     * Returns the combined state of the carrier and the interface requested state.
     */
    bool isUp() const { return getState() == UP && hasCarrier(); }

    const ModuleIdAddress getModuleIdAddress() const { return ModuleIdAddress(getId()); }
    const ModulePathAddress getModulePathAddress() const { return ModulePathAddress(getId()); }
    const L3Address getNetworkAddress() const;
    virtual bool hasNetworkAddress(const L3Address& address) const;

    /** @name Field getters. Note they are non-virtual and inline, for performance reasons. */
    //@{
    int getInterfaceId() const { return interfaceId; }
    const char *getInterfaceName() const { return interfaceName.c_str(); }
    int getNodeOutputGateId() const { return nodeOutputGateId; }
    int getNodeInputGateId() const { return nodeInputGateId; }
    int getMtu() const { return mtu; }
    bool hasCarrier() const { return carrier; }
    bool isBroadcast() const { return broadcast; }
    bool isMulticast() const { return multicast; }
    bool isPointToPoint() const { return pointToPoint; }
    bool isLoopback() const { return loopback; }
    double getDatarate() const { return datarate; }
    const MacAddress& getMacAddress() const { return macAddr; }
    const InterfaceToken& getInterfaceToken() const { return token; }
    //@}

    /** @name Field setters */
    //@{
    virtual void setInterfaceName(const char *s) { interfaceName = s; configChanged(F_NAME); }
    virtual void setNodeOutputGateId(int i) { if (nodeOutputGateId != i) { nodeOutputGateId = i; configChanged(F_NODE_OUT_GATEID); } }
    virtual void setNodeInputGateId(int i) { if (nodeInputGateId != i) { nodeInputGateId = i; configChanged(F_NODE_IN_GATEID); } }
    virtual void setMtu(int m) { if (mtu != m) { mtu = m; configChanged(F_MTU); } }
    virtual void setState(State s);
    virtual void setCarrier(bool b);
    virtual void setBroadcast(bool b) { if (broadcast != b) { broadcast = b; configChanged(F_BROADCAST); } }
    virtual void setMulticast(bool b) { if (multicast != b) { multicast = b; configChanged(F_MULTICAST); } }
    virtual void setPointToPoint(bool b) { if (pointToPoint != b) { pointToPoint = b; configChanged(F_POINTTOPOINT); } }
    virtual void setLoopback(bool b) { if (loopback != b) { loopback = b; configChanged(F_LOOPBACK); } }
    virtual void setDatarate(double d) { if (datarate != d) { datarate = d; configChanged(F_DATARATE); } }
    virtual void setMacAddress(const MacAddress& addr) { if (macAddr != addr) { macAddr = addr; configChanged(F_MACADDRESS); } }
    virtual void setInterfaceToken(const InterfaceToken& t) { token = t; configChanged(F_TOKEN); }
    //@}

    /** @name Tag related functions */
    //@{
    /**
     * Returns the message tag at the given index.
     */
    cObject *getProtocolData(int index) const {
        return protocolDataSet.getTag(index);
    }

    /**
     * Clears the set of message tags.
     */
    void clearProtocolDataSet();

    /**
     * Returns the message tag for the provided type or returns nullptr if no such message tag is found.
     */
    template<typename T> T *findProtocolData() const {
        return protocolDataSet.findTag<T>();
    }

    /**
     * Returns the message tag for the provided type or throws an exception if no such message tag is found.
     */
    template<typename T> T *getProtocolData() const {
        return protocolDataSet.getTag<T>();
    }

    /**
     * Returns a newly added message tag for the provided type, or throws an exception if such a message tag is already present.
     */
    template<typename T> T *addProtocolData() {
        auto t = protocolDataSet.addTag<T>();
        auto ipd = check_and_cast<InterfaceProtocolData *>(t);
        ipd->ownerp = this;
        changed(interfaceConfigChangedSignal, ipd->id);
        return t;
    }

    /**
     * Returns a newly added message tag for the provided type if absent, or returns the message tag that is already present.
     */
    template<typename T> T *addProtocolDataIfAbsent() {
        auto t = protocolDataSet.findTag<T>();
        if (t == nullptr)
            t = protocolDataSet.addTag<T>();
        return t;
    }

    /**
     * Removes the message tag for the provided type, or throws an exception if no such message tag is found.
     */
    template<typename T> T *removeProtocolData() {
        auto t = protocolDataSet.removeTag<T>();
        check_and_cast<InterfaceProtocolData *>(t)->ownerp = nullptr;
        changed(interfaceConfigChangedSignal, t->id);
        return t;
    }

    /**
     * Removes the message tag for the provided type if present, or returns nullptr if no such message tag is found.
     */
    template<typename T> T *removeProtocolDataIfPresent() {
        auto t = protocolDataSet.removeTagIfPresent<T>();
        if (t != nullptr) {
            check_and_cast<InterfaceProtocolData *>(t)->ownerp = nullptr;
            changed(interfaceConfigChangedSignal, t->id);
        }
        return t;
    }
    //@}

    /** @name Accessing protocol-specific interface data. Note methods are non-virtual, for performance reasons. */
    //@{
    Ipv4Address getIpv4Address() const;
    Ipv4Address getIpv4Netmask() const;
    //@}

    virtual void joinMulticastGroup(const L3Address& address) const;    // XXX why const method?
    virtual void changeMulticastGroupMembership(const L3Address& multicastAddress,
            McastSourceFilterMode oldFilterMode, const std::vector<L3Address>& oldSourceList,
            McastSourceFilterMode newFilterMode, const std::vector<L3Address>& newSourceList);

    /** @name access to the cost process estimation  */
    //@{
    virtual bool setEstimateCostProcess(int, MacEstimateCostProcess *p);
    virtual MacEstimateCostProcess *getEstimateCostProcess(int);
    //@}
};

} // namespace inet

#endif // ifndef __INET_INTERFACEENTRY_H

