//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISP_H
#define __INET_LISP_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/linklayer/tun/TunSocket.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/routing/lisp/LispMapDatabase.h"
#include "inet/routing/lisp/LispMapStorageBase.h"
#include "inet/routing/lisp/LispMessages_m.h"
#include "inet/routing/lisp/LispServerEntry.h"
#include "inet/routing/lisp/LispSiteDatabase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {
namespace lisp {

/**
 * Locator/ID Separation Protocol (LISP, RFC 6830). See the NED file. Binds the
 * LISP control (UDP 4342) and data (UDP 4341) sockets and runs the ITR/ETR data
 * plane over a TUN interface: EID-destined traffic is routed to the TUN
 * interface, captured, LISP-encapsulated and sent to the remote RLOC; received
 * encapsulated traffic is decapsulated and re-injected through the TUN interface.
 * The control plane (Map-Request/Reply/Register/Notify) is added in later steps.
 */
class INET_API Lisp : public ApplicationBase, public UdpSocket::ICallback, public TunSocket::ICallback
{
  protected:
    int controlPort = 4342;
    int dataPort = 4341;

    UdpSocket controlSocket; // LISP control messages (Map-Request/Reply/Register/Notify)
    UdpSocket dataSocket;    // LISP-encapsulated data packets
    TunSocket tunSocket;     // capture of EID traffic (ITR) and re-injection (ETR)
    SocketMap socketMap;     // demultiplexes inbound packets to the right socket

    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    int tunInterfaceId = -1;

    // mapping system (owned as members; no separate submodules)
    LispMapDatabase mapDatabase;     // this ETR's own static EID-to-RLOC mappings
    LispMapStorageBase mapCache;     // dynamic EID-to-RLOC cache learned from Map-Replies
    LispSiteDatabase siteDatabase;   // Map-Server: registered sites/ETRs

    // Map-Server / Map-Resolver lists
    ServerAddresses mapServers;
    ServerAddresses mapResolvers;
    ServerCItem mapResolverQueue;

    // roles
    bool mapServerV4 = false, mapServerV6 = false;
    bool mapResolverV4 = false, mapResolverV6 = false;

    // behaviour
    bool acceptMapRequestMapping = false;
    bool advertOnlyOwnEids = false;
    bool echoNonceAlgo = false;
    bool ciscoStartupDelays = false;
    unsigned short mapCacheTtl = 1440;
    enum ProbeAlgo { PROBE_CISCO, PROBE_SIMPLE, PROBE_SOPHISTICATED };
    ProbeAlgo rlocProbingAlgo = PROBE_CISCO;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    void parseConfig(cXMLElement *config);
    void parseMapServerConfig(cXMLElement *config);
    void parseMapResolverConfig(cXMLElement *config);
    LispServerEntry *findServerEntryByAddress(ServerAddresses& list, const L3Address& addr);
    bool isMapServer() const { return mapServerV4 || mapServerV6; }
    bool isMapResolver() const { return mapResolverV4 || mapResolverV6; }

    // data plane
    void installEidRoutes();        ///< route the cached EID prefixes to the TUN interface (ITR capture)
    void encapsulate(Packet *packet); ///< ITR: LISP-encapsulate an EID-destined packet and send to the RLOC
    void decapsulate(Packet *packet); ///< ETR: strip the LISP header and re-inject the inner datagram

    // control plane
    void handleSelfMessage(cMessage *msg);
    void handleControlMessage(Packet *packet);
    LispMapRecord makeMapRecord(const LispMapEntry& entry, bool aBit, int action);
    void scheduleRegistration();                  ///< ETR: schedule Map-Register for each Map-Server
    void expireRegisterTimer(cMessage *timer);    ///< ETR: (re)send a Map-Register
    void sendMapRegister(LispServerEntry& se);    ///< ETR: build and send a Map-Register
    void receiveMapRegister(Packet *packet);      ///< MS: store a received registration

    // UdpSocket::ICallback
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    // TunSocket::ICallback
    virtual void socketDataArrived(TunSocket *socket, Packet *packet) override;
    virtual void socketClosed(TunSocket *socket) override;

    // ApplicationBase lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    Lisp() {}
    virtual ~Lisp() {}
};

} // namespace lisp
} // namespace inet

#endif
