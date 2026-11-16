//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "CoapDtlsTestPeer.h"

#include <algorithm>

#include "inet/common/MemoryOutputStream.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {
namespace protocoltest {

Define_Module(CoapDtlsTestPeer);

simsignal_t CoapDtlsTestPeer::controlStateChangedSignal = cComponent::registerSignal("controlStateChanged");

void CoapDtlsTestPeer::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        serverMode = par("server").boolValue();
}

void CoapDtlsTestPeer::handleMessageWhenUp(cMessage *msg)
{
    throw cRuntimeError("Unexpected message in CoapDtlsTestPeer: %s", msg->getName());
}

void CoapDtlsTestPeer::handleStartOperation(LifecycleOperation *operation)
{
    auto *engine = getModuleFromPar<dtls::IDtls>(par("dtlsModule"), this);
    dtlsSocket.setEngine(engine);
    dtlsSocket.setCallback(this);
    dtlsBound = true;
    if (serverMode)
        dtlsSocket.listen();
}

void CoapDtlsTestPeer::handleStopOperation(LifecycleOperation *operation)
{
    auto ids = activeSessions;
    activeSessions.clear();
    if (dtlsBound) {
        for (uint64_t id : ids)
            dtlsSocket.close(id);
        dtlsBound = false;
    }
}

void CoapDtlsTestPeer::handleCrashOperation(LifecycleOperation *operation)
{
    activeSessions.clear();
    dtlsBound = false;
}

uint64_t CoapDtlsTestPeer::openSession(const L3Address& destAddr, int destPort)
{
    Enter_Method("openSession");
    if (!dtlsBound)
        throw cRuntimeError("CoapDtlsTestPeer is not bound to a DTLS engine");
    uint64_t id = dtlsSocket.open(destAddr, destPort);
    if (id != 0)
        activeSessions.insert(id);
    return id;
}

uint64_t CoapDtlsTestPeer::openSession(const std::string& destAddrStr, int destPort)
{
    return openSession(L3AddressResolver().resolve(destAddrStr.c_str()), destPort);
}

void CoapDtlsTestPeer::sendBytes(uint64_t sessionId, const std::vector<uint8_t>& bytes, const char *packetName)
{
    Enter_Method("sendBytes");
    if (!dtlsBound)
        throw cRuntimeError("CoapDtlsTestPeer is not bound to a DTLS engine");
    Packet *packet = new Packet(packetName);
    if (!bytes.empty())
        packet->insertAtBack(makeShared<BytesChunk>(bytes));
    emit(packetSentSignal, packet);
    emit(packetSentToLowerSignal, packet);
    drop(packet);
    dtlsSocket.send(sessionId, packet);
}

void CoapDtlsTestPeer::closeSession(uint64_t sessionId)
{
    Enter_Method("closeSession");
    if (dtlsBound)
        dtlsSocket.close(sessionId);
}

bool CoapDtlsTestPeer::requestKeyUpdate(uint64_t sessionId, bool requestPeerUpdate)
{
    Enter_Method("requestKeyUpdate");
    if (!dtlsBound)
        return false;
    return dtlsSocket.requestKeyUpdate(sessionId, requestPeerUpdate);
}

bool CoapDtlsTestPeer::isSessionEstablished(uint64_t sessionId) const
{
    return std::find(establishedIds.begin(), establishedIds.end(), sessionId) != establishedIds.end();
}

bool CoapDtlsTestPeer::isSessionClosed(uint64_t sessionId) const
{
    return std::find(closedIds.begin(), closedIds.end(), sessionId) != closedIds.end();
}

void CoapDtlsTestPeer::accepted(dtls::IDtls::AssociationId id, const L3Address& localAddress, int localPort,
        const L3Address& peerAddress, int peerPort)
{
    Enter_Method("accepted");
    activeSessions.insert(id);
    acceptedIds.push_back(id);
}

void CoapDtlsTestPeer::established(dtls::IDtls::AssociationId id)
{
    Enter_Method("established");
    activeSessions.insert(id);
    establishedIds.push_back(id);
    emit(controlStateChangedSignal, static_cast<long>(ESTABLISHED));
}

void CoapDtlsTestPeer::dataArrived(dtls::IDtls::AssociationId id, Packet *packet)
{
    Enter_Method("dataArrived");
    take(packet);
    emit(packetReceivedSignal, packet);
    emit(packetReceivedFromLowerSignal, packet);
    MemoryOutputStream stream;
    if (packet->getDataLength() > B(0))
        Chunk::serialize(stream, packet->peekData());
    ReceivedRecord rec{simTime(), id, stream.getData()};
    receivedRecords.push_back(rec);
    if (inboundCallback)
        inboundCallback(*this, rec);
    delete packet;
}

void CoapDtlsTestPeer::sendError(dtls::IDtls::AssociationId id, long packetId, const char *reason)
{
    Enter_Method("sendError");
    errors.emplace_back(id, reason != nullptr ? reason : "sendError");
    emit(controlStateChangedSignal, static_cast<long>(ERROR));
}

void CoapDtlsTestPeer::error(dtls::IDtls::AssociationId id, const char *reason)
{
    Enter_Method("error");
    errors.emplace_back(id, reason != nullptr ? reason : "error");
    emit(controlStateChangedSignal, static_cast<long>(ERROR));
}

void CoapDtlsTestPeer::closed(dtls::IDtls::AssociationId id)
{
    Enter_Method("closed");
    activeSessions.erase(id);
    closedIds.push_back(id);
    emit(controlStateChangedSignal, static_cast<long>(CLOSED));
}

} // namespace protocoltest
} // namespace inet
