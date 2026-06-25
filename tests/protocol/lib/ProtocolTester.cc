//
// Protocol Test Framework for INET -- Phase 0: observer / event normaliser.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ProtocolTester.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/DirectionTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {
namespace protocoltest {

Define_Module(ProtocolTester);

const char *getEventKindName(EventKind kind)
{
    switch (kind) {
        case EventKind::SentToLower: return "sentToLower";
        case EventKind::ReceivedFromLower: return "receivedFromLower";
        case EventKind::SentToUpper: return "sentToUpper";
        case EventKind::ReceivedFromUpper: return "receivedFromUpper";
        case EventKind::SentToPeer: return "sentToPeer";
        case EventKind::ReceivedFromPeer: return "receivedFromPeer";
        case EventKind::Dropped: return "dropped";
        default: return "other";
    }
}

const char *getLayerName(Layer layer)
{
    switch (layer) {
        case Layer::Physical: return "phy";
        case Layer::Link: return "link";
        case Layer::Network: return "network";
        case Layer::Transport: return "transport";
        case Layer::Application: return "app";
        default: return "?";
    }
}

void ProtocolTester::initialize()
{
    logEvents = par("logEvents");

    // Map the standard INET packet signals to normalised event kinds.
    signalKinds[packetSentToLowerSignal] = EventKind::SentToLower;
    signalKinds[packetReceivedFromLowerSignal] = EventKind::ReceivedFromLower;
    signalKinds[packetSentToUpperSignal] = EventKind::SentToUpper;
    signalKinds[packetReceivedFromUpperSignal] = EventKind::ReceivedFromUpper;
    signalKinds[packetSentToPeerSignal] = EventKind::SentToPeer;
    signalKinds[packetReceivedFromPeerSignal] = EventKind::ReceivedFromPeer;
    signalKinds[packetDroppedSignal] = EventKind::Dropped;

    // Subscribe network-wide (PcapRecorder style): signals propagate to ancestor
    // listeners, so attaching at the network module catches every node.
    subscriptionModule = getSystemModule();
    for (auto& it : signalKinds)
        subscriptionModule->subscribe(it.first, this);
}

ProtocolTester::~ProtocolTester()
{
    // Best-effort detach; safe-guarded in case teardown already removed the module.
    if (subscriptionModule != nullptr) {
        for (auto& it : signalKinds)
            subscriptionModule->unsubscribe(it.first, this);
    }
}

void ProtocolTester::finish()
{
    EV_INFO << "ProtocolTester observed " << numObserved << " packet events" << endl;
}

void ProtocolTester::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    auto it = signalKinds.find(signalID);
    if (it == signalKinds.end())
        return;
    auto packet = dynamic_cast<const Packet *>(obj);
    if (packet == nullptr)
        return;
    PacketEvent event = normalize(source, it->second, packet);
    numObserved++;
    if (logEvents)
        logEvent(event);
}

PacketEvent ProtocolTester::normalize(cComponent *source, EventKind kind, const Packet *packet)
{
    PacketEvent event;
    event.packet = packet;
    event.kind = kind;
    event.module = source;
    event.time = simTime();
    event.treeId = packet->getTreeId();

    if (auto module = dynamic_cast<cModule *>(source))
        event.node = findContainingNode(module);

    // Direction comes from the signal identity (the kind); packets generally do not
    // carry a DirectionTag at these observation points. DirectionTag is only a
    // fallback for kinds that don't imply a direction (e.g. Dropped).
    switch (kind) {
        case EventKind::SentToLower:
        case EventKind::ReceivedFromUpper:
        case EventKind::SentToPeer:
            event.direction = DIRECTION_OUTBOUND;
            break;
        case EventKind::ReceivedFromLower:
        case EventKind::SentToUpper:
        case EventKind::ReceivedFromPeer:
            event.direction = DIRECTION_INBOUND;
            break;
        default:
            if (auto directionTag = packet->findTag<DirectionTag>())
                event.direction = directionTag->getDirection();
            break;
    }

    if (auto interfaceInd = packet->findTag<InterfaceInd>())
        event.interfaceId = interfaceInd->getInterfaceId();
    else if (auto interfaceReq = packet->findTag<InterfaceReq>())
        event.interfaceId = interfaceReq->getInterfaceId();

    event.layer = inferLayer(source);
    return event;
}

Layer ProtocolTester::inferLayer(const cComponent *source)
{
    // Coarse Phase-0 heuristic from the module path; refined in later phases when
    // selectors need precise layer classification.
    std::string path = source->getFullPath();
    auto has = [&](const char *s) { return path.find(s) != std::string::npos; };
    if (has(".phy") || has("Phy")) return Layer::Physical;
    if (has(".mac") || has("Mac")) return Layer::Link;
    if (has(".ipv4") || has(".ipv6") || has(".ip.") || has(".network")) return Layer::Network;
    if (has(".tcp") || has(".udp") || has(".sctp")) return Layer::Transport;
    if (has(".app")) return Layer::Application;
    return Layer::Unknown;
}

void ProtocolTester::logEvent(const PacketEvent& event)
{
    const char *protocolName = "-";
    if (auto protocolTag = event.packet->findTag<PacketProtocolTag>())
        if (auto protocol = protocolTag->findProtocol())
            protocolName = protocol->getName();

    const char *directionName = event.direction == DIRECTION_INBOUND ? "IN"
                              : event.direction == DIRECTION_OUTBOUND ? "OUT" : "-";

    // Printed on stdout (not EV) so the trace is clean and parseable regardless of
    // the express-mode / log-level settings of the user interface.
    std::cout << "PE"
              << " t=" << event.time
              << " node=" << (event.node ? event.node->getFullName() : "?")
              << " mod=" << event.module->getFullPath()
              << " dir=" << directionName
              << " kind=" << getEventKindName(event.kind)
              << " layer=" << getLayerName(event.layer)
              << " if=" << event.interfaceId
              << " proto=" << protocolName
              << " name=" << event.packet->getName()
              << " len=" << event.packet->getByteLength() << "B"
              << " treeId=" << event.treeId
              << std::endl;
}

} // namespace protocoltest
} // namespace inet
