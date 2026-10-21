//
// Protocol Test Framework for INET -- a radio medium that is a tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "TapRadioMedium.h"

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"

namespace inet {
namespace protocoltest {

using namespace inet::physicallayer;

Define_Module(TapRadioMedium);

TapRadioMedium::~TapRadioMedium()
{
    for (auto& it : pendingCopies) {
        cancelAndDelete(it.first);
        delete it.second.packet;
    }
}

int TapRadioMedium::addInterception(const Interception& interception, const CaptureStore *captures)
{
    if (interception.action == "delay")
        throw cRuntimeError("tap '%s': a radio signal cannot arrive late, so a rule on a radio medium cannot delay() a frame",
                            interception.tapName.c_str());
    if (interception.action == "duplicate" && interception.spacing <= SIMTIME_ZERO)
        throw cRuntimeError("tap '%s': duplicate() on a radio medium needs a spacing above 0, because the medium refuses two transmissions that start at the same time",
                            interception.tapName.c_str());
    return rules.add(interception, captures);
}

void TapRadioMedium::startInterception(int handle)
{
    Enter_Method("startInterception");
    rules.start(handle);
}

std::string TapRadioMedium::getNodeName(const IRadio *radio)
{
    auto module = check_and_cast<const cModule *>(radio);
    auto node = findContainingNode(module);
    return (node != nullptr ? node : module)->getFullName();
}

void TapRadioMedium::checkRuleNodes()
{
    // The radios register in a later stage of initialization than the one in which the
    // tester installs the rules, so the nodes are checked when the first frame starts.
    std::set<std::string> nodes;
    communicationCache->mapRadios([&] (const IRadio *radio) {
        if (radio != nullptr)
            nodes.insert(getNodeName(radio));
    });
    for (const auto& rule : rules.getRules())
        for (const auto& node : { rule.interception.fromNode, rule.interception.toNode })
            if (!node.empty() && nodes.find(node) == nodes.end())
                throw cRuntimeError("tap '%s': '%s' has no radio on this medium", rule.interception.tapName.c_str(), node.c_str());
    rulesChecked = true;
}

void TapRadioMedium::addTransmission(const IRadio *transmitter, const ITransmission *transmission)
{
    // Decide before the medium sends a signal or computes energy for the new frame, so that
    // every later question about it finds the decision.
    decide(transmitter, transmission);
    RadioMedium::addTransmission(transmitter, transmission);
}

void TapRadioMedium::decide(const IRadio *transmitter, const ITransmission *transmission)
{
    if (!rulesChecked)
        checkRuleNodes();
    const Packet *packet = transmission->getPacket();
    std::string fromNode = getNodeName(transmitter);
    const InterceptionRules::Rule *copyOf = copyInTransmission;
    const InterceptionRules::Rule *duplicateRule = nullptr;
    communicationCache->mapRadios([&] (const IRadio *receiver) {
        if (receiver == nullptr || receiver == transmitter || receiver->getReceiver() == nullptr)
            return;
        std::string toNode = getNodeName(receiver);
        Decision decision;
        // A copy of a rule that names to(node) exists only for that node.
        if (copyOf != nullptr && !copyOf->interception.toNode.empty() && copyOf->interception.toNode != toNode)
            decision.effect = Effect::DROP;
        else if (auto rule = rules.select({packet, fromNode, toNode, copyOf, transmission->getId()})) {
            const auto& action = rule->interception.action;
            decision.rule = rule;
            if (action == "drop")
                decision.effect = Effect::DROP;
            else if (action == "corrupt")
                decision.effect = Effect::CORRUPT;
            else if (action == "mutate")
                decision.effect = Effect::MUTATE;
            else if (action == "duplicate")
                duplicateRule = rule;
        }
        if (decision.effect != Effect::NONE) {
            EV_INFO << "TapRadioMedium: " << packet->getName() << " to " << toNode << ": "
                    << (decision.rule != nullptr ? decision.rule->interception.action : std::string("drop (the copy is for another node)")) << endl;
            decisions[{transmission->getId(), receiver->getId()}] = decision;
            if (decision.effect == Effect::DROP)
                numDropped++;
        }
    });
    if (duplicateRule != nullptr) {
        // The copies are new transmissions from the same transmitter, so each receiver in
        // range gets them, and the medium keeps its own arrival data consistent.
        EV_INFO << "TapRadioMedium: duplicating " << packet->getName() << endl;
        for (int k = 1; k <= duplicateRule->interception.numCopies; k++) {
            auto timer = new cMessage("copy");
            pendingCopies[timer] = {transmitter, packet->dup(), duplicateRule};
            scheduleAt(transmission->getStartTime() + k * duplicateRule->interception.spacing, timer);
        }
    }
}

TapRadioMedium::Decision TapRadioMedium::getDecision(const IRadio *receiver, const ITransmission *transmission) const
{
    auto it = decisions.find({transmission->getId(), receiver->getId()});
    return it != decisions.end() ? it->second : Decision();
}

bool TapRadioMedium::isDropped(const IRadio *receiver, const ITransmission *transmission) const
{
    return getDecision(receiver, transmission).effect == Effect::DROP;
}

bool TapRadioMedium::isPotentialReceiver(const IRadio *receiver, const ITransmission *transmission) const
{
    // Both ways to give a receiver its signal ask this: sending it when the frame starts, and
    // picking it up when the receiver turns on later.
    return !isDropped(receiver, transmission) && RadioMedium::isPotentialReceiver(receiver, transmission);
}

bool TapRadioMedium::isInterferingTransmission(const ITransmission *transmission, const IListening *listening) const
{
    // The energy for the carrier sense of the receiver.
    return !isDropped(listening->getReceiverRadio(), transmission) && RadioMedium::isInterferingTransmission(transmission, listening);
}

bool TapRadioMedium::isInterferingTransmission(const ITransmission *transmission, const IReception *reception) const
{
    // The energy for the SNIR of the other receptions of the receiver.
    return !isDropped(reception->getReceiverRadio(), transmission) && RadioMedium::isInterferingTransmission(transmission, reception);
}

const IReceptionDecision *TapRadioMedium::computeReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    auto decision = RadioMedium::computeReceptionDecision(receiver, listening, transmission, part);
    // The data part fails to decode, so the receiver sets the bit error of the packet itself,
    // and the radio, the error indication and the MAC see the same failure.
    if ((part == IRadioSignal::SIGNAL_PART_DATA || part == IRadioSignal::SIGNAL_PART_WHOLE) &&
        decision->isReceptionSuccessful() && getDecision(receiver, transmission).effect == Effect::CORRUPT)
    {
        auto failed = new ReceptionDecision(decision->getReception(), part, decision->isReceptionPossible(), decision->isReceptionAttempted(), false);
        delete decision;
        return failed;
    }
    return decision;
}

Packet *TapRadioMedium::receivePacket(const IRadio *receiver, IWirelessSignal *signal)
{
    Enter_Method("receivePacket");
    auto packet = RadioMedium::receivePacket(receiver, signal);
    auto decision = getDecision(receiver, signal->getTransmission());
    if (decision.effect == Effect::MUTATE) {
        EV_INFO << "TapRadioMedium: mutating " << packet->getName() << " for " << getNodeName(receiver) << endl;
        if (decision.rule->interception.mutator)
            decision.rule->interception.mutator(packet);
        numMutated++;
    }
    else if (decision.effect == Effect::CORRUPT)
        numCorrupted++;
    return packet;
}

void TapRadioMedium::handleMessage(cMessage *message)
{
    auto it = pendingCopies.find(message);
    if (it == pendingCopies.end()) {
        RadioMedium::handleMessage(message);
        return;
    }
    PendingCopy copy = it->second;
    pendingCopies.erase(it);
    delete message;
    // A copy starts as a transmission of its own. decide() sees it through
    // copyInTransmission, so the copy does not go through the rule that made it.
    EV_INFO << "TapRadioMedium: transmitting a copy of " << copy.packet->getName() << endl;
    copyInTransmission = copy.rule;
    transmitPacket(copy.transmitter, copy.packet);
    copyInTransmission = nullptr;
    numCopied++;
}

void TapRadioMedium::finish()
{
    RadioMedium::finish();
    EV_INFO << "TapRadioMedium: dropped " << numDropped << ", corrupted " << numCorrupted << ", mutated " << numMutated
            << ", copied " << numCopied << " (" << rules.getNumSelected() << " selected)" << endl;
}

} // namespace protocoltest
} // namespace inet
