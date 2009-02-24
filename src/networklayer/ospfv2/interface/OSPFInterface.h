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
    Neighbor*           GetNeighborByID                     (RouterID neighborID);
    Neighbor*           GetNeighborByAddress                (IPv4Address address);
    void                AddNeighbor                         (Neighbor* neighbor);
    InterfaceStateType  GetState                            (void) const;
    static const char*  GetStateString                      (InterfaceStateType stateType);
    bool                HasAnyNeighborInStates              (int states) const;
    void                RemoveFromAllRetransmissionLists    (LSAKeyType lsaKey);
    bool                IsOnAnyRetransmissionList           (LSAKeyType lsaKey) const;
    bool                FloodLSA                            (OSPFLSA* lsa, Interface* intf = NULL, Neighbor* neighbor = NULL);
    void                AddDelayedAcknowledgement           (OSPFLSAHeader& lsaHeader);
    void                SendDelayedAcknowledgements         (void);
    void                AgeTransmittedLSALists              (void);

    OSPFLinkStateUpdatePacket*  CreateUpdatePacket          (OSPFLSA* lsa);

    void                    SetType                         (OSPFInterfaceType ifType)  { interfaceType = ifType; }
    OSPFInterfaceType       GetType                         (void) const                { return interfaceType; }
    void                    SetIfIndex                      (unsigned char index);
    unsigned char           GetIfIndex                      (void) const                { return ifIndex; }
    void                    SetMTU                          (unsigned short ifMTU)      { mtu = ifMTU; }
    unsigned short          GetMTU                          (void) const                { return mtu; }
    void                    SetAreaID                       (AreaID areaId)             { areaID = areaId; }
    AreaID                  GetAreaID                       (void) const                { return areaID; }
    void                    SetTransitAreaID                (AreaID areaId)             { transitAreaID = areaId; }
    AreaID                  GetTransitAreaID                (void) const                { return transitAreaID; }
    void                    SetOutputCost                   (Metric cost)               { interfaceOutputCost = cost; }
    Metric                  GetOutputCost                   (void) const                { return interfaceOutputCost; }
    void                    SetRetransmissionInterval       (short interval)            { retransmissionInterval = interval; }
    short                   GetRetransmissionInterval       (void) const                { return retransmissionInterval; }
    void                    SetTransmissionDelay            (short delay)               { interfaceTransmissionDelay = delay; }
    short                   GetTransmissionDelay            (void) const                { return interfaceTransmissionDelay; }
    void                    SetAcknowledgementDelay         (short delay)               { acknowledgementDelay = delay; }
    short                   GetAcknowledgementDelay         (void) const                { return acknowledgementDelay; }
    void                    SetRouterPriority               (unsigned char priority)    { routerPriority = priority; }
    unsigned char           GetRouterPriority               (void) const                { return routerPriority; }
    void                    SetHelloInterval                (short interval)            { helloInterval = interval; }
    short                   GetHelloInterval                (void) const                { return helloInterval; }
    void                    SetPollInterval                 (short interval)            { pollInterval = interval; }
    short                   GetPollInterval                 (void) const                { return pollInterval; }
    void                    SetRouterDeadInterval           (short interval)            { routerDeadInterval = interval; }
    short                   GetRouterDeadInterval           (void) const                { return routerDeadInterval; }
    void                    SetAuthenticationType           (AuthenticationType type)   { authenticationType = type; }
    AuthenticationType      GetAuthenticationType           (void) const                { return authenticationType; }
    void                    SetAuthenticationKey            (AuthenticationKeyType key) { authenticationKey = key; }
    AuthenticationKeyType   GetAuthenticationKey            (void) const                { return authenticationKey; }
    void                    SetAddressRange                 (IPv4AddressRange range)    { interfaceAddressRange = range; }
    IPv4AddressRange        GetAddressRange                 (void) const                { return interfaceAddressRange; }

    OSPFTimer*              GetHelloTimer                   (void)                      { return helloTimer; }
    OSPFTimer*              GetWaitTimer                    (void)                      { return waitTimer; }
    OSPFTimer*              GetAcknowledgementTimer         (void)                      { return acknowledgementTimer; }
    DesignatedRouterID      GetDesignatedRouter             (void) const                { return designatedRouter; }
    DesignatedRouterID      GetBackupDesignatedRouter       (void) const                { return backupDesignatedRouter; }
    unsigned long           GetNeighborCount                (void) const                { return neighboringRouters.size(); }
    Neighbor*               GetNeighbor                     (unsigned long i)           { return neighboringRouters[i]; }
    const Neighbor*         GetNeighbor                     (unsigned long i) const     { return neighboringRouters[i]; }

    void                    SetArea                         (Area* area)                { parentArea = area; }
    Area*                   GetArea                         (void)                      { return parentArea; }
    const Area*             GetArea                         (void) const                { return parentArea; }
};

} // namespace OSPF

#endif // __INET_OSPFINTERFACE_H

