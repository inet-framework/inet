//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @file EigrpIpv4Pdm.h
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 * @brief EIGRP IPv4 Protocol Dependent Module
 * @detail Main module, it mediates control exchange between DUAL, routing table and
   topology table.
 */

#ifndef __INET_EIGRPIPV4PDM_H
#define __INET_EIGRPIPV4PDM_H

#include <omnetpp.h>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/InterfaceMatcher.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/eigrp/EigrpDeviceConfigurator.h"
#include "inet/routing/eigrp/EigrpDual.h"
#include "inet/routing/eigrp/messages/EigrpMessage_m.h"
#include "inet/routing/eigrp/messages/EigrpMsgReq.h"
#include "inet/routing/eigrp/pdms/EigrpMetricHelper.h"
#include "inet/routing/eigrp/pdms/IEigrpModule.h"
#include "inet/routing/eigrp/pdms/IEigrpPdm.h"
#include "inet/routing/eigrp/tables/EigrpDisabledInterfaces.h"
#include "inet/routing/eigrp/tables/EigrpInterfaceTable.h"
#include "inet/routing/eigrp/tables/EigrpNeighborTable.h"
#include "inet/routing/eigrp/tables/EigrpNetworkTable.h"
#include "inet/routing/eigrp/tables/EigrpRoute.h"
#include "inet/routing/eigrp/tables/EigrpTopologyTable.h"

namespace inet {
namespace eigrp {

class INET_API EigrpIpv4Pdm : public cSimpleModule, public IEigrpModule<Ipv4Address>, public IEigrpPdm<Ipv4Address>, protected cListener
{
  protected:
    typedef std::vector<Ipv4Route *> RouteVector;
    typedef std::vector<EigrpMsgReq *> RequestVector;

    cModule *host = nullptr;

    const int64_t sizeOfMsg = 20;
    const char *SPLITTER_OUTGW;         /**< Output gateway to the EIGRP Splitter module */
    const char *RTP_OUTGW;              /**< Output gateway to the RTP module */
    const Ipv4Address EIGRP_IPV4_MULT; /**< Multicast address for EIGRP messages */
    EigrpKValues KVALUES_MAX;           /**< K-values (from K1 to K5) are set to max */
    const Ipv4Address EIGRP_SELF_ADDR;  /**< Next hop address 0.0.0.0 (self address) */
    EigrpRouteSource<Ipv4Address> *oldsource = nullptr; /**< Latest route change */

    int asNum;                  /**< Autonomous system number */
    EigrpKValues kValues;       /**< K-values for calculation of metric */
    int maximumPath;            /**< Maximum number of parallel routes that EIGRP will support */
    int variance;               /**< Parameter for unequal cost load balancing */
    unsigned int adminDistInt; /**< Administrative distance */
    bool useClassicMetric;      /**< Use classic metric computation or wide metric computation */
    int ribScale;               /**< Scaling factor for Wide metric */
    bool eigrpStubEnabled;      /**< True when EIGRP stub is on */
    EigrpStub eigrpStub;        /**< EIGRP stub configuration */

    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt;

    EigrpDual<Ipv4Address> *eigrpDual;
    EigrpMetricHelper *eigrpMetric;
    EigrpInterfaceTable *eigrpIft;                   /**< Table with enabled EIGRP interfaces */
    EigrpDisabledInterfaces *eigrpIftDisabled;       /**< Disabled EIGRP interfaces */
    EigrpIpv4NeighborTable *eigrpNt;                /**< Table with EIGRP neighbors */
    EigrpIpv4TopologyTable *eigrpTt;                /**< Topology table */
    EigrpNetworkTable<Ipv4Address> *routingForNetworks;          /**< Networks included in EIGRP */
    RequestVector reqQueue;                         /**< Requests for sending EIGRP messages from DUAL */

    void printSentMsg(int routeCnt, Ipv4Address& destAddress, EigrpMsgReq *msgReq);
    void printRecvMsg(const EigrpMessage *msg, Ipv4Address& addr, int ifaceId);

    //-- TIMERS
    /**
     * Creates timer of specified type.
     */
    EigrpTimer *createTimer(char timerKind, void *context);
    /**
     * Sets specified timer to given interval.
     */
    void resetTimer(EigrpTimer *timer, int interval) { cancelEvent(timer); scheduleAt(simTime() + interval /*- uniform(0,0.4)*/, timer); }
    /**
     * Schedule hello timer to the specified interval.
     */
    void startHelloTimer(EigrpInterface *eigrpIface, simtime_t interval);
    /**
     * Stops Hold timer.
     */
    void cancelHoldTimer(EigrpNeighbor<Ipv4Address> *neigh);
    /**
     * Stops Hello Timers on all interfaces.
     */
    void cancelHelloTimers();

    //-- METHODS FOR CREATING MESSAGES
    Packet *createHelloPacket(int holdInt, EigrpKValues kValues, Ipv4Address& destAddress, EigrpMsgReq *msgReq);
    Packet *createAckPacket(Ipv4Address& destAddress, EigrpMsgReq *msgReq);
    Packet *createUpdatePacket(const Ipv4Address& destAddress, EigrpMsgReq *msgReq);
    Packet *createQueryPacket(Ipv4Address& destAddress, EigrpMsgReq *msgReq);
    Packet *createReplyPacket(Ipv4Address& destAddress, EigrpMsgReq *msgReq);

    void addMessageHeader(const Ptr<EigrpMessage>& msg, int opcode, EigrpMsgReq *msgReq);
    void unlockRoutes(const EigrpMsgReq *msgReq);

    void createRouteTlv(EigrpMpIpv4Internal *routeTlv, EigrpRoute<Ipv4Address> *route, bool unreachable = false);
    /**
     * Add routes from request to the message.
     */
    void addRoutesToMsg(const Ptr<EigrpIpv4Message>& msg, const EigrpMsgReq *msgReq);
    void setRouteTlvMetric(EigrpWideMetricPar *msgMetric, EigrpWideMetricPar *rtMetric);
    /**
     * Creates request for sending of EIGRP message for RTP.
     */
    EigrpMsgReq *createMsgReq(HeaderOpcode msgType, int destNeighbor, int destIface);

    //-- PROCESSING MESSAGES
    void processTimer(cMessage *msg);
    /**
     * Process message from network layer.
     */
    void processMsgFromNetwork(cMessage *msg);
    /**
     * Process message request from RTP.
     */
    void processMsgFromRtp(cMessage *msg);
    void processAckPacket(Packet *pk, Ipv4Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv4Address> *neigh);
    void processHelloPacket(Packet *pk, Ipv4Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv4Address> *neigh);
    void processUpdatePacket(Packet *pk, Ipv4Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv4Address> *neigh);
    void processQueryPacket(Packet *pk, Ipv4Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv4Address> *neigh);
    void processReplyPacket(Packet *pk, Ipv4Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv4Address> *neigh);
    /**
     * Process route TLV.
     */
    EigrpRouteSource<Ipv4Address> *processInterRoute(const EigrpMpIpv4Internal& tlv, Ipv4Address& nextHop, int sourceNeighId, EigrpInterface *eigrpIface, bool *notifyDual, bool *isSourceNew);

    //-- NEIGHBORSHIP MANAGEMENT
    /**
     * Creates and sends message with all routes from routing table to specified neighbor.
     */
    void sendAllEigrpPaths(EigrpInterface *eigrpIface, EigrpNeighbor<Ipv4Address> *neigh);
    /**
     * Creates relationship with neighbor.
     * @param srcAddress address of the neighbor
     * @param ifaceId ID of interface where the neighbor is connected
     */
    void processNewNeighbor(int ifaceId, Ipv4Address& srcAddress, const EigrpIpv4Hello *helloMessage);
    /**
     * Checks neighborship rules.
     * @param ifaceId ID of interface where the neighbor is connected.
     * @return returns code from enumeration eigrp::UserMsgCodes.
     */
    int checkNeighborshipRules(int ifaceId, int neighAsNum, Ipv4Address& neighAddr,
            const EigrpKValues& neighKValues);
    /**
     * Create record in the neighbor table and start hold timer.
     */
    EigrpNeighbor<Ipv4Address> *createNeighbor(EigrpInterface *eigrpIface, Ipv4Address& address, uint16_t holdInt);
    /**
     * Removes neighbor from neighbor table and delete it. Notifies DUAL about event.
     */
    void removeNeighbor(EigrpNeighbor<Ipv4Address> *neigh);

    //-- INTERFACE MANAGEMENT
    /**
     * Remove interface from EIGRP interface table. Removes all neighbors on the interface.
     */
    void disableInterface(NetworkInterface *iface, EigrpInterface *eigrpIface, Ipv4Address& ifAddress, Ipv4Address& ifMask);
    /**
     * Add interface to the EIGRP interface table and notifies DUAL.
     */
    void enableInterface(EigrpInterface *eigrpIface, Ipv4Address& ifAddress, Ipv4Address& ifMask, int networkId);
    /**
     * Returns EIGRP interface (enabled or disabled) or nullptr.
     */
    EigrpInterface *getInterfaceById(int ifaceId);
    /**
     * Creates interface and inserts it to the table.
     */
    EigrpInterface *addInterfaceToEigrp(int ifaceId, int networkId, bool enabled);

    //-- PROCESSING EVENTS FROM NOTIFICATION BOARD
    void processIfaceStateChange(NetworkInterface *iface);
    void processIfaceConfigChange(EigrpInterface *eigrpIface);
    void processRTRouteDel(const cObject *details);

    /**
     * Returns next hop address. If next hop in message is 0.0.0.0, then next hop must be
     * replaced by IP address of sender.
     */
    Ipv4Address getNextHopAddr(const Ipv4Address& nextHopAddr, Ipv4Address& senderAddr)
    { return (nextHopAddr.isUnspecified()) ? senderAddr : nextHopAddr; }
    /**
     * Returns IP address for sending EIGRP message.
     */
    bool getDestIpAddress(int destNeigh, Ipv4Address *resultAddress);

    //-- ROUTING TABLE MANAGEMENT
    bool removeRouteFromRT(EigrpRouteSource<Ipv4Address> *successor, IRoute::SourceType *removedRtSrc);
    Ipv4Route *createRTRoute(EigrpRouteSource<Ipv4Address> *successor);
    /**
     * Updates existing route in the routing table or creates new one.
     */
    bool installRouteToRT(EigrpRoute<Ipv4Address> *route, EigrpRouteSource<Ipv4Address> *source, uint64_t dmin, Ipv4Route *rtEntry);
    /**
     * Returns true, if routing table does not contain route with given address, mask and
     * smaller administrative distance.
     */
    bool isRTSafeForAdd(EigrpRoute<Ipv4Address> *route, unsigned int eigrpAd);
    /**
     * Changes metric of route in routing table. For wide metric uses scale.
     */
    void setRTRouteMetric(Ipv4Route *route, uint64_t metric) { if (!useClassicMetric) { metric = metric / ribScale; } route->setMetric(metric); }
    /**
     * Removes route from routing table and changes old successor's record in topology table.
     */
    bool removeOldSuccessor(EigrpRouteSource<Ipv4Address> *source, EigrpRoute<Ipv4Address> *route);

    //-- METHODS FOR MESSAGE REQUESTS
    /**
     * Records request to send message to all neighbors.
     */
    void msgToAllIfaces(int destination, HeaderOpcode msgType, EigrpRouteSource<Ipv4Address> *source, bool forcePoisonRev, bool forceUnreachable);
    /**
     * Creates request for sending message on specified interface.
     */
    void msgToIface(HeaderOpcode msgType, EigrpRouteSource<Ipv4Address> *source, EigrpInterface *eigrpIface, bool forcePoisonRev = false, bool forceUnreachable = false);
    /**
     * Sends all message requests to RTP.
     * */
    void flushMsgRequests();
    /**
     * Insert route into the queue with requests.
     */
    EigrpMsgReq *pushMsgRouteToQueue(HeaderOpcode msgType, int ifaceId, int neighId, const EigrpMsgRoute& msgRt);

    //-- RULES APPLICATION
    /**
     * @return true, if Split Horizon rule is met for the route, otherwise false.
     */
    bool applySplitHorizon(EigrpInterface *destInterface, EigrpRouteSource<Ipv4Address> *source, EigrpRoute<Ipv4Address> *route);
    /**
     * Apply stub configuration to the route in outgoing Update message.
     * @return true, if stub setting limits sending of the route, otherwise false
     */
    bool applyStubToUpdate(EigrpRouteSource<Ipv4Address> *src);
    /**
     * Apply stub configuration to the route in outgoing Query message.
     * @return true, if stub setting limits sending of the route, otherwise false
     */
    bool applyStubToQuery(EigrpInterface *eigrpIface, int numOfNeigh);

    Ipv4Route *findRoute(const Ipv4Address& network, const Ipv4Address& netmask);
    Ipv4Route *findRoute(const Ipv4Address& network, const Ipv4Address& netmask, const Ipv4Address& nexthop);

  public:
    EigrpIpv4Pdm();
    ~EigrpIpv4Pdm();

    //-- INTERFACE IEigrpModule
    virtual void updateInterface(int interfaceId) override { processIfaceConfigChange(this->eigrpIft->findInterfaceById(interfaceId)); }
    void addInterface(int ifaceId, int networkId, bool enabled) override { addInterfaceToEigrp(ifaceId, networkId, enabled); }
    void addInterface(int ifaceId, bool enabled) override { /* useful only for IPv6 */ }
    EigrpNetwork<Ipv4Address> *addNetwork(Ipv4Address address, Ipv4Address mask) override;
    void setASNum(int asNum) override { this->asNum = asNum; }
    int getASNum() override { return this->asNum; }
    void setKValues(const EigrpKValues& kValues) override { this->kValues = kValues; }
    void setMaximumPath(int maximumPath) override { this->maximumPath = maximumPath; }
    void setVariance(int variance) override { this->variance = variance; }
    void setHelloInt(int interval, int ifaceId) override;
    void setHoldInt(int interval, int ifaceId) override;
    void setSplitHorizon(bool shenabled, int ifaceId) override;
    void setPassive(bool passive, int ifaceId) override;
    void setStub(const EigrpStub& stub) override { this->eigrpStub = stub; this->eigrpStubEnabled = true; }
    void setRouterId(Ipv4Address routerID) override { this->eigrpTt->setRouterId(routerID); }
    bool addNetPrefix(const Ipv4Address& network, const short int prefixLen, const int ifaceId) override { return false; /* useful only for IPv6 */ }
    void setLoad(int load, int interfaceId) override { this->eigrpIft->findInterfaceById(interfaceId)->setLoad(load); }
    void setBandwidth(int bandwith, int interfaceId) override { this->eigrpIft->findInterfaceById(interfaceId)->setBandwidth(bandwith); }
    void setDelay(int delay, int interfaceId) override { this->eigrpIft->findInterfaceById(interfaceId)->setDelay(delay); }
    void setReliability(int reliability, int interfaceId) override { this->eigrpIft->findInterfaceById(interfaceId)->setReliability(reliability); }

    //-- INTERFACE IEigrpPdm;
    void sendUpdate(int destNeighbor, EigrpRoute<Ipv4Address> *route, EigrpRouteSource<Ipv4Address> *source, bool forcePoisonRev, const char *reason) override;
    void sendQuery(int destNeighbor, EigrpRoute<Ipv4Address> *route, EigrpRouteSource<Ipv4Address> *source, bool forcePoisonRev = false) override;
    void sendReply(EigrpRoute<Ipv4Address> *route, int destNeighbor, EigrpRouteSource<Ipv4Address> *source, bool forcePoisonRev = false, bool isUnreachable = false) override;
    EigrpRouteSource<Ipv4Address> *updateRoute(EigrpRoute<Ipv4Address> *route, uint64_t dmin, bool *rtableChanged, bool removeUnreach = false) override;
    uint64_t findRouteDMin(EigrpRoute<Ipv4Address> *route) override { return eigrpTt->findRouteDMin(route); }
    bool hasFeasibleSuccessor(EigrpRoute<Ipv4Address> *route, uint64_t& resultDmin) override { return eigrpTt->hasFeasibleSuccessor(route, resultDmin); }
    EigrpRouteSource<Ipv4Address> *getBestSuccessor(EigrpRoute<Ipv4Address> *route) override { return eigrpTt->getBestSuccessor(route); }
    bool setReplyStatusTable(EigrpRoute<Ipv4Address> *route, EigrpRouteSource<Ipv4Address> *source, bool forcePoisonRev, int *neighCount, int *stubCount) override;
    bool hasNeighborForUpdate(EigrpRouteSource<Ipv4Address> *source) override;
    void setDelayedRemove(int neighId, EigrpRouteSource<Ipv4Address> *src) override;
    void sendUpdateToStubs(EigrpRouteSource<Ipv4Address> *succ, EigrpRouteSource<Ipv4Address> *oldSucc, EigrpRoute<Ipv4Address> *route) override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void preDelete(cComponent *root) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
};

} // namespace eigrp
} // namespace inet
#endif

