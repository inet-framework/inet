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

#include "inet/networklayer/common/L3Address.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/InterfaceToken.h"
#include "inet/common/NotifierConsts.h"

namespace inet {

// Forward declarations. Do NOT #include the corresponding header files
// since that would create dependence on IPv4 and IPv6 stuff!
class InterfaceEntry;
class IInterfaceTable;
class InterfaceProtocolData;
class GenericNetworkProtocolInterfaceData;
class IPv4InterfaceData;
class IPv6InterfaceData;
class TRILLInterfaceData;
class ISISInterfaceData;
class Ieee8021dInterfaceData;

enum McastSourceFilterMode { MCAST_INCLUDE_SOURCES, MCAST_EXCLUDE_SOURCES };

class INET_API MacEstimateCostProcess
{
  public:
    virtual ~MacEstimateCostProcess() {};
    virtual double getCost(int, MACAddress&) = 0;
    virtual double getNumCost() = 0;
    virtual int getNumNeighbors() = 0;
    virtual int getNeighbors(MACAddress[]) = 0;
};

/**
 * Base class for protocol-specific data on an interface.
 * Notable subclasses are IPv4InterfaceData and IPv6InterfaceData.
 */
class INET_API InterfaceProtocolData : public cObject
{
    friend class InterfaceEntry;    //only this guy is allowed to set ownerp

  protected:
    InterfaceEntry *ownerp = nullptr;    // the interface entry this object belongs to

  protected:
    // fires notification with the given signalID, and the interface entry as obj
    virtual void changed(simsignal_t signalID, int fieldId);

  public:
    InterfaceProtocolData() { }

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
    virtual std::string info() const override;
    virtual std::string detailedInfo() const override;
};

/**
 * Interface entry for the interface table in IInterfaceTable.
 *
 * @see IInterfaceTable
 */
class INET_API InterfaceEntry : public cNamedObject
{
    friend class InterfaceProtocolData;    // to call protocolDataChanged()

  public:
    enum State { UP, DOWN, GOING_UP, GOING_DOWN };

  protected:
    IInterfaceTable *ownerp = nullptr;    ///< IInterfaceTable that contains this interface, or nullptr
    cModule *interfaceModule = nullptr;    ///< interface module, or nullptr
    int interfaceId = -1;    ///< identifies the interface in the IInterfaceTable
    int nwLayerGateIndex = -1;    ///< index of ifIn[],ifOut[] gates to that interface (or -1 if virtual interface)
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
    MACAddress macAddr;    ///< link-layer address (for now, only IEEE 802 MAC addresses are supported)
    InterfaceToken token;    ///< for IPv6 stateless autoconfig (RFC 1971), interface identifier (RFC 2462)

    IPv4InterfaceData *ipv4data = nullptr;    ///< IPv4-specific interface info (IPv4 address, etc)
    IPv6InterfaceData *ipv6data = nullptr;    ///< IPv6-specific interface info (IPv6 addresses, etc)
    GenericNetworkProtocolInterfaceData *genericNetworkProtocolData = nullptr;    ///< GenericNetworkProtocol-specific interface info (Address, etc)
    ISISInterfaceData *isisdata = nullptr;    ///< ISIS-specific interface info
    TRILLInterfaceData *trilldata = nullptr;    ///< TRILL-specific interface info
    Ieee8021dInterfaceData *ieee8021ddata = nullptr;
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
        F_IPV4_DATA, F_IPV6_DATA, F_GENERIC_DATA, F_ISIS_DATA, F_TRILL_DATA, F_IEEE8021D_DATA
    };

  protected:
    // change notifications
    virtual void configChanged(int fieldId) { changed(NF_INTERFACE_CONFIG_CHANGED, fieldId); }
    virtual void stateChanged(int fieldId) { changed(NF_INTERFACE_STATE_CHANGED, fieldId); }
    virtual void changed(simsignal_t signalID, int fieldId);

  public:
    // internal: to be invoked from InterfaceTable only!
    virtual void setInterfaceTable(IInterfaceTable *t) { ownerp = t; }
    virtual void setInterfaceId(int id) { interfaceId = id; }
    virtual void resetInterface();

  public:
    InterfaceEntry(cModule *module);
    virtual ~InterfaceEntry();
    virtual std::string info() const override;
    virtual std::string detailedInfo() const override;
    virtual std::string getFullPath() const override;

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

    const ModuleIdAddress getModuleIdAddress() const { return ModuleIdAddress(getInterfaceModule()->getId()); }
    const ModulePathAddress getModulePathAddress() const { return ModulePathAddress(getInterfaceModule()->getId()); }
    const L3Address getNetworkAddress() const;

    /** @name Field getters. Note they are non-virtual and inline, for performance reasons. */
    //@{
    int getInterfaceId() const { return interfaceId; }
    cModule *getInterfaceModule() const { return interfaceModule; }
    int getNetworkLayerGateIndex() const { return nwLayerGateIndex; }
    int getNodeOutputGateId() const { return nodeOutputGateId; }
    int getNodeInputGateId() const { return nodeInputGateId; }
    int getMTU() const { return mtu; }
    bool hasCarrier() const { return carrier; }
    bool isBroadcast() const { return broadcast; }
    bool isMulticast() const { return multicast; }
    bool isPointToPoint() const { return pointToPoint; }
    bool isLoopback() const { return loopback; }
    double getDatarate() const { return datarate; }
    const MACAddress& getMacAddress() const { return macAddr; }
    const InterfaceToken& getInterfaceToken() const { return token; }
    //@}

    /** @name Field setters */
    //@{
    virtual void setName(const char *s) override { cNamedObject::setName(s); configChanged(F_NAME); }
    virtual void setNetworkLayerGateIndex(int i) { if (nwLayerGateIndex != i) { nwLayerGateIndex = i; configChanged(F_NETW_GATEIDX); } }
    virtual void setNodeOutputGateId(int i) { if (nodeOutputGateId != i) { nodeOutputGateId = i; configChanged(F_NODE_OUT_GATEID); } }
    virtual void setNodeInputGateId(int i) { if (nodeInputGateId != i) { nodeInputGateId = i; configChanged(F_NODE_IN_GATEID); } }
    virtual void setMtu(int m) { if (mtu != m) { mtu = m; configChanged(F_MTU); } }
    virtual void setState(State s) { if (state != s) { state = s; stateChanged(F_STATE); } }
    virtual void setCarrier(bool b) { if (carrier != b) { carrier = b; stateChanged(F_CARRIER); } }
    virtual void setBroadcast(bool b) { if (broadcast != b) { broadcast = b; configChanged(F_BROADCAST); } }
    virtual void setMulticast(bool b) { if (multicast != b) { multicast = b; configChanged(F_MULTICAST); } }
    virtual void setPointToPoint(bool b) { if (pointToPoint != b) { pointToPoint = b; configChanged(F_POINTTOPOINT); } }
    virtual void setLoopback(bool b) { if (loopback != b) { loopback = b; configChanged(F_LOOPBACK); } }
    virtual void setDatarate(double d) { if (datarate != d) { datarate = d; configChanged(F_DATARATE); } }
    virtual void setMACAddress(const MACAddress& addr) { if (macAddr != addr) { macAddr = addr; configChanged(F_MACADDRESS); } }
    virtual void setInterfaceToken(const InterfaceToken& t) { token = t; configChanged(F_TOKEN); }
    //@}

    /** @name Accessing protocol-specific interface data. Note methods are non-virtual, for performance reasons. */
    //@{
    IPv4InterfaceData *ipv4Data() const { return ipv4data; }
    IPv6InterfaceData *ipv6Data() const { return ipv6data; }
    GenericNetworkProtocolInterfaceData *getGenericNetworkProtocolData() const { return genericNetworkProtocolData; }
    TRILLInterfaceData *trillData() const { return trilldata; }
    ISISInterfaceData *isisData() const { return isisdata; }
    Ieee8021dInterfaceData *ieee8021dData() const { return ieee8021ddata; }
    //@}

    virtual void joinMulticastGroup(const L3Address& address) const;    // XXX why const method?
    virtual void changeMulticastGroupMembership(const L3Address& multicastAddress,
            McastSourceFilterMode oldFilterMode, const std::vector<L3Address>& oldSourceList,
            McastSourceFilterMode newFilterMode, const std::vector<L3Address>& newSourceList);

    /** @name Installing protocol-specific interface data */
    //@{
    virtual void setIPv4Data(IPv4InterfaceData *p);
    virtual void setIPv6Data(IPv6InterfaceData *p);
    virtual void setGenericNetworkProtocolData(GenericNetworkProtocolInterfaceData *p);
    virtual void setTRILLInterfaceData(TRILLInterfaceData *p);
    virtual void setISISInterfaceData(ISISInterfaceData *p);
    virtual void setIeee8021dInterfaceData(Ieee8021dInterfaceData *p);
    //@}

    /** @name access to the cost process estimation  */
    //@{
    virtual bool setEstimateCostProcess(int, MacEstimateCostProcess *p);
    virtual MacEstimateCostProcess *getEstimateCostProcess(int);
    //@}
};

} // namespace inet

#endif // ifndef __INET_INTERFACEENTRY_H

