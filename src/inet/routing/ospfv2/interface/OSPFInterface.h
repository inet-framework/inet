//
// Copyright (C) 2006 Andras Babos and Andras Varga
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

#ifndef __INET_OSPFINTERFACE_H
#define __INET_OSPFINTERFACE_H

#include <map>
#include <vector>
#include <list>

#include "inet/common/INETDefs.h"

#include "inet/routing/ospfv2/router/OSPFcommon.h"
#include "inet/routing/ospfv2/neighbor/OSPFNeighbor.h"
#include "inet/routing/ospfv2/OSPFTimer.h"
#include "inet/networklayer/common/IInterfaceTable.h"

namespace inet {

namespace ospf {

class InterfaceState;
class Area;

class Interface
{
  public:
    enum InterfaceEventType {
        INTERFACE_UP = 0,
        HELLO_TIMER = 1,
        WAIT_TIMER = 2,
        ACKNOWLEDGEMENT_TIMER = 3,
        BACKUP_SEEN = 4,
        NEIGHBOR_CHANGE = 5,
        LOOP_INDICATION = 6,
        UNLOOP_INDICATION = 7,
        INTERFACE_DOWN = 8
    };

    enum OSPFInterfaceType {
        UNKNOWN_TYPE = 0,
        POINTTOPOINT = 1,
        BROADCAST = 2,
        NBMA = 3,
        POINTTOMULTIPOINT = 4,
        VIRTUAL = 5
    };

    enum InterfaceStateType {
        DOWN_STATE = 0,
        LOOPBACK_STATE = 1,
        WAITING_STATE = 2,
        POINTTOPOINT_STATE = 3,
        NOT_DESIGNATED_ROUTER_STATE = 4,
        BACKUP_STATE = 5,
        DESIGNATED_ROUTER_STATE = 6
    };

  private:
    OSPFInterfaceType interfaceType;
    InterfaceState *state;
    InterfaceState *previousState;
    int ifIndex;
    unsigned short mtu;
    IPv4AddressRange interfaceAddressRange;
    AreaID areaID;
    AreaID transitAreaID;
    short helloInterval;
    short pollInterval;
    short routerDeadInterval;
    short interfaceTransmissionDelay;
    unsigned char routerPriority;
    cMessage *helloTimer;
    cMessage *waitTimer;
    cMessage *acknowledgementTimer;
    std::map<RouterID, Neighbor *> neighboringRoutersByID;
    std::map<IPv4Address, Neighbor *> neighboringRoutersByAddress;
    std::vector<Neighbor *> neighboringRouters;
    std::map<IPv4Address, std::list<OSPFLSAHeader> > delayedAcknowledgements;
    DesignatedRouterID designatedRouter;
    DesignatedRouterID backupDesignatedRouter;
    Metric interfaceOutputCost;
    short retransmissionInterval;
    short acknowledgementDelay;
    AuthenticationType authenticationType;
    AuthenticationKeyType authenticationKey;

    Area *parentArea;

  private:
    friend class InterfaceState;
    void changeState(InterfaceState *newState, InterfaceState *currentState);

  public:
    Interface(OSPFInterfaceType ifType = UNKNOWN_TYPE);
    virtual ~Interface();

    void processEvent(InterfaceEventType event);
    void reset();
    void sendHelloPacket(IPv4Address destination, short ttl = 1);
    void sendLSAcknowledgement(OSPFLSAHeader *lsaHeader, IPv4Address destination);
    Neighbor *getNeighborByID(RouterID neighborID);
    Neighbor *getNeighborByAddress(IPv4Address address);
    void addNeighbor(Neighbor *neighbor);
    InterfaceStateType getState() const;
    static const char *getStateString(InterfaceStateType stateType);
    bool hasAnyNeighborInStates(int states) const;
    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);
    bool isOnAnyRetransmissionList(LSAKeyType lsaKey) const;
    bool floodLSA(OSPFLSA *lsa, Interface *intf = NULL, Neighbor *neighbor = NULL);
    void addDelayedAcknowledgement(OSPFLSAHeader& lsaHeader);
    void sendDelayedAcknowledgements();
    void ageTransmittedLSALists();

    OSPFLinkStateUpdatePacket *createUpdatePacket(OSPFLSA *lsa);

    void setType(OSPFInterfaceType ifType) { interfaceType = ifType; }
    OSPFInterfaceType getType() const { return interfaceType; }
    void setIfIndex(IInterfaceTable *ift, int index);
    int getIfIndex() const { return ifIndex; }
    void setMTU(unsigned short ifMTU) { mtu = ifMTU; }
    unsigned short getMTU() const { return mtu; }
    void setAreaID(AreaID areaId) { areaID = areaId; }
    AreaID getAreaID() const { return areaID; }
    void setTransitAreaID(AreaID areaId) { transitAreaID = areaId; }
    AreaID getTransitAreaID() const { return transitAreaID; }
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
    void setAddressRange(IPv4AddressRange range) { interfaceAddressRange = range; }
    IPv4AddressRange getAddressRange() const { return interfaceAddressRange; }

    cMessage *getHelloTimer() { return helloTimer; }
    cMessage *getWaitTimer() { return waitTimer; }
    cMessage *getAcknowledgementTimer() { return acknowledgementTimer; }
    DesignatedRouterID getDesignatedRouter() const { return designatedRouter; }
    DesignatedRouterID getBackupDesignatedRouter() const { return backupDesignatedRouter; }
    unsigned long getNeighborCount() const { return neighboringRouters.size(); }
    Neighbor *getNeighbor(unsigned long i) { return neighboringRouters[i]; }
    const Neighbor *getNeighbor(unsigned long i) const { return neighboringRouters[i]; }

    void setArea(Area *area) { parentArea = area; }
    Area *getArea() { return parentArea; }
    const Area *getArea() const { return parentArea; }
};

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_OSPFINTERFACE_H

