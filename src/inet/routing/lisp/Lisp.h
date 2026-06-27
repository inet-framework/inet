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
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/lisp/LispMapDatabase.h"
#include "inet/routing/lisp/LispMapStorageBase.h"
#include "inet/routing/lisp/LispServerEntry.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {
namespace lisp {

/**
 * Locator/ID Separation Protocol (LISP, RFC 6830). See the NED file. This is a
 * scaffold: it binds the LISP control (UDP 4342) and data (UDP 4341) sockets;
 * the message handling, mapping system (map-cache / map-database / site-database)
 * and the ITR/ETR data-plane (via network-layer hooks) are added in later steps.
 */
class INET_API Lisp : public ApplicationBase, public UdpSocket::ICallback
{
  protected:
    int controlPort = 4342;
    int dataPort = 4341;

    UdpSocket controlSocket; // LISP control messages (Map-Request/Reply/Register/Notify)
    UdpSocket dataSocket;    // LISP-encapsulated data packets
    SocketMap socketMap;     // demultiplexes inbound UDP packets to the right socket

    ModuleRefByPar<IInterfaceTable> ift;

    // mapping system (owned as members; no separate submodules)
    LispMapDatabase mapDatabase; // this ETR's own static EID-to-RLOC mappings
    LispMapStorageBase mapCache; // dynamic EID-to-RLOC cache learned from Map-Replies

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

    // UdpSocket::ICallback
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

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
