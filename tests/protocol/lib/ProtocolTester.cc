//
// Protocol Test Framework for INET -- observer + matching engine.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ProtocolTester.h"
#include "ProtocolTestDescriber.h"
#include "PacketTap.h"

#include <iostream>
#include <set>
#include <sstream>

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

    // Simu5G RLC/PDCP boundary signals: the same inet::Packet* emissions under Simu5G's
    // own names. Selectors match by signal-name string, so these become observable as
    // soon as they are subscribed. (See Simu5G/tests/protocol/rlc/.)
    signalKinds[registerSignal("sentPacketToLowerLayer")] = EventKind::SentToLower;
    signalKinds[registerSignal("receivedPacketFromLowerLayer")] = EventKind::ReceivedFromLower;
    signalKinds[registerSignal("sentPacketToUpperLayer")] = EventKind::SentToUpper;
    signalKinds[registerSignal("receivedPacketFromUpperLayer")] = EventKind::ReceivedFromUpper;

    // Subscribe network-wide (PcapRecorder style): signals propagate to ancestor
    // listeners, so attaching at the network module catches every node.
    subscriptionModule = getSystemModule();
    for (auto& it : signalKinds)
        subscriptionModule->subscribe(it.first, this);

    // Matching mode: load a program and arm the first step. A `testName` selects a named
    // program from the registry; otherwise, if the build defines a single unnamed program
    // (Define_ProtocolTestProgram), run that -- no name or selection needed.
    std::string testName = par("testName").stdstringValue();
    if (!testName.empty()) {
        matchingMode = true;
        program = ProtocolTestRegistry::build(testName.c_str());
    }
    else if (ProtocolTestRegistry::hasDefault()) {
        matchingMode = true;
        program = ProtocolTestRegistry::buildDefault();
        logEvents = false;   // a program is running, so don't also dump the raw trace
    }

    // State channel: subscribe to the scalar signals named by the program's state steps
    // and/or the stateSignals parameter (the latter also enables the state-trace dump).
    subscribeStateSignals();

    if (matchingMode) {
        if (par("printDescription").boolValue())
            std::cout << describe(*program);
        installInterceptions();
        currentStep = 0;
        anchorTime = simTime();
        enterStep();
    }
}

void ProtocolTester::subscribeStateSignals()
{
    std::set<std::string> names;

    // Non-packet (scalar) signals named by the program's signal() steps. The packet signals
    // are already subscribed above (signalKinds); scalar signals (FSM state, counters, IDs)
    // are subscribed here and delivered to the intval_t receiveSignal overload.
    static const std::set<std::string> packetSignals = {
        "packetSentToLower", "packetReceivedFromLower", "packetSentToUpper",
        "packetReceivedFromUpper", "packetSentToPeer", "packetReceivedFromPeer", "packetDropped",
        // Simu5G RLC/PDCP boundary signals (subscribed via signalKinds in initialize()).
        "sentPacketToLowerLayer", "receivedPacketFromLowerLayer",
        "sentPacketToUpperLayer", "receivedPacketFromUpperLayer"
    };
    auto collect = [&](const EventPattern& p) {
        if (!p.selSignal.empty() && packetSignals.find(p.selSignal) == packetSignals.end())
            names.insert(p.selSignal);
    };
    if (matchingMode)
        for (const auto& step : program->steps) {
            collect(step.pattern);
            collect(step.pattern2);
            for (const auto& g : step.group)
                collect(g);
        }

    // signals named by the stateSignals parameter (space-separated) -- enables the trace dump
    std::string param = par("stateSignals").stdstringValue();
    std::istringstream iss(param);
    for (std::string name; iss >> name; )
        names.insert(name);
    traceState = !param.empty();

    // Register each name and subscribe network-wide; scalar signals propagate to the
    // ancestor listener just like the packet signals do.
    for (const auto& name : names) {
        simsignal_t signal = cComponent::registerSignal(name.c_str());
        stateSignalNames[signal] = name;
        subscriptionModule->subscribe(signal, this);
    }
}

void ProtocolTester::installInterceptions()
{
    // Push each standing intercept(...) rule onto its named PacketTap (a sibling module).
    for (const auto& interception : program->interceptions) {
        cModule *module = getParentModule()->getSubmodule(interception.tapName.c_str());
        auto tap = dynamic_cast<PacketTap *>(module);
        if (tap == nullptr)
            throw cRuntimeError("ProtocolTest '%s': intercept target '%s' is not a PacketTap",
                                program->name.c_str(), interception.tapName.c_str());
        tap->configure(interception.matchExpression, interception.minimumBytes, interception.occurrence,
                       interception.action, interception.delayTime, interception.mutator);
    }
}

ProtocolTester::~ProtocolTester()
{
    if (subscriptionModule != nullptr) {
        for (auto& it : signalKinds)
            subscriptionModule->unsubscribe(it.first, this);
        for (auto& it : stateSignalNames)
            subscriptionModule->unsubscribe(it.first, this);
    }
    cancelAndDelete(deadlineMsg);
    cancelAndDelete(injectMsg);
    cancelAndDelete(endMsg);
}

void ProtocolTester::handleMessage(cMessage *msg)
{
    if (msg == endMsg) {
        endSimulation();
        return;
    }
    if (msg == injectMsg) {
        performInjection(program->steps[currentStep].injection);
        advance(simTime());
        return;
    }
    if (msg == deadlineMsg) {
        // Deadline expiry means different things per step kind.
        Step& step = program->steps[currentStep];
        switch (step.type) {
            case StepType::Once:
                decide(false, "deadline missed for step " + std::to_string(currentStep) + " [" + step.pattern.str() + "]");
                break;
            case StepType::Unordered:
                decide(false, "unordered step " + std::to_string(currentStep) + ": " +
                              std::to_string(groupRemaining) + " of " + std::to_string(step.group.size()) +
                              " patterns did not match within the window");
                break;
            case StepType::AnyOf:
                decide(false, "anyOf step " + std::to_string(currentStep) + ": no alternative matched within the window");
                break;
            case StepType::ExactlyTimes:
                decide(false, "exactlyTimes step " + std::to_string(currentStep) + ": " +
                              std::to_string(step.count - repeatRemaining) + " of " + std::to_string(step.count) +
                              " occurrences within the window [" + step.pattern.str() + "]");
                break;
            case StepType::Count:  // window closed: pass iff enough matches were seen (overflow already failed earlier)
                if (cardCount >= step.cardMin)
                    advance(simTime());
                else
                    decide(false, "count step " + std::to_string(currentStep) + ": " +
                                  std::to_string(cardCount) + " occurrence(s), need at least " +
                                  std::to_string(step.cardMin) + " [" + step.pattern.str() + "]");
                break;
            case StepType::Delivery:
                decide(false, "delivery step " + std::to_string(currentStep) +
                              ": the matching receive did not arrive within the window after the send");
                break;
            case StepType::Never:        // window passed with no violation -> success
            case StepType::AtMostOnce:   // window passed with no match -> skip
                advance(simTime());
                break;
            default:
                break;
        }
        return;
    }
    throw cRuntimeError("Unexpected message");
}

void ProtocolTester::finish()
{
    if (!matchingMode) {
        EV_INFO << "ProtocolTester observed " << numObserved << " packet events" << endl;
        return;
    }
    finishing = true;
    if (decided)
        return;

    // Steps satisfied by reaching end-of-sim without a violation/match resolve as
    // success: Never (no forbidden event), AtMostOnce (a skipped optional), and any
    // Count whose accumulated occurrences already satisfy its range. The current
    // step carries the live cardCount; trailing steps have observed nothing, so reset
    // it to 0 as we walk past them.
    while (currentStep < program->steps.size()) {
        const Step& step = program->steps[currentStep];
        bool satisfied = step.type == StepType::Never ||
                         step.type == StepType::AtMostOnce ||
                         (step.type == StepType::Count &&
                          cardCount >= step.cardMin &&
                          (step.cardMax < 0 || cardCount <= step.cardMax));
        if (!satisfied)
            break;
        currentStep++;
        cardCount = 0;
    }

    if (currentStep >= program->steps.size())
        decide(true, "all steps satisfied at end of simulation");
    else {
        Step& step = program->steps[currentStep];
        std::string what = step.type == StepType::Unordered
                             ? "[unordered: " + std::to_string(groupRemaining) + " pattern(s) unmatched]"
                         : step.type == StepType::Inject ? "[inject pending]"
                         : "[" + step.pattern.str() + "]";
        decide(false, "simulation ended with step " + std::to_string(currentStep) + " pending " + what);
    }
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
    event.signalName = cComponent::getSignalName(signalID);
    numObserved++;
    if (logEvents)
        logEvent(event);
    if (matchingMode && !decided)
        processMatch(event);
}

void ProtocolTester::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    // State channel: only the scalar signals we explicitly subscribed to (FSM state,
    // counters, IDs). Everything else is ignored. Normalised into the same PacketEvent the
    // packet channel uses (no packet, a scalar value) and fed to the same matching engine.
    if (stateSignalNames.find(signalID) == stateSignalNames.end())
        return;
    PacketEvent event;
    event.module = source;
    if (auto module = dynamic_cast<cModule *>(source))
        event.node = findContainingNode(module);
    event.signalName = cComponent::getSignalName(signalID);
    std::string fullPath = source->getFullPath();
    auto firstDot = fullPath.find('.');
    event.sourcePath = (firstDot == std::string::npos) ? fullPath : fullPath.substr(firstDot + 1);
    event.hasValue = true;
    event.value = (long)value;
    event.time = simTime();
    numObserved++;
    if (traceState)
        std::cout << "SE t=" << event.time << " mod=" << fullPath
                  << " signal=" << event.signalName << " value=" << event.value << std::endl;
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

    if (auto protocolTag = packet->findTag<PacketProtocolTag>())
        if (auto protocol = protocolTag->findProtocol())
            event.protocolName = protocol->getName();
    if (auto dispatchReq = packet->findTag<DispatchProtocolReq>())
        if (auto protocol = dispatchReq->getProtocol())
            event.dispatchName = protocol->getName();

    // Source module path, relative to the network (strip the leading network-name component).
    std::string fullPath = source->getFullPath();
    auto firstDot = fullPath.find('.');
    event.sourcePath = (firstDot == std::string::npos) ? fullPath : fullPath.substr(firstDot + 1);

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
    const char *protocolName = event.protocolName.empty() ? "-" : event.protocolName.c_str();

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
    switch (step.type) {
        case StepType::Once:
        case StepType::AtMostOnce:
        case StepType::Never:
            // Wait for a matching event (see processMatch); the deadline resolves the
            // step if it expires (fail / skip / pass, depending on the kind).
            armDeadline(step.pattern.selHasWithin ? step.pattern.selWithin : simtime_t(0));
            break;
        case StepType::Unordered:
        case StepType::AnyOf: {
            groupMatched.assign(step.group.size(), 0);
            groupRemaining = (int)step.group.size();
            simtime_t window = 0;
            for (auto& pattern : step.group)
                if (pattern.selHasWithin && pattern.selWithin > window)
                    window = pattern.selWithin;
            armDeadline(window);
            if (step.type == StepType::Unordered && groupRemaining == 0)
                advance(simTime());
            break;
        }
        case StepType::ExactlyTimes:
            repeatRemaining = step.count;
            armDeadline(step.pattern.selHasWithin ? step.pattern.selWithin : simtime_t(0));
            if (repeatRemaining <= 0)
                advance(simTime());
            break;
        case StepType::Count:
            // Greedy: accumulate matches for the whole window, then resolve at the
            // deadline (see handleMessage). An overflow past cardMax fails earlier.
            cardCount = 0;
            armDeadline(step.pattern.selHasWithin ? step.pattern.selWithin : simtime_t(0));
            break;
        case StepType::Delivery:
            // Wait (unbounded) for the send; the window applies to send->receive and is
            // armed once the send is observed (see processMatch).
            deliveryStage = 0;
            deliveryTreeId = -1;
            break;
        case StepType::Inject: {
            const Injection& injection = step.injection;
            simtime_t fireTime = injection.hasAtTime ? injection.atTime
                               : injection.hasAfter ? anchorTime + injection.afterDelay
                               : simTime();
            if (injectMsg == nullptr)
                injectMsg = new cMessage("inject");
            scheduleAt(fireTime, injectMsg);
            break;
        }
    }
}

bool ProtocolTester::patternMatches(const EventPattern& pattern, const PacketEvent& event) const
{
    if (pattern.selHasNotBefore && event.time < anchorTime + pattern.selNotBefore)
        return false;
    MatchContext context{event, captureStore};
    return pattern.selectorMatches(context);
}

void ProtocolTester::runCaptures(const EventPattern& pattern, const PacketEvent& event)
{
    for (auto& capture : pattern.captures)
        captureStore[capture.first] = capture.second(event);
}

void ProtocolTester::advance(simtime_t at)
{
    cancelDeadline();
    anchorTime = at;
    currentStep++;
    enterStep();
}

void ProtocolTester::processMatch(const PacketEvent& event)
{
    // receiveSignal runs in the emitting module's context; switch to ours so the
    // engine's scheduleAt()/cancelEvent() and self-message ownership are valid.
    Enter_Method_Silent("processMatch");

    if (currentStep >= program->steps.size())
        return;
    Step& step = program->steps[currentStep];

    switch (step.type) {
        case StepType::Once:
        case StepType::AtMostOnce:
            // Open-world: non-matching events are ignored.
            if (patternMatches(step.pattern, event)) {
                runCaptures(step.pattern, event);
                EV_INFO << "ProtocolTest " << program->name << ": step " << currentStep
                        << " matched at t=" << event.time << " by " << event.module->getFullPath() << endl;
                advance(event.time);
            }
            // strict (closed-world): an in-scope packet that isn't the expected one fails.
            else if (program->strictMode && step.type == StepType::Once
                     && !(step.pattern.selHasNotBefore && event.time < anchorTime + step.pattern.selNotBefore)
                     && step.pattern.scopeMatches(event)) {
                decide(false, "strict: unexpected in-scope packet at t=" + event.time.str() +
                              " (" + event.module->getFullPath() + ") for step " +
                              std::to_string(currentStep) + " [" + step.pattern.str() + "]");
            }
            break;
        case StepType::AnyOf:
            for (auto& pattern : step.group) {
                if (patternMatches(pattern, event)) {
                    runCaptures(pattern, event);
                    advance(event.time);
                    break;
                }
            }
            break;
        case StepType::ExactlyTimes:
            if (patternMatches(step.pattern, event)) {
                runCaptures(step.pattern, event);
                if (--repeatRemaining <= 0)
                    advance(event.time);
            }
            break;
        case StepType::Count:
            // Greedy: count every match; an overflow past cardMax fails immediately,
            // otherwise keep consuming until the window closes (see handleMessage).
            if (patternMatches(step.pattern, event)) {
                runCaptures(step.pattern, event);
                cardCount++;
                if (step.cardMax >= 0 && cardCount > step.cardMax)
                    decide(false, "count step " + std::to_string(currentStep) + ": more than " +
                                  std::to_string(step.cardMax) + " occurrence(s) [" + step.pattern.str() + "]");
            }
            break;
        case StepType::Delivery:
            if (deliveryStage == 0) {
                if (patternMatches(step.pattern, event)) {
                    deliveryTreeId = event.treeId;
                    runCaptures(step.pattern, event);
                    deliveryStage = 1;
                    anchorTime = event.time;   // the receive window starts at the send
                    armDeadline(step.pattern2.selHasWithin ? step.pattern2.selWithin : simtime_t(0));
                }
            }
            else if (event.treeId == deliveryTreeId && patternMatches(step.pattern2, event)) {
                runCaptures(step.pattern2, event);
                advance(event.time);
            }
            break;
        case StepType::Never:
            // A match within the window is a violation.
            if (patternMatches(step.pattern, event))
                decide(false, "forbidden event occurred at t=" + event.time.str() +
                              " for step " + std::to_string(currentStep) + " [" + step.pattern.str() + "]");
            break;
        case StepType::Unordered:
            // Offer the event to each still-unmatched pattern; consume the first it satisfies.
            for (size_t i = 0; i < step.group.size(); i++) {
                if (!groupMatched[i] && patternMatches(step.group[i], event)) {
                    groupMatched[i] = 1;
                    runCaptures(step.group[i], event);
                    groupRemaining--;
                    EV_INFO << "ProtocolTest " << program->name << ": unordered step " << currentStep
                            << " matched pattern " << i << " at t=" << event.time << endl;
                    break;
                }
            }
            if (groupRemaining == 0)
                advance(event.time);
            break;
        default:
            break;   // Inject: ignore observed events while pending
    }
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

void ProtocolTester::armDeadline(simtime_t window)
{
    if (window <= SIMTIME_ZERO)
        return; // no window -> wait indefinitely (resolved at sim end)
    if (deadlineMsg == nullptr)
        deadlineMsg = new cMessage("deadline");
    if (deadlineMsg->isScheduled())
        cancelEvent(deadlineMsg);
    scheduleAt(anchorTime + window, deadlineMsg);
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
    // (Not when deciding from finish() -- the simulation is already ending.)
    if (!finishing) {
        if (endMsg == nullptr)
            endMsg = new cMessage("endTest");
        if (!endMsg->isScheduled())
            scheduleAt(simTime(), endMsg);
    }
}

} // namespace protocoltest
} // namespace inet
