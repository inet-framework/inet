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
#include "ManetAddress.h"
#include "NotifierConsts.h"
#include "ICMP.h"

#ifdef WITH_80211MESH
#include "ILocator.h"
#endif

#include "ARP.h"

#include <vector>
#include <set>

class ManetRoutingBase;


/**
 * ManetTimer abstract base class
 *
 * Timer for a ManetRoutingBase agent
 *
 * This class uses *timerMultiMapPtr from ManetRoutingBase
 */
class ManetTimer :  public cOwnedObject
{
  protected:
    ManetRoutingBase *agent_; ///< OLSR agent which created the timer.
  public:
    /// Constructor: uses owner as agent, owner of class must be a ManetRoutingBase module
    ManetTimer();

    /// Constructor with specified agent
    ManetTimer(ManetRoutingBase *agent);

    virtual void expire() = 0;

    //FIXME remove one of the next two functions, or define any different of these functions

    /// Remove timer from agent's timerMultiMap queue
    virtual void removeQueueTimer();

    /// Remove timer from agent's timerMultiMap queue, alias for removeQueueTimer()
    virtual void removeTimer();

    /// Reschedule timer in agent, time is relative to current simtime
    virtual void resched(double time);  //FIXME rename it, two functions with same name but absolute/relative time parameters is dangerous

    /// Reschedule timer in agent, time is absolute simtime
    virtual void resched(simtime_t time);

    /// is scheduled this timer in agent?
    virtual bool isScheduled();

    /// Destructor
    virtual ~ManetTimer();
};

typedef std::multimap <simtime_t, ManetTimer *> TimerMultiMap;
typedef std::set<ManetAddress> AddressGroup;

/**
 * Base class for Manet Routing
 */
class INET_API ManetRoutingBase : public cSimpleModule, public INotifiable, protected cListener
{
  private:
    static simsignal_t mobilityStateChangedSignal;

    typedef std::map<ManetAddress,ManetAddress> RouteMap;

    class ProtocolRoutingData
    {
        public:
            RouteMap *routesVector;
            bool isProactive;
    };

    typedef std::vector<ProtocolRoutingData> ProtocolsRoutes;
    typedef std::map<ManetAddress,ProtocolsRoutes>GlobalRouteMap;

    RouteMap *routesVector;
    static bool createInternalStore;
    static GlobalRouteMap *globalRouteMap;

    IRoutingTable *inet_rt;
    IInterfaceTable *inet_ift;
    NotificationBoard *nb;
    ICMP *icmpModule;
    bool mac_layer_;
    ManetAddress    hostAddress;
    ManetAddress    routerId;
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
        InterfaceEntry *interfacePtr;
        int index;
        inline  InterfaceIdentification & operator=(const  InterfaceIdentification& b)
        {
            interfacePtr = b.interfacePtr;
            index = b.index;
            return *this;
        }
    } InterfaceIdentification;
    typedef std::vector <InterfaceIdentification> InterfaceVector;

    InterfaceVector *interfaceVector;

    // variables for ManetTimer class
    TimerMultiMap *timerMultiMapPtr;
    cMessage *timerMessagePtr;

    std::vector<AddressGroup> addressGroupVector;
    std::vector<int> inAddressGroup;
    bool staticNode;

    struct ManetProxyAddress
    {
            ManetAddress mask;
            ManetAddress address;
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

/////////////////////////////////
//
// functions for ManetTimer class
//
/////////////////////////////////
    /// initialize timer queue
    virtual void createTimerQueue();

    /**
     * Cancel timer if already scheduled.
     * Remove expired timers from queue and calls theirs expire() function.
     * schedule first timer.
     * //FIXME rename to rescheduleEvent() ???
     */
    virtual void scheduleEvent();

    /**
     * Checks msg is equals to timerMessagePtr and returns false if not.
     * Remove expired timers from queue and calls theirs expire() function, and returns true.
     */
    virtual bool checkTimer(cMessage *msg);


/////////////////////////////////
//
//   encapsulate messages and send to the next layer
//
/////////////////////////////////////
    void sendToIpOnIface(cPacket *pk, int srcPort, const ManetAddress& destAddr, int destPort, int ttl, double delay, InterfaceEntry *iface);
    virtual void sendToIp(cPacket *pk, int srcPort, const ManetAddress& destAddr, int destPort, int ttl, double delay, const ManetAddress& ifaceAddr);
    virtual void sendToIp(cPacket *pk, int srcPort, const ManetAddress& destAddr, int destPort, int ttl, double delay, int ifaceIndex = -1);

/////////////////////////////////
//
//   Ip4 routing table access routines
//
/////////////////////////////////////

    /**
     *  @name delete/actualize/insert and record in the routing table
     */
    //@{
    //FIXME reduce these variations
    virtual void omnet_chg_rte(const ManetAddress &dst, const ManetAddress &gtwy, const ManetAddress &netm, short int hops, bool del_entry, const ManetAddress &iface = ManetAddress::ZERO);
    virtual void omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm, short int hops, bool del_entry);
    virtual void omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm, short int hops, bool del_entry, const struct in_addr &iface);
    virtual void omnet_chg_rte(const ManetAddress &dst, const ManetAddress &gtwy, const ManetAddress &netm, short int hops, bool del_entry, int index);
    virtual void omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm, short int hops, bool del_entry, int index);

    virtual void deleteIpEntry(const ManetAddress &dst) {omnet_chg_rte(dst, dst, dst, 0, true);}
    virtual void setIpEntry(const ManetAddress &dst, const ManetAddress &gtwy, const ManetAddress &netm, short int hops, const ManetAddress &iface = ManetAddress::ZERO) {omnet_chg_rte(dst, gtwy, netm, hops, false, iface);}
    //@}

    /**
     * Check if it exists in the ip4 routing table the address dst
     * if it exist return gateway address
     * if it doesn't exist return ALLONES_ADDRESS
     */
    virtual ManetAddress omnet_exist_rte(ManetAddress dst);

    /**
     * Check if it exists in the ip4 routing table the address dst
     * if it doesn't exist return false
     */
    virtual bool omnet_exist_rte(struct in_addr dst);
    virtual void omnet_clean_rte();

    /**
     *  @name Cross layer routines
     */
    //@{

    /// Activate the LLF break
    virtual void linkLayerFeeback();

    /// activate the promiscuous option
    virtual void linkPromiscuous();

    /// Activate the full promiscuous option
    virtual void linkFullPromiscuous();

    /**
     * activate the register position. For position protocols
     * this method must be activated in the stage 0 to register the initial node position
     */
    virtual void registerPosition();
    //@}

    /**
     * Link layer feedback routines
     */
    //@{
    /**
     * @brief Called by the signaling mechanism to inform of changes.
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
    //@}

    /**
     *  Replacement for gettimeofday(), used for timers.
     *  The timeval should only be interpreted as number of seconds and
     *  fractions of seconds since the start of the simulation.
     */
    virtual int gettimeofday(struct timeval *, struct timezone *);

    /// Get the address of the first wlan interface
    virtual ManetAddress getAddress() const {return hostAddress;}

    virtual ManetAddress getRouterId() const {return routerId;}

    /// Return true if the routing protocols is execute in the mac layer
    virtual bool isInMacLayer() const {return mac_layer_;}

    /// get the i-esime interface
    virtual InterfaceEntry *getInterfaceEntry(int index) const {return inet_ift->getInterface(index);}
    virtual InterfaceEntry *getInterfaceEntryById(int id) const {return inet_ift->getInterfaceById(id);}

    /// Total number of interfaces
    virtual int getNumInterfaces() const {return inet_ift->getNumInterfaces();}

    /// Check if the address is local
    virtual bool isLocalAddress(const ManetAddress& dest) const;

    /// Check if the address is multicast
    virtual bool isMulticastAddress(const ManetAddress& dest) const;

    /// @name wlan interface access routines
    //@{
    /// Get the index of interface with the same address that add
    virtual int getWlanInterfaceIndexByAddress(ManetAddress = ManetAddress::ZERO);

    /// Get the interface with the same address that add
    virtual InterfaceEntry *getInterfaceWlanByAddress(ManetAddress = ManetAddress::ZERO) const;

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
    virtual double getXPos();
    virtual double getYPos();
    virtual double getSpeed();
    virtual double getDirection();
    //@}

    virtual void getApList(const MACAddress &,std::vector<MACAddress>&);
    virtual void getApListIp(const IPv4Address &,std::vector<IPv4Address>&);
    virtual void getListRelatedAp(const ManetAddress &, std::vector<ManetAddress>&);

  public:
    std::string convertAddressToString(const ManetAddress&);
    virtual void setColaborativeProtocol(cObject *p) {colaborativeProtocol = dynamic_cast<ManetRoutingBase*>(p);}
    virtual ManetRoutingBase *getColaborativeProtocol() const {return colaborativeProtocol;}
    virtual void setStaticNode(bool v) {staticNode=v;}
    virtual bool isStaticNode() {return staticNode;}

    // Routing information access
    virtual void setInternalStore(bool i);
    virtual ManetAddress getNextHopInternal(const ManetAddress &dest);
    virtual bool getInternalStore() const { return createInternalStore;}

    // it should return 0 if not route, if complete route number of hops and, if only next hop
    // it should return -1
    virtual uint32_t getRoute(const ManetAddress &, std::vector<ManetAddress> &) = 0;
    virtual bool getNextHop(const ManetAddress &, ManetAddress &add, int &iface, double &cost) = 0;
    virtual void setRefreshRoute(const ManetAddress &destination, const ManetAddress & nextHop,bool isReverse) = 0;

    // set/delete routing entry
    //FIXME nextHop.isUnspecified() means: need delete entry. Should add a new parameter for choose set/delete, should rename function
    //FIXME setRoute() vs omnet_chg_rte()
    virtual bool setRoute(const ManetAddress & destination, const ManetAddress &nextHop, const int &ifaceIndex, const int &hops, const ManetAddress &mask = ManetAddress::ZERO);
    virtual bool setRoute(const ManetAddress & destination, const ManetAddress &nextHop, const char *ifaceName, const int &hops, const ManetAddress &mask = ManetAddress::ZERO);

    virtual bool isProactive() = 0;
    virtual bool isOurType(cPacket *) = 0;
    virtual bool getDestAddress(cPacket *, ManetAddress &) = 0;
    virtual bool addressIsForUs(const ManetAddress &) const; // return true if the address is local or is in the proxy list
    virtual TimerMultiMap *getTimerMultimMap() const {return timerMultiMapPtr;}
    virtual void setPtr(void *ptr) {commonPtr = ptr;}
    virtual const void *getPtr()const {return commonPtr;}
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
    virtual void addInAddressGroup(const ManetAddress&, int group = 0);
    virtual bool delInAddressGroup(const ManetAddress&, int group = 0);
    virtual bool findInAddressGroup(const ManetAddress&, int group = 0);
    virtual bool findAddressAndGroup(const ManetAddress&, int &);
    virtual bool isInAddressGroup(int group = 0);
    virtual bool getAddressGroup(AddressGroup &, int group = 0);
    virtual bool getAddressGroup(std::vector<ManetAddress> &, int group = 0);
    virtual int  getRouteGroup(const AddressGroup &gr, std::vector<ManetAddress> &){opp_error("getRouteGroup, method is not implemented"); return 0;}
    virtual bool getNextHopGroup(const AddressGroup &gr, ManetAddress &add, int &iface, ManetAddress&){opp_error("getNextHopGroup, method is not implemented"); return false;}
    virtual int  getRouteGroup(const ManetAddress&, std::vector<ManetAddress> &, ManetAddress&, bool &, int group = 0){opp_error("getRouteGroup, method is not implemented"); return 0;}
    virtual bool getNextHopGroup(const ManetAddress&, ManetAddress &add, int &iface, ManetAddress&, bool &, int group = 0){opp_error("getNextHopGroup, method is not implemented"); return false;}


    // proxy/gateway methods, this methods help to the reactive protocols to answer the RREQ for a address that are in other subnetwork

    /// Set if the node will work like gateway for address in the list
    virtual void setIsGateway(bool p) {isGateway = p;}

    virtual bool getIsGateway() {return isGateway;}

    /// return true if the node must answer because the address are in the list
    virtual bool isAddressInProxyList(const ManetAddress& addr);

    virtual void setAddressInProxyList(const ManetAddress& addr, const ManetAddress& mask);
    virtual int getNumAddressInProxyList() {return (int)proxyAddress.size();}

    /// get i-th address/mask pair from proxyAddress vector
    virtual bool getAddressInProxyList(int i, ManetAddress &outAddr, ManetAddress &outMask);

    /**
     * access to locator information
     */
    virtual bool getAp(const ManetAddress& destination, ManetAddress& outAccesPointAddr) const;
    virtual bool isAp() const;
    //
    static bool getRouteFromGlobal(const ManetAddress &src, const ManetAddress &dest, std::vector<ManetAddress> &route);
};

#define interface80211ptr getInterfaceWlanByAddress()
#endif

