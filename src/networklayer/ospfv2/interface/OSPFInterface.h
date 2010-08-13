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

#include "INETDefs.h"
#include "OSPFcommon.h"
#include "OSPFTimer_m.h"
#include "OSPFNeighbor.h"
#include <map>
#include <vector>
#include <list>

namespace OSPF {

class InterfaceState;
class Area;

class Interface {
public:
    enum InterfaceEventType {
        InterfaceUp          = 0,
        HelloTimer           = 1,
        WaitTimer            = 2,
        AcknowledgementTimer = 3,
        BackupSeen           = 4,
        NeighborChange       = 5,
        LoopIndication       = 6,
        UnloopIndication     = 7,
        InterfaceDown        = 8
    };

    enum OSPFInterfaceType {
        UnknownType       = 0,
        PointToPoint      = 1,
        Broadcast         = 2,
        NBMA              = 3,
        PointToMultiPoint = 4,
        Virtual           = 5
    };

    enum InterfaceStateType {
        DownState                = 0,
        LoopbackState            = 1,
        WaitingState             = 2,
        PointToPointState        = 3,
        NotDesignatedRouterState = 4,
        BackupState              = 5,
        DesignatedRouterState    = 6
    };

private:
    OSPFInterfaceType                                                   interfaceType;
    InterfaceState*                                                     state;
    InterfaceState*                                                     previousState;
    unsigned char                                                       ifIndex;
    unsigned short                                                      mtu;
    IPv4AddressRange                                                    interfaceAddressRange;
    AreaID                                                              areaID;
    AreaID                                                              transitAreaID;
    short                                                               helloInterval;
    short                                                               pollInterval;
    short                                                               routerDeadInterval;
    short                                                               interfaceTransmissionDelay;
    unsigned char                                                       routerPriority;
    OSPFTimer*                                                          helloTimer;
    OSPFTimer*                                                          waitTimer;
    OSPFTimer*                                                          acknowledgementTimer;
    std::map<RouterID, Neighbor*>                                       neighboringRoutersByID;
    std::map<IPv4Address, Neighbor*, IPv4Address_Less>                  neighboringRoutersByAddress;
    std::vector<Neighbor*>                                              neighboringRouters;
    std::map<IPv4Address, std::list<OSPFLSAHeader>, IPv4Address_Less>   delayedAcknowledgements;
    DesignatedRouterID                                                  designatedRouter;
    DesignatedRouterID                                                  backupDesignatedRouter;
    Metric                                                              interfaceOutputCost;
    short                                                               retransmissionInterval;
    short                                                               acknowledgementDelay;
    AuthenticationType                                                  authenticationType;
    AuthenticationKeyType                                               authenticationKey;

    Area*                                                               parentArea;
private:
    friend class InterfaceState;
    void ChangeState(InterfaceState* newState, InterfaceState* currentState);

public:
            Interface(OSPFInterfaceType ifType = UnknownType);
    virtual ~Interface(void);

    void                ProcessEvent                        (InterfaceEventType event);
    void                Reset                               (void);
    void                SendHelloPacket                     (IPv4Address destination, short ttl = 1);
    void                SendLSAcknowledgement               (OSPFLSAHeader* lsaHeader, IPv4Address destination);
    Neighbor*           getNeighborByID                     (RouterID neighborID);
    Neighbor*           getNeighborByAddress                (IPv4Address address);
    void                AddNeighbor                         (Neighbor* neighbor);
    InterfaceStateType  getState                            (void) const;
    static const char*  getStateString                      (InterfaceStateType stateType);
    bool                HasAnyNeighborInStates              (int states) const;
    void                RemoveFromAllRetransmissionLists    (LSAKeyType lsaKey);
    bool                IsOnAnyRetransmissionList           (LSAKeyType lsaKey) const;
    bool                FloodLSA                            (OSPFLSA* lsa, Interface* intf = NULL, Neighbor* neighbor = NULL);
    void                AddDelayedAcknowledgement           (OSPFLSAHeader& lsaHeader);
    void                SendDelayedAcknowledgements         (void);
    void                AgeTransmittedLSALists              (void);

    OSPFLinkStateUpdatePacket*  CreateUpdatePacket          (OSPFLSA* lsa);

    void                    setType                         (OSPFInterfaceType ifType)  { interfaceType = ifType; }
    OSPFInterfaceType       getType                         (void) const                { return interfaceType; }
    void                    setIfIndex                      (unsigned char index);
    unsigned char           getIfIndex                      (void) const                { return ifIndex; }
    void                    setMTU                          (unsigned short ifMTU)      { mtu = ifMTU; }
    unsigned short          getMTU                          (void) const                { return mtu; }
    void                    setAreaID                       (AreaID areaId)             { areaID = areaId; }
    AreaID                  getAreaID                       (void) const                { return areaID; }
    void                    setTransitAreaID                (AreaID areaId)             { transitAreaID = areaId; }
    AreaID                  getTransitAreaID                (void) const                { return transitAreaID; }
    void                    setOutputCost                   (Metric cost)               { interfaceOutputCost = cost; }
    Metric                  getOutputCost                   (void) const                { return interfaceOutputCost; }
    void                    setRetransmissionInterval       (short interval)            { retransmissionInterval = interval; }
    short                   getRetransmissionInterval       (void) const                { return retransmissionInterval; }
    void                    setTransmissionDelay            (short delay)               { interfaceTransmissionDelay = delay; }
    short                   getTransmissionDelay            (void) const                { return interfaceTransmissionDelay; }
    void                    setAcknowledgementDelay         (short delay)               { acknowledgementDelay = delay; }
    short                   getAcknowledgementDelay         (void) const                { return acknowledgementDelay; }
    void                    setRouterPriority               (unsigned char priority)    { routerPriority = priority; }
    unsigned char           getRouterPriority               (void) const                { return routerPriority; }
    void                    setHelloInterval                (short interval)            { helloInterval = interval; }
    short                   getHelloInterval                (void) const                { return helloInterval; }
    void                    setPollInterval                 (short interval)            { pollInterval = interval; }
    short                   getPollInterval                 (void) const                { return pollInterval; }
    void                    setRouterDeadInterval           (short interval)            { routerDeadInterval = interval; }
    short                   getRouterDeadInterval           (void) const                { return routerDeadInterval; }
    void                    setAuthenticationType           (AuthenticationType type)   { authenticationType = type; }
    AuthenticationType      getAuthenticationType           (void) const                { return authenticationType; }
    void                    setAuthenticationKey            (AuthenticationKeyType key) { authenticationKey = key; }
    AuthenticationKeyType   getAuthenticationKey            (void) const                { return authenticationKey; }
    void                    setAddressRange                 (IPv4AddressRange range)    { interfaceAddressRange = range; }
    IPv4AddressRange        getAddressRange                 (void) const                { return interfaceAddressRange; }

    OSPFTimer*              getHelloTimer                   (void)                      { return helloTimer; }
    OSPFTimer*              getWaitTimer                    (void)                      { return waitTimer; }
    OSPFTimer*              getAcknowledgementTimer         (void)                      { return acknowledgementTimer; }
    DesignatedRouterID      getDesignatedRouter             (void) const                { return designatedRouter; }
    DesignatedRouterID      getBackupDesignatedRouter       (void) const                { return backupDesignatedRouter; }
    unsigned long           getNeighborCount                (void) const                { return neighboringRouters.size(); }
    Neighbor*               getNeighbor                     (unsigned long i)           { return neighboringRouters[i]; }
    const Neighbor*         getNeighbor                     (unsigned long i) const     { return neighboringRouters[i]; }

    void                    setArea                         (Area* area)                { parentArea = area; }
    Area*                   getArea                         (void)                      { return parentArea; }
    const Area*             getArea                         (void) const                { return parentArea; }
};

} // namespace OSPF

#endif // __INET_OSPFINTERFACE_H

