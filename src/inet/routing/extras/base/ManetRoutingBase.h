/***************************************************************************
 *   Copyright (C) 2008 by Alfonso Ariza                                   *
 *   aarizaq@uma.es                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

///
#ifndef __INET_MANETROUTINGBASE
#define __INET_MANETROUTINGBASE

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#include "inet/routing/extras/base/compatibility.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/extras/base/ManetNetfilterHook.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/ICMP.h"
#include "inet/networklayer/ipv4/IPv4.h"

#include <vector>
#include <set>

namespace inet {

class IARP;

namespace inetmanet {

class ManetRoutingBase;

typedef std::set<L3Address> AddressGroup;
typedef std::set<L3Address>::iterator AddressGroupIterator;
typedef std::set<L3Address>::const_iterator AddressGroupConstIterator;

/**
 * Base class for Manet Routing
 */
class INET_API ManetRoutingBase : public cSimpleModule, public cListener, public ManetNetfilterHook
{
  private:
    static simsignal_t mobilityStateChangedSignal;

    typedef std::map<L3Address,L3Address> RouteMap;

    class ProtocolRoutingData
    {
        public:
            RouteMap* routesVector;
            bool isProactive;
    };

    typedef std::vector<ProtocolRoutingData> ProtocolsRoutes;
    typedef std::map<L3Address,ProtocolsRoutes>GlobalRouteMap;

    RouteMap *routesVector;
    static bool createInternalStore;
    static GlobalRouteMap *globalRouteMap;

    IIPv4RoutingTable *inet_rt;
    IInterfaceTable *inet_ift;
    cModule *hostModule;
    ICMP *icmpModule;
    bool mac_layer_;
    L3Address    hostAddress;
    L3Address    routerId;
    Coord curPosition;
    Coord curSpeed;
    simtime_t posTimer;
    bool   regPosition;
    bool   useManetLabelRouting;
    bool   isRegistered;
    void *commonPtr;
    bool sendToICMP;
    ManetRoutingBase *collaborativeProtocol;

    IARP *arp;

    typedef struct InterfaceIdentification
    {
        InterfaceEntry *interfacePtr;
        int index;
        inline InterfaceIdentification& operator=(const InterfaceIdentification& b)
        {
            interfacePtr = b.interfacePtr;
            index = b.index;
            return *this;
        }
    } InterfaceIdentification;
    typedef std::vector <InterfaceIdentification> InterfaceVector;

    InterfaceVector *interfaceVector;

    std::vector<AddressGroup> addressGroupVector;
    std::vector<int> inAddressGroup;
    bool staticNode;

    struct ManetProxyAddress
    {
        L3Address mask;
        L3Address address;
    };

    bool isGateway;     /// true if the node will work like gateway for address in the list

    std::vector<ManetProxyAddress> proxyAddress;

  protected:
    ~ManetRoutingBase();
    ManetRoutingBase();

////////////////////////////////////////////
//
//   to be called in INITSTAGE_ROUTING_PROTOCOLS
//
/////////////////////////////////////////////
    virtual void registerRoutingModule();

/////////////////////////////////
//
//   encapsulate messages and send to the next layer
//
/////////////////////////////////////
    void sendToIpOnIface(cPacket *pk, int srcPort, const L3Address& destAddr, int destPort, int ttl, double delay, InterfaceEntry *iface);
    virtual void sendToIp(cPacket *pk, int srcPort, const L3Address& destAddr, int destPort, int ttl, double delay, const L3Address& ifaceAddr);
    virtual void sendToIp(cPacket *pk, int srcPort, const L3Address& destAddr, int destPort, int ttl, double delay, int ifaceIndex = -1);

/////////////////////////////////
//
//   Ip4 routing table access routines
//
/////////////////////////////////////

    /**
     *  @name delete/actualize/insert and record in the IPv4 routing table
     */
    //@{
    //FIXME reduce these variations
    virtual void omnet_chg_rte(const L3Address &dst, const L3Address &gtwy, const L3Address &netm, short int hops, bool del_entry, const L3Address &iface = L3Address());
    virtual void omnet_chg_rte(const L3Address &dst, const L3Address &gtwy, const L3Address &netm, short int hops, bool del_entry, int index);
    virtual void omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm, short int hops, bool del_entry);
    virtual void omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm, short int hops, bool del_entry, const struct in_addr &iface);
    virtual void omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm, short int hops, bool del_entry, int index);

    virtual void deleteIpEntry(const L3Address &dst) {omnet_chg_rte(dst, dst, dst, 0, true);}
    virtual void setIpEntry(const L3Address &dst, const L3Address &gtwy, const L3Address &netm, short int hops, const L3Address &iface = L3Address())
            {omnet_chg_rte(dst, gtwy, netm, hops, false, iface);}
    //@}

    /**
     * Check existing the dst address in the IPv4 routing table
     * if it exists, returns gateway address
     * if it doesn't exists, returns ALLONES_ADDRESS
     * if uses mac_layer, returns ZERO
     */
    virtual L3Address omnet_exist_rte(L3Address dst);     //FIXME revise return values

    /**
     * Check existing the dst address in the IPv4 routing table
     * if it doesn't exists, return false
     */
    virtual bool omnet_exist_rte(struct in_addr dst);   //FIXME remove it, use the another version

    /// Erase all entries for wlan* interfaces in the routing table
    virtual void omnet_clean_rte();

    /**
     *  @name Cross layer routines
     */
    //@{

    /// Activate the LLF break (subscribe to NF_LINK_BREAK)
    virtual void linkLayerFeeback();

    /// activate the promiscuous option (subscribe to NF_LINK_PROMISCUOUS)
    virtual void linkPromiscuous();

    /// Activate the full promiscuous option (subscribe to NF_LINK_FULL_PROMISCUOUS)
    virtual void linkFullPromiscuous();

    /**
     * Activate the register position (subscribe to mobilityStateChanged signal).
     */
    virtual void registerPosition();
    //@}

    /** Link layer feedback routines */
    //@{
    /**
     * @brief Called by the signaling mechanism to inform of changes.
     *
     * ManetRoutingBase is subscribed to position changes.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
    virtual void processLinkBreak(const cObject *details);
    virtual void processLinkBreakManagement(const cObject *details);
    virtual void processPromiscuous(const cObject *details);
    virtual void processFullPromiscuous(const cObject *details);
    virtual void processLocatorAssoc(const cObject *details);
    virtual void processLocatorDisAssoc(const cObject *details);
    //@}

    /**
     *  Replacement for gettimeofday(), used for timers.
     *  The timeval should only be interpreted as number of seconds and
     *  fractions of seconds since the start of the simulation.
     */
    virtual int gettimeofday(struct timeval *, struct timezone *);

    /// Get the address of the first wlan interface
    virtual L3Address getAddress() const {return hostAddress;}

    virtual L3Address getRouterId() const {return routerId;}

    /// Return true if the routing protocols is execute in the mac layer
    virtual bool isInMacLayer() const {return mac_layer_;}

    /// get the i-esime interface
    virtual InterfaceEntry *getInterfaceEntry(int index) const {return inet_ift->getInterface(index);}
    virtual InterfaceEntry *getInterfaceEntryById(int id) const {return inet_ift->getInterfaceById(id);}

    /// Total number of interfaces
    virtual int getNumInterfaces() const {return inet_ift->getNumInterfaces();}

    /// Check if the address is local

    virtual bool isIpLocalAddress(const IPv4Address& dest) const;
    virtual bool isLocalAddress(const L3Address& dest) const;

    /// Check if the address is multicast
    virtual bool isMulticastAddress(const L3Address& dest) const;

    /// @name wlan interface access routines
    //@{
    /// Get the index of interface with the same address that add
    virtual int getWlanInterfaceIndexByAddress(L3Address = L3Address());

    /// Get the interface with the same address that add
    virtual InterfaceEntry *getInterfaceWlanByAddress(L3Address = L3Address()) const;

    /// get number of wlan interfaces
    virtual int getNumWlanInterfaces() const {return interfaceVector->size();}

    /// Get the index used in the general interface table
    virtual int getWlanInterfaceIndex(int i) const;

    /// Get the i-esime wlan interface
    virtual InterfaceEntry *getWlanInterfaceEntry(int i) const;

    /// Returns true if ie found in the general interface table
    virtual bool isThisInterfaceRegistered(InterfaceEntry *ie);
    //@}

    /// @name Access to the node position
    //@{
    virtual const Coord& getPosition();
    virtual double getSpeed();
    virtual const Coord& getDirection();  //FIXME rename?
    //@}

    //FIXME 3 variations for do the same, reduce it
    virtual void getApList(const MACAddress &,std::vector<MACAddress>&);
    virtual void getApListIp(const IPv4Address &,std::vector<IPv4Address>&);
    virtual void getListRelatedAp(const L3Address &, std::vector<L3Address>&);
    virtual void setRouteInternalStorege(const L3Address &, const L3Address &, const bool &);

  public:
    std::string convertAddressToString(const L3Address&);
    virtual void setCollaborativeProtocol(cObject *p) {collaborativeProtocol = dynamic_cast<ManetRoutingBase*>(p);}
    virtual ManetRoutingBase * getCollaborativeProtocol() const {return collaborativeProtocol;}
    virtual void setStaticNode(bool v) {staticNode=v;}
    virtual bool isStaticNode() {return staticNode;}
// Routing information access
    virtual void setInternalStore(bool i);
    virtual L3Address getNextHopInternal(const L3Address &dest);
    virtual bool getInternalStore() const { return createInternalStore;}
    // it should return 0 if not route, if complete route number of hops and, if only next hop
    // it should return -1
    // if the protocol has implemented supportGetRoute()
    virtual bool supportGetRoute () = 0;

    /**
     * Get list of hops for routing to dest destination
     * it should return 0 if not route,
     *    number of hops if complete route,
     *    -1 if only next hop
     */
    virtual uint32_t getRoute(const L3Address& dest, std::vector<L3Address>& hopsList) = 0;   //FIXME use int instead uint32_t for return value

    /**
     * Get next hop address.
     * Returns true if found next hop for dest destination,
     * and set nextHop to next hop, ifaceID to interface ID, and cost to cost value
     */
    virtual bool getNextHop(const L3Address& dest, L3Address& nextHop, int& ifaceId, double& cost) = 0;

    /**
     * Update timeout of specified routing entry
     *   //FIXME what is the isReverse parameter?
     */
    virtual void setRefreshRoute(const L3Address& dest, const L3Address& nextHop, bool isReverse) = 0;

    // set/delete routing entry
    //FIXME nextHop.isUnspecified() means: need delete entry. Should add a new parameter for choose set/delete, should rename function
    //FIXME setRoute() vs omnet_chg_rte()
    virtual bool setRoute(const L3Address & destination, const L3Address &nextHop, const int &ifaceIndex, const int &hops, const L3Address &mask = L3Address());
    virtual bool setRoute(const L3Address & destination, const L3Address &nextHop, const char *ifaceName, const int &hops, const L3Address &mask = L3Address());

    virtual bool isProactive() = 0;

    /// Returns true if type of pk is our type.
    virtual bool isOurType(cPacket *pk) = 0;

    /**
     *  Get destination address from pk packet.
     *  returns true and fill dest parameter from pk if pk is our type.
     */
    virtual bool getDestAddress(cPacket *pk, L3Address &dest) = 0;

    /// Returns true if the address is local or is in the proxy list
    virtual bool addressIsForUs(const L3Address &) const;

    /// set/get commonPtr member.
    virtual void setPtr(void *ptr) {commonPtr = ptr;}
    virtual const void *getPtr()const {return commonPtr;}

    /// send an ICMP DEST UNREACHABLE error based on pk
    virtual void sendICMP(cPacket *pk);

    /// returns true if ICMP error sending is enabled
    virtual bool getSendToICMP() {return sendToICMP;}

    /// enable/disable ICMP error sending
    virtual void setSendToICMP(bool val)
    {
        if (icmpModule)
            sendToICMP = val;
        else
            sendToICMP = false;
    }

    // group address, it's similar to anycast
    virtual int  getNumGroupAddress(){return addressGroupVector.size();}
    virtual int  getNumAddressInAGroups(int group = 0);
    virtual void addInAddressGroup(const L3Address& addr, int group = 0);
    virtual bool delInAddressGroup(const L3Address& addr, int group = 0);
    virtual bool findInAddressGroup(const L3Address& addr, int group = 0);

    /// find address in all groups and if found, returns true and set group to index of group that contains the addr
    virtual bool findAddressAndGroup(const L3Address& addr, int &group);

    virtual bool isInAddressGroup(int group = 0);

    //FIXME why duplicated the next functions?
    virtual bool getAddressGroup(AddressGroup &, int group = 0);
    virtual bool getAddressGroup(std::vector<L3Address> &, int group = 0);

    virtual int  getRouteGroup(const AddressGroup &gr, std::vector<L3Address> &) { throw cRuntimeError("getRouteGroup, method is not implemented"); }
    virtual int  getRouteGroup(const L3Address&, std::vector<L3Address> &, L3Address&, bool &, int group = 0) { throw cRuntimeError("getRouteGroup, method is not implemented"); }

    virtual bool getNextHopGroup(const AddressGroup &gr, L3Address &add, int &iface, L3Address&) { throw cRuntimeError("getNextHopGroup, method is not implemented"); }
    virtual bool getNextHopGroup(const L3Address&, L3Address &add, int &iface, L3Address&, bool &, int group = 0) { throw cRuntimeError("getNextHopGroup, method is not implemented"); }


    /// proxy/gateway methods, this methods help to the reactive protocols to answer the RREQ for a address that are in other subnetwork
    //@{
    /// Set if the node will work like gateway for address in the list
    virtual void setIsGateway(bool p) {isGateway = p;}

    virtual bool getIsGateway() {return isGateway;}

    /// return true if the node must answer because the address is in the list
    virtual bool isAddressInProxyList(const L3Address& addr);

    virtual void setAddressInProxyList(const L3Address& addr, const L3Address& mask);

    virtual int getNumAddressInProxyList() {return (int)proxyAddress.size();}

    /// get i-th address/mask pair from proxyAddress vector
    virtual bool getAddressInProxyList(int i, L3Address &outAddr, L3Address &outMask);
    //@}

    /**
     * access to locator information
     */
    virtual bool getAp(const L3Address& destination, L3Address& outAccesPointAddr) const;
    virtual bool isAp() const;
    //
    static bool getRouteFromGlobal(const L3Address &src, const L3Address &dest, std::vector<L3Address> &route);
};

#define interface80211ptr getInterfaceWlanByAddress()

} // namespace inetmanet

} // namespace inet


#endif

