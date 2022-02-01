//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2INTERFACE_H
#define __INET_OSPFV2INTERFACE_H

#include <list>
#include <map>
#include <vector>

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2Neighbor.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"

namespace inet {

namespace ospfv2 {

class Ospfv2Area;
class Ospfv2InterfaceState;

class INET_API Ospfv2Interface
{
  public:
    enum Ospfv2InterfaceType {
        UNKNOWN_TYPE      = 0,
        POINTTOPOINT      = 1,
        BROADCAST         = 2,
        NBMA              = 3,
        POINTTOMULTIPOINT = 4,
        VIRTUAL           = 5
    };

    enum Ospfv2InterfaceEventType {
        INTERFACE_UP          = 0,
        HELLO_TIMER           = 1,
        WAIT_TIMER            = 2,
        ACKNOWLEDGEMENT_TIMER = 3,
        BACKUP_SEEN           = 4,
        NEIGHBOR_CHANGE       = 5,
        LOOP_INDICATION       = 6,
        UNLOOP_INDICATION     = 7,
        INTERFACE_DOWN        = 8
    };

    enum Ospfv2InterfaceStateType {
        DOWN_STATE                  = 0,
        LOOPBACK_STATE              = 1,
        WAITING_STATE               = 2,
        POINTTOPOINT_STATE          = 3,
        NOT_DESIGNATED_ROUTER_STATE = 4,
        BACKUP_STATE                = 5,
        DESIGNATED_ROUTER_STATE     = 6
    };

    enum Ospfv2InterfaceMode {
        ACTIVE  = 0,
        PASSIVE = 1,
        NO_OSPF = 2,
    };

  private:
    Ospfv2InterfaceType interfaceType;
    Ospfv2InterfaceMode interfaceMode;
    CrcMode crcMode;
    Ospfv2InterfaceState *state;
    Ospfv2InterfaceState *previousState;
    std::string interfaceName;
    int ifIndex;
    unsigned short mtu;
    Ipv4AddressRange interfaceAddressRange;
    AreaId areaID;
    AreaId transitAreaID;
    short helloInterval;
    short pollInterval;
    short routerDeadInterval;
    short interfaceTransmissionDelay;
    unsigned char routerPriority;
    cMessage *helloTimer;
    cMessage *waitTimer;
    cMessage *acknowledgementTimer;
    std::map<RouterId, Neighbor *> neighboringRoutersByID;
    std::map<Ipv4Address, Neighbor *> neighboringRoutersByAddress;
    std::vector<Neighbor *> neighboringRouters;
    std::map<Ipv4Address, std::list<Ospfv2LsaHeader>> delayedAcknowledgements;
    DesignatedRouterId designatedRouter;
    DesignatedRouterId backupDesignatedRouter;
    Metric interfaceOutputCost;
    short retransmissionInterval;
    short acknowledgementDelay;
    AuthenticationType authenticationType;
    AuthenticationKeyType authenticationKey;

    Ospfv2Area *parentArea;

  private:
    friend class Ospfv2InterfaceState;
    void changeState(Ospfv2InterfaceState *newState, Ospfv2InterfaceState *currentState);

  public:
    Ospfv2Interface(Ospfv2InterfaceType ifType = UNKNOWN_TYPE);
    virtual ~Ospfv2Interface();

    void processEvent(Ospfv2InterfaceEventType event);
    void reset();
    void sendHelloPacket(Ipv4Address destination, short ttl = 1);
    void sendLsAcknowledgement(const Ospfv2LsaHeader *lsaHeader, Ipv4Address destination);
    Neighbor *getNeighborById(RouterId neighborID);
    Neighbor *getNeighborByAddress(Ipv4Address address);
    void addNeighbor(Neighbor *neighbor);
    Ospfv2InterfaceStateType getState() const;
    static const char *getStateString(Ospfv2InterfaceStateType stateType);
    bool hasAnyNeighborInStates(int states) const;
    void removeFromAllRetransmissionLists(LsaKeyType lsaKey);
    bool isOnAnyRetransmissionList(LsaKeyType lsaKey) const;
    bool floodLsa(const Ospfv2Lsa *lsa, Ospfv2Interface *intf = nullptr, Neighbor *neighbor = nullptr);
    void addDelayedAcknowledgement(const Ospfv2LsaHeader& lsaHeader);
    void sendDelayedAcknowledgements();
    void ageTransmittedLsaLists();

    Packet *createUpdatePacket(const Ospfv2Lsa *lsa);

    void setType(Ospfv2InterfaceType ifType) { interfaceType = ifType; }
    Ospfv2InterfaceType getType() const { return interfaceType; }
    static const char *getTypeString(Ospfv2InterfaceType intfType);
    Ospfv2InterfaceMode getMode() const { return interfaceMode; }
    static const char *getModeString(Ospfv2InterfaceMode intfMode);
    void setMode(Ospfv2InterfaceMode intfMode) { interfaceMode = intfMode; }
    CrcMode getCrcMode() const { return crcMode; }
    void setCrcMode(CrcMode crcMode) { this->crcMode = crcMode; }
    void setIfIndex(IInterfaceTable *ift, int index);
    int getIfIndex() const { return ifIndex; }
    void setInterfaceName(std::string ifName) { this->interfaceName = ifName; }
    std::string getInterfaceName() const { return interfaceName; }
    void setMtu(unsigned short ifMTU) { mtu = ifMTU; }
    unsigned short getMtu() const { return mtu; }
    void setAreaId(AreaId areaId) { areaID = areaId; }
    AreaId getAreaId() const { return areaID; }
    void setTransitAreaId(AreaId areaId) { transitAreaID = areaId; }
    AreaId getTransitAreaId() const { return transitAreaID; }
    void setOutputCost(Metric cost) { interfaceOutputCost = cost; }
    Metric getOutputCost() const { return interfaceOutputCost; }
    void setRetransmissionInterval(short interval) { retransmissionInterval = interval; }
    short getRetransmissionInterval() const { return retransmissionInterval; }
    void setTransmissionDelay(short delay) { interfaceTransmissionDelay = delay; }
    short getTransmissionDelay() const { return interfaceTransmissionDelay; }
    void setAcknowledgementDelay(short delay) { acknowledgementDelay = delay; }
    short getAcknowledgementDelay() const { return acknowledgementDelay; }
    void setRouterPriority(unsigned char priority) { routerPriority = priority; }
    unsigned char getRouterPriority() const { return routerPriority; }
    void setHelloInterval(short interval) { helloInterval = interval; }
    short getHelloInterval() const { return helloInterval; }
    void setPollInterval(short interval) { pollInterval = interval; }
    short getPollInterval() const { return pollInterval; }
    void setRouterDeadInterval(short interval) { routerDeadInterval = interval; }
    short getRouterDeadInterval() const { return routerDeadInterval; }
    void setAuthenticationType(AuthenticationType type) { authenticationType = type; }
    AuthenticationType getAuthenticationType() const { return authenticationType; }
    void setAuthenticationKey(AuthenticationKeyType key) { authenticationKey = key; }
    AuthenticationKeyType getAuthenticationKey() const { return authenticationKey; }
    void setAddressRange(Ipv4AddressRange range) { interfaceAddressRange = range; }
    Ipv4AddressRange getAddressRange() const { return interfaceAddressRange; }

    cMessage *getHelloTimer() { return helloTimer; }
    cMessage *getWaitTimer() { return waitTimer; }
    cMessage *getAcknowledgementTimer() { return acknowledgementTimer; }
    DesignatedRouterId getDesignatedRouter() const { return designatedRouter; }
    DesignatedRouterId getBackupDesignatedRouter() const { return backupDesignatedRouter; }
    unsigned long getNeighborCount() const { return neighboringRouters.size(); }
    Neighbor *getNeighbor(unsigned long i) { return neighboringRouters[i]; }
    const Neighbor *getNeighbor(unsigned long i) const { return neighboringRouters[i]; }

    void setArea(Ospfv2Area *area) { parentArea = area; }
    Ospfv2Area *getArea() { return parentArea; }
    const Ospfv2Area *getArea() const { return parentArea; }

    friend std::ostream& operator<<(std::ostream& stream, const Ospfv2Interface& intf);
};

} // namespace ospfv2

} // namespace inet

#endif

