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

#ifndef __INET_OSPFNEIGHBOR_H
#define __INET_OSPFNEIGHBOR_H

#include "OSPFPacket_m.h"
#include "OSPFTimer_m.h"
#include "OSPFcommon.h"
#include "LSA.h"
#include <list>

namespace OSPF {

class NeighborState;
class Interface;

class Neighbor {

friend class NeighborState;

public:
    enum NeighborEventType {
        HelloReceived               = 0,
        Start                       = 1,
        TwoWayReceived              = 2,
        NegotiationDone             = 3,
        ExchangeDone                = 4,
        BadLinkStateRequest         = 5,
        LoadingDone                 = 6,
        IsAdjacencyOK               = 7,
        SequenceNumberMismatch      = 8,
        OneWayReceived              = 9,
        KillNeighbor                = 10,
        InactivityTimer             = 11,
        PollTimer                   = 12,
        LinkDown                    = 13,
        DDRetransmissionTimer       = 14,
        UpdateRetransmissionTimer   = 15,
        RequestRetransmissionTimer  = 16
    };

    enum NeighborStateType {
        DownState          = 0,
        AttemptState       = 1,
        InitState          = 2,
        TwoWayState        = 4,
        ExchangeStartState = 8,
        ExchangeState      = 16,
        LoadingState       = 32,
        FullState          = 64
    };

    enum DatabaseExchangeRelationshipType {
        Master = 0,
        Slave = 1
    };

    struct DDPacketID {
        OSPFDDOptions   ddOptions;
        OSPFOptions     options;
        unsigned long   sequenceNumber;
    };

private:
    struct TransmittedLSA {
        LSAKeyType      lsaKey;
        unsigned short  age;
    };

private:
    NeighborState*                      state;
    NeighborState*                      previousState;
    OSPFTimer*                          inactivityTimer;
    OSPFTimer*                          pollTimer;
    OSPFTimer*                          ddRetransmissionTimer;
    OSPFTimer*                          updateRetransmissionTimer;
    bool                                updateRetransmissionTimerActive;
    OSPFTimer*                          requestRetransmissionTimer;
    bool                                requestRetransmissionTimerActive;
    DatabaseExchangeRelationshipType    databaseExchangeRelationship;
    bool                                firstAdjacencyInited;
    unsigned long                       ddSequenceNumber;
    DDPacketID                          lastReceivedDDPacket;
    RouterID                            neighborID;
    unsigned char                       neighborPriority;
    IPv4Address                         neighborIPAddress;
    OSPFOptions                         neighborOptions;
    DesignatedRouterID                  neighborsDesignatedRouter;
    DesignatedRouterID                  neighborsBackupDesignatedRouter;
    bool                                designatedRoutersSetUp;
    short                               neighborsRouterDeadInterval;
    std::list<OSPFLSA*>                 linkStateRetransmissionList;
    std::list<OSPFLSAHeader*>           databaseSummaryList;
    std::list<OSPFLSAHeader*>           linkStateRequestList;
    std::list<TransmittedLSA>           transmittedLSAs;
    OSPFDatabaseDescriptionPacket*      lastTransmittedDDPacket;

    Interface*                          parentInterface;

    // FIXME!!! Should come from a global unique number generator module.
    static unsigned long                ddSequenceNumberInitSeed;

private:
    void ChangeState(NeighborState* newState, NeighborState* currentState);

public:
            Neighbor(RouterID neighbor = NullRouterID);
    virtual ~Neighbor(void);

    void                ProcessEvent                        (NeighborEventType event);
    void                Reset                               (void);
    void                InitFirstAdjacency                  (void);
    NeighborStateType   GetState                            (void) const;
    static const char*  GetStateString                      (NeighborStateType stateType);
    void                SendDatabaseDescriptionPacket       (bool init = false);
    bool                RetransmitDatabaseDescriptionPacket(void);
    void                CreateDatabaseSummary               (void);
    void                SendLinkStateRequestPacket          (void);
    void                RetransmitUpdatePacket              (void);
    bool                NeedAdjacency                       (void);
    void                AddToRetransmissionList             (OSPFLSA* lsa);
    void                RemoveFromRetransmissionList        (LSAKeyType lsaKey);
    bool                IsLSAOnRetransmissionList           (LSAKeyType lsaKey) const;
    OSPFLSA*            FindOnRetransmissionList            (LSAKeyType lsaKey);
    void                StartUpdateRetransmissionTimer      (void);
    void                ClearUpdateRetransmissionTimer      (void);
    void                AddToRequestList                    (OSPFLSAHeader* lsaHeader);
    void                RemoveFromRequestList               (LSAKeyType lsaKey);
    bool                IsLSAOnRequestList                  (LSAKeyType lsaKey) const;
    OSPFLSAHeader*      FindOnRequestList                   (LSAKeyType lsaKey);
    void                StartRequestRetransmissionTimer     (void);
    void                ClearRequestRetransmissionTimer     (void);
    void                AddToTransmittedLSAList             (LSAKeyType lsaKey);
    bool                IsOnTransmittedLSAList              (LSAKeyType lsaKey) const;
    void                AgeTransmittedLSAList               (void);
    unsigned long       GetUniqueULong                      (void);
    void                DeleteLastSentDDPacket              (void);

    void                SetNeighborID               (RouterID id)                   { neighborID = id; }
    RouterID            GetNeighborID               (void) const                    { return neighborID; }
    void                SetPriority                 (unsigned char priority)        { neighborPriority = priority; }
    unsigned char       GetPriority                 (void) const                    { return neighborPriority; }
    void                SetAddress                  (IPv4Address address)           { neighborIPAddress = address; }
    IPv4Address         GetAddress                  (void) const                    { return neighborIPAddress; }
    void                SetDesignatedRouter         (DesignatedRouterID routerID)   { neighborsDesignatedRouter = routerID; }
    DesignatedRouterID  GetDesignatedRouter         (void) const                    { return neighborsDesignatedRouter; }
    void                SetBackupDesignatedRouter   (DesignatedRouterID routerID)   { neighborsBackupDesignatedRouter = routerID; }
    DesignatedRouterID  GetBackupDesignatedRouter   (void) const                    { return neighborsBackupDesignatedRouter; }
    void                SetRouterDeadInterval       (short interval)                { neighborsRouterDeadInterval = interval; }
    short               GetRouterDeadInterval       (void) const                    { return neighborsRouterDeadInterval; }
    void                SetDDSequenceNumber         (unsigned long sequenceNumber)  { ddSequenceNumber = sequenceNumber; }
    unsigned long       GetDDSequenceNumber         (void) const                    { return ddSequenceNumber; }
    void                SetOptions                  (OSPFOptions options)           { neighborOptions = options; }
    OSPFOptions         GetOptions                  (void) const                    { return neighborOptions; }
    void                SetLastReceivedDDPacket     (DDPacketID packetID)           { lastReceivedDDPacket = packetID; }
    DDPacketID          GetLastReceivedDDPacket     (void) const                    { return lastReceivedDDPacket; }

    void                                SetDatabaseExchangeRelationship(DatabaseExchangeRelationshipType relation) { databaseExchangeRelationship = relation; }
    DatabaseExchangeRelationshipType    GetDatabaseExchangeRelationship(void) const                                { return databaseExchangeRelationship; }

    void                SetInterface                (Interface* intf)               { parentInterface = intf; }
    Interface*          GetInterface                (void)                          { return parentInterface; }
    const Interface*    GetInterface                (void) const                    { return parentInterface; }

    OSPFTimer*          GetInactivityTimer                  (void)                  { return inactivityTimer; }
    OSPFTimer*          GetPollTimer                        (void)                  { return pollTimer; }
    OSPFTimer*          GetDDRetransmissionTimer            (void)                  { return ddRetransmissionTimer; }
    OSPFTimer*          GetUpdateRetransmissionTimer        (void)                  { return updateRetransmissionTimer; }
    bool                IsUpdateRetransmissionTimerActive   (void) const            { return updateRetransmissionTimerActive; }
    bool                IsRequestRetransmissionTimerActive  (void) const            { return requestRetransmissionTimerActive; }
    bool                IsFirstAdjacencyInited              (void) const            { return firstAdjacencyInited; }
    bool                DesignatedRoutersAreSetUp           (void) const            { return designatedRoutersSetUp; }
    void                SetUpDesignatedRouters              (bool setUp)            { designatedRoutersSetUp = setUp; }
    unsigned long       GetDatabaseSummaryListCount         (void) const            { return databaseSummaryList.size(); }

    void IncrementDDSequenceNumber          (void)       { ddSequenceNumber++; }
    bool IsLinkStateRequestListEmpty        (void) const { return linkStateRequestList.empty(); }
    bool IsLinkStateRetransmissionListEmpty(void) const { return linkStateRetransmissionList.empty(); }
    void PopFirstLinkStateRequest           (void)       { linkStateRequestList.pop_front(); }
};

} // namespace OSPF

inline bool operator== (OSPF::Neighbor::DDPacketID leftID, OSPF::Neighbor::DDPacketID rightID)
{
    return ((leftID.ddOptions.I_Init         == rightID.ddOptions.I_Init) &&
            (leftID.ddOptions.M_More         == rightID.ddOptions.M_More) &&
            (leftID.ddOptions.MS_MasterSlave == rightID.ddOptions.MS_MasterSlave) &&
            (leftID.options                  == rightID.options) &&
            (leftID.sequenceNumber           == rightID.sequenceNumber));
}

inline bool operator!= (OSPF::Neighbor::DDPacketID leftID, OSPF::Neighbor::DDPacketID rightID)
{
    return (!(leftID == rightID));
}

#endif // __INET_OSPFNEIGHBOR_H

