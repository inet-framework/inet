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
#ifndef __Manet_routing_base_h__
#define __Manet_routing_base_h__

#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include "compatibility.h"
#include "IRoutingTable.h"
#include "NotificationBoard.h"
#include "IInterfaceTable.h"
#include "IPvXAddress.h"
#include "uint128.h"
#include "NotifierConsts.h"
#include "ICMP.h"
#ifdef WITH_80211MESH
#include "ILocator.h"
#endif
#include "ARP.h"
#include <vector>
#include <set>

class ManetRoutingBase;

class ManetTimer :  public cOwnedObject
{
  protected:
    ManetRoutingBase *      agent_; ///< OLSR agent which created the timer.
  public:
    ManetTimer();
    ManetTimer(ManetRoutingBase* agent);
    virtual void expire() = 0;
    virtual void removeQueueTimer();
    virtual void removeTimer();
    virtual void resched(double time);
    virtual void resched(simtime_t time);
    virtual bool isScheduled();
    virtual ~ManetTimer();
};

typedef std::multimap <simtime_t, ManetTimer *> TimerMultiMap;
typedef std::set<Uint128> AddressGroup;
typedef std::set<Uint128>::iterator AddressGroupIterator;
typedef std::set<Uint128>::const_iterator AddressGroupConstIterator;
class INET_API ManetRoutingBase : public cSimpleModule, public INotifiable, protected cListener
{
 public:
    static IPv4Address  LL_MANET_Routers;
    static IPv6Address  LL_MANET_RoutersV6;
  private:
    static simsignal_t mobilityStateChangedSignal;
    typedef std::map<Uint128,Uint128> RouteMap;
    class ProtocolRoutingData
    {
        public:
            RouteMap* routesVector;
            bool isProactive;
    };

    typedef std::vector<ProtocolRoutingData> ProtocolsRoutes;
    typedef std::map<Uint128,ProtocolsRoutes>GlobalRouteMap;
    RouteMap *routesVector;
    static bool createInternalStore;
    static GlobalRouteMap *globalRouteMap;

    IRoutingTable *inet_rt;
    IInterfaceTable *inet_ift;
    NotificationBoard *nb;
    ICMP * icmpModule;
    bool mac_layer_;
    Uint128    hostAddress;
    Uint128    routerId;
    double xPosition;
    double yPosition;
    double xPositionPrev;
    double yPositionPrev;
    simtime_t posTimer;
    simtime_t posTimerPrev;
    bool   regPosition;
    bool   usetManetLabelRouting;
    bool   isRegistered;
    void *commonPtr;
    bool sendToICMP;
    ManetRoutingBase *colaborativeProtocol;

    ARP *arp;

    typedef struct InterfaceIdentification
    {
        InterfaceEntry* interfacePtr;
        int index;
        inline  InterfaceIdentification & operator=(const  InterfaceIdentification& b)
        {
            interfacePtr = b.interfacePtr;
            index = b.index;
            return *this;
        }
    } InterfaceIdentification;
    typedef std::vector <InterfaceIdentification> InterfaceVector;
    InterfaceVector * interfaceVector;
    TimerMultiMap *timerMultiMapPtr;
    cMessage *timerMessagePtr;
    std::vector<AddressGroup> addressGroupVector;
    std::vector<int> inAddressGroup;
    bool staticNode;

    struct ManetProxyAddress
    {
            Uint128 mask;
            Uint128 address;
    };
    bool isGateway;
    std::vector<ManetProxyAddress> proxyAddress;

#ifdef WITH_80211MESH
    ILocator *locator;
#endif

  protected:
    ~ManetRoutingBase();
    ManetRoutingBase();

////////////////////////////////////////////
////////////////////////////////////////////
///     CRITICAL
//      must be executed in the initialize method with stage 4 o bigger, this method initialize all the estructures
/////////////////////////////////////////////
/////////////////////////////////////////////
    virtual void registerRoutingModule();

//
//
    virtual void createTimerQueue();
    virtual void scheduleEvent();
    virtual bool checkTimer(cMessage *msg);


/////////////////////////////////
//
//   encapsulate messages and send to the next layer
//
/////////////////////////////////////
    virtual void sendToIp(cPacket *, int, const Uint128 &, int, int, double, const Uint128 & iface = 0);
    virtual void sendToIp(cPacket *p, int port, int dest, int porDest, int ttl, double delay, int iface = 0)
    {
        sendToIp(p, port, (Uint128) dest, porDest, ttl, delay, (Uint128) iface);
    }
    virtual void sendToIp(cPacket *, int, const Uint128 &, int, int, const Uint128 &iface = 0);
    virtual void sendToIp(cPacket *, int, const Uint128 &, int, int, double, int index = -1);
/////////////////////////////////
//
//   Ip4 routing table access routines
//
/////////////////////////////////////

//
// delete/actualize/insert and record in the routing table
//
    virtual void omnet_chg_rte(const Uint128 &dst, const Uint128 &gtwy, const Uint128 &netm, short int hops, bool del_entry, const Uint128 &iface = 0);
    virtual void omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm, short int hops, bool del_entry);
    virtual void omnet_chg_rte(const struct in_addr &, const struct in_addr &, const struct in_addr &, short int, bool, const struct in_addr &);
    virtual void omnet_chg_rte(const Uint128 &dst, const Uint128 &gtwy, const Uint128 &netm, short int hops, bool del_entry, int);
    virtual void omnet_chg_rte(const struct in_addr &, const struct in_addr &, const struct in_addr &, short int, bool, int);


    virtual void deleteIpEntry(const Uint128 &dst) {omnet_chg_rte(dst, dst, dst, 0, true);}
    virtual void setIpEntry(const Uint128 &dst, const Uint128 &gtwy, const Uint128 &netm, short int hops, const Uint128 &iface = 0) {omnet_chg_rte(dst, gtwy, netm, hops, false, iface);}
//
// Check if it exists in the ip4 routing table the address dst
// if it doesn't exist return ALLONES_ADDRESS
//
    virtual Uint128 omnet_exist_rte(Uint128 dst);

//
// Check if it exists in the ip4 routing table the address dst
// if it doesn't exist return false
//
    virtual bool omnet_exist_rte(struct in_addr dst);
    virtual void omnet_clean_rte();

/////////////////////////
//  Cross layer routines
/////////////////////////

//
// Activate the LLF break
//
    virtual void linkLayerFeeback();
//
//      activate the promiscuous option
//
    virtual void linkPromiscuous();

//      activate the full promiscuous option
//
    virtual void linkFullPromiscuous();
//
//     activate the register position. For position protocols
//     this method must be activated in the stage 0 to register the initial node position
//
    virtual void registerPosition();

//
// Link layer feedback routines
//
    /**
     * @brief Called by the signalling mechanism to inform of changes.
     *
     * ManetRoutingBase is subscribed to position changes.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
    virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void processLinkBreak(const cObject *details);
    virtual void processLinkBreakManagement(const cObject *details);
    virtual void processPromiscuous(const cObject *details);
    virtual void processFullPromiscuous(const cObject *details);
    virtual void processLocatorAssoc(const cObject *details);
    virtual void processLocatorDisAssoc(const cObject *details);

//
//  Replacement for gettimeofday(), used for timers.
//  The timeval should only be interpreted as number of seconds and
//  fractions of seconds since the start of the simulation.
//
    virtual int gettimeofday(struct timeval *, struct timezone *);

////////////////////////
//   Information access routines
////////////////////////

//
//      get the address of the first wlan interface
//
    virtual Uint128 getAddress() const {return hostAddress;}
    virtual Uint128 getRouterId() const {return routerId;}
//
// Return true if the routing protocols is execure in the mac layer
//
    virtual bool isInMacLayer() const {return mac_layer_;}

//
// get the i-esime interface
//
    virtual InterfaceEntry * getInterfaceEntry(int index) const {return inet_ift->getInterface(index);}
    virtual InterfaceEntry * getInterfaceEntryById(int id) const {return inet_ift->getInterfaceById(id);}
//
// Total number of interfaces
//
    virtual int getNumInterfaces() const {return inet_ift->getNumInterfaces();}

// Check if the address is local
    virtual bool isIpLocalAddress(const IPv4Address& dest) const;
    virtual bool isLocalAddress(const Uint128& dest) const;
// Check if the address is multicast
    virtual bool isMulticastAddress(const Uint128& dest) const;

///////////////
// wlan Interface access routines
//////////////////

//
// Get the index of interface with the same address that add
//
    virtual int getWlanInterfaceIndexByAddress(Uint128 = 0);

//
// Get the interface with the same address that add
//
    virtual InterfaceEntry * getInterfaceWlanByAddress(Uint128 = 0) const;

//
// get number wlan interfaces
//
    virtual int getNumWlanInterfaces() const {return interfaceVector->size();}
//
// Get the index used in the general interface table
//
    virtual int getWlanInterfaceIndex(int i) const;
//
// Get the i-esime wlan interface
//
    virtual InterfaceEntry *getWlanInterfaceEntry(int i) const;

    virtual bool isThisInterfaceRegistered(InterfaceEntry *);

//
//     Access to the node position
//
    virtual double getXPos();
    virtual double getYPos();
    virtual double getSpeed();
    virtual double getDirection();

    virtual void getApList(const MACAddress &,std::vector<MACAddress>&);
    virtual void getApListIp(const IPv4Address &,std::vector<IPv4Address>&);
    virtual void getListRelatedAp(const Uint128 &, std::vector<Uint128>&);

  public:
//
    std::string convertAddressToString(const Uint128&);
    virtual void setColaborativeProtocol(cObject *p) {colaborativeProtocol = dynamic_cast<ManetRoutingBase*>(p);}
    virtual ManetRoutingBase * getColaborativeProtocol() const {return colaborativeProtocol;}
    virtual void setStaticNode(bool v) {staticNode=v;}
    virtual bool isStaticNode() {return staticNode;}
// Routing information access
    virtual void setInternalStore(bool i);
    virtual Uint128 getNextHopInternal(const Uint128 &dest);
    virtual bool getInternalStore() const { return createInternalStore;}
    // it should return 0 if not route, if complete route number of hops and, if only next hop
    // it should return -1
    virtual uint32_t getRoute(const Uint128 &, std::vector<Uint128> &) = 0;
    virtual bool getNextHop(const Uint128 &, Uint128 &add, int &iface, double &cost) = 0;
    virtual void setRefreshRoute(const Uint128 &destination, const Uint128 & nextHop,bool isReverse) = 0;
    virtual bool setRoute(const Uint128 & destination, const Uint128 &nextHop, const int &ifaceIndex, const int &hops, const Uint128 &mask = (Uint128)0);
    virtual bool setRoute(const Uint128 & destination, const Uint128 &nextHop, const char *ifaceName, const int &hops, const Uint128 &mask = (Uint128)0);
    virtual bool isProactive() = 0;
    virtual bool isOurType(cPacket *) = 0;
    virtual bool getDestAddress(cPacket *, Uint128 &) = 0;
    virtual bool addressIsForUs(const Uint128 &) const; // return true if the address is local or is in the proxy list
    virtual TimerMultiMap *getTimerMultimMap() const {return timerMultiMapPtr;}
    virtual void setPtr(void *ptr) {commonPtr = ptr;}
    virtual const void * getPtr()const {return commonPtr;}
    virtual void sendICMP(cPacket*);
    virtual bool getSendToICMP() {return sendToICMP;}
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
    virtual void addInAddressGroup(const Uint128&, int group = 0);
    virtual bool delInAddressGroup(const Uint128&, int group = 0);
    virtual bool findInAddressGroup(const Uint128&, int group = 0);
    virtual bool findAddressAndGroup(const Uint128&, int &);
    virtual bool isInAddressGroup(int group = 0);
    virtual bool getAddressGroup(AddressGroup &, int group = 0);
    virtual bool getAddressGroup(std::vector<Uint128> &, int group = 0);
    virtual int  getRouteGroup(const AddressGroup &gr, std::vector<Uint128> &){opp_error("getRouteGroup, method is not implemented"); return 0;}
    virtual bool getNextHopGroup(const AddressGroup &gr, Uint128 &add, int &iface, Uint128&){opp_error("getNextHopGroup, method is not implemented"); return false;}
    virtual int  getRouteGroup(const Uint128&, std::vector<Uint128> &, Uint128&, bool &, int group = 0){opp_error("getRouteGroup, method is not implemented"); return 0;}
    virtual bool getNextHopGroup(const Uint128&, Uint128 &add, int &iface, Uint128&, bool &, int group = 0){opp_error("getNextHopGroup, method is not implemented"); return false;}


    // proxy/gateway methods, this methods help to the reactive protocols to answer the RREQ for a address that are in other subnetwork
    // Set if the node will work like gateway for address in the list
    virtual void setIsGateway(bool p) {isGateway = p;}
    virtual bool getIsGateway() {return isGateway;}
    // return true if the node must answer because the addres are in the list
    virtual bool isAddressInProxyList(const Uint128 &);
    virtual void setAddressInProxyList(const Uint128 &,const Uint128 &);
    virtual int getNumAddressInProxyList() {return (int)proxyAddress.size();}
    virtual bool getAddressInProxyList(int,Uint128 &addr, Uint128 &mask);
    // access to locator information
    virtual bool getAp(const Uint128 &, Uint128 &) const;
    virtual bool isAp() const;
    //
    static bool getRouteFromGlobal(const Uint128 &src, const Uint128 &dest, std::vector<Uint128> &route);
};

#define interface80211ptr getInterfaceWlanByAddress()
#endif

