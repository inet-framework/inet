//
// Protocol Test Framework for INET -- observer + matching engine.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ProtocolTester.h"
#include "ProtocolTestDescriber.h"

#include <iostream>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/DirectionTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

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

    // Matching mode: load the named program and arm the first step.
    std::string testName = par("testName").stdstringValue();
    if (!testName.empty()) {
        matchingMode = true;
        program = ProtocolTestRegistry::build(testName.c_str());
        if (par("printDescription").boolValue())
            std::cout << describe(*program);
        currentStep = 0;
        anchorTime = simTime();
        enterStep();
    }
}

ProtocolTester::~ProtocolTester()
{
    if (subscriptionModule != nullptr) {
        for (auto& it : signalKinds)
            subscriptionModule->unsubscribe(it.first, this);
    }
    cancelAndDelete(deadlineMsg);
    cancelAndDelete(injectMsg);
    cancelAndDelete(endMsg);
}

void ProtocolTester::handleMessage(cMessage *msg)
{
    if (msg == endMsg)
        endSimulation();
    else if (msg == deadlineMsg) {
        const auto& step = program->steps[currentStep].pattern;
        decide(false, "deadline missed for step " + std::to_string(currentStep) + " [" + step.str() + "]");
    }
    else if (msg == injectMsg) {
        performInjection(program->steps[currentStep].injection);
        anchorTime = simTime();
        currentStep++;
        enterStep();
    }
    else
        throw cRuntimeError("Unexpected message");
}

void ProtocolTester::finish()
{
    if (matchingMode) {
        if (!decided) {
            Step& step = program->steps[currentStep];
            std::string what = step.type == StepType::Expect ? "[" + step.pattern.str() + "]" : "[inject]";
            decide(false, "simulation ended with step " + std::to_string(currentStep) +
                          " still pending " + what);
        }
    }
    else
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
    if (matchingMode && !decided)
        processMatch(event);
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

    if (event.interfaceId != -1 && event.node != nullptr) {
        if (auto interfaceTable = L3AddressResolver().findInterfaceTableOf(event.node))
            if (auto networkInterface = interfaceTable->getInterfaceById(event.interfaceId))
                event.interfaceName = networkInterface->getInterfaceName();
    }

    event.layer = inferLayer(source);
    return event;
}

Layer ProtocolTester::inferLayer(const cComponent *source)
{
    // Coarse heuristic from the module path; refined in later phases when selectors
    // need precise layer classification.
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

    std::string interfaceLabel = !event.interfaceName.empty() ? event.interfaceName
                               : event.interfaceId != -1 ? std::to_string(event.interfaceId) : "-";

    // Printed on stdout (not EV) so the trace is clean and parseable regardless of
    // the express-mode / log-level settings of the user interface.
    std::cout << "PE"
              << " t=" << event.time
              << " node=" << (event.node ? event.node->getFullName() : "?")
              << " mod=" << event.module->getFullPath()
              << " dir=" << directionName
              << " kind=" << getEventKindName(event.kind)
              << " layer=" << getLayerName(event.layer)
              << " if=" << interfaceLabel
              << " proto=" << protocolName
              << " name=" << event.packet->getName()
              << " len=" << event.packet->getByteLength() << "B"
              << " treeId=" << event.treeId
              << std::endl;
}

void ProtocolTester::enterStep()
{
    if (currentStep >= program->steps.size()) {
        decide(true, "all steps matched");
        return;
    }
    Step& step = program->steps[currentStep];
    if (step.type == StepType::Expect)
        armCurrentDeadline();   // wait for a matching event (see processMatch)
    else {
        // Inject step: schedule the injection, then advance once it fires.
        const Injection& injection = step.injection;
        simtime_t fireTime = injection.hasAtTime ? injection.atTime
                           : injection.hasAfter ? anchorTime + injection.afterDelay
                           : simTime();
        if (injectMsg == nullptr)
            injectMsg = new cMessage("inject");
        scheduleAt(fireTime, injectMsg);
    }
}

void ProtocolTester::processMatch(const PacketEvent& event)
{
    // receiveSignal runs in the emitting module's context; switch to ours so the
    // engine's scheduleAt()/cancelEvent() and self-message ownership are valid.
    Enter_Method_Silent("processMatch");

    if (currentStep >= program->steps.size())
        return;
    Step& current = program->steps[currentStep];
    if (current.type != StepType::Expect)   // while an inject is pending, ignore events
        return;
    const EventPattern& step = current.pattern;

    // Earliest gate: events before the window opens cannot satisfy this step.
    if (step.selHasNotBefore && event.time < anchorTime + step.selNotBefore)
        return;
    // Open-world: events that don't match the current step are simply ignored.
    MatchContext context{event, captureStore};
    if (!step.selectorMatches(context))
        return;

    // Bind this step's captures from the matched packet, for use by later steps.
    for (auto& capture : step.captures)
        captureStore[capture.first] = capture.second(event);

    EV_INFO << "ProtocolTest " << program->name << ": step " << currentStep
            << " matched at t=" << event.time << " by " << event.module->getFullPath() << endl;

    cancelDeadline();
    anchorTime = event.time;
    currentStep++;
    enterStep();
}

void ProtocolTester::performInjection(const Injection& injection)
{
    cModule *node = getSystemModule()->getSubmodule(injection.nodeName.c_str());
    if (node == nullptr)
        throw cRuntimeError("inject: node '%s' not found", injection.nodeName.c_str());
    cModule *target = injection.moduleSubPath.empty()
                    ? node
                    : node->getModuleByPath(("." + injection.moduleSubPath).c_str());
    if (target == nullptr)
        throw cRuntimeError("inject: module '%s' not found under node '%s'",
                injection.moduleSubPath.c_str(), injection.nodeName.c_str());

    auto sink = dynamic_cast<queueing::IPassivePacketSink *>(target);
    if (sink == nullptr)
        throw cRuntimeError("inject: module '%s' is not an IPassivePacketSink", target->getFullPath().c_str());
    cGate *gate = target->gate(injection.gateName.c_str());
    if (gate == nullptr)
        throw cRuntimeError("inject: gate '%s' not found on '%s'", injection.gateName.c_str(), target->getFullPath().c_str());

    Packet *packet = injection.builder(captureStore);
    EV_INFO << "ProtocolTest " << program->name << ": injecting " << packet->getName()
            << " into " << target->getFullPath() << "." << injection.gateName
            << " at t=" << simTime() << endl;
    // pushPacket() does its own Enter_Method; the interface adds InterfaceInd etc.
    sink->pushPacket(packet, gate);
}

void ProtocolTester::armCurrentDeadline()
{
    const auto& step = program->steps[currentStep].pattern;
    if (step.selHasWithin) {
        if (deadlineMsg == nullptr)
            deadlineMsg = new cMessage("deadline");
        if (deadlineMsg->isScheduled())
            cancelEvent(deadlineMsg);
        scheduleAt(anchorTime + step.selWithin, deadlineMsg);
    }
}

void ProtocolTester::cancelDeadline()
{
    if (deadlineMsg != nullptr && deadlineMsg->isScheduled())
        cancelEvent(deadlineMsg);
}

void ProtocolTester::decide(bool pass, const std::string& reason)
{
    if (decided)
        return;
    decided = true;
    verdictPass = pass;
    cancelDeadline();

    std::cout << "PROTOCOLTEST " << program->name << ": " << (pass ? "PASS" : "FAIL") << std::endl;
    if (!pass)
        std::cout << "  reason: " << reason << std::endl;

    // End the simulation right after the current event so the verdict is prompt.
    if (endMsg == nullptr)
        endMsg = new cMessage("endTest");
    if (!endMsg->isScheduled())
        scheduleAt(simTime(), endMsg);
}

} // namespace protocoltest
} // namespace inet
