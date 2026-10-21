//
// Protocol Test Framework for INET -- Phase 6: inline MITM packet tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "PacketTap.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace protocoltest {

Define_Module(PacketTap);

PacketTap::~PacketTap()
{
    cancelAndDelete(pumpA);
    cancelAndDelete(pumpB);
}

void PacketTap::initialize()
{
    pumpA = new cMessage("pumpA");
    pumpB = new cMessage("pumpB");
    // A test program may have already driven us via addInterception(); if so, keep that
    // and ignore the parameters (the program always wins, whatever the init order).
    if (programmaticallyConfigured)
        return;
    // The parameters describe a single rule, which is what an ini file can express.
    Interception rule;
    rule.tapName = getName();
    rule.matchExpression = par("matchExpression").stdstringValue();
    rule.minimumBytes = par("minPacketBytes").intValue();
    rule.action = par("action").stdstringValue();
    rule.occurrence = par("occurrence").intValue();
    rule.delayTime = par("delayTime");
    rules.add(rule);
}

void PacketTap::addInterception(const Interception& interception)
{
    // A node that is on neither side would make the rule select nothing, in silence.
    for (const auto& node : { interception.fromNode, interception.toNode })
        if (!node.empty() && node != getSideNode(false) && node != getSideNode(true))
            throw cRuntimeError("tap '%s': '%s' is on neither side of the tap (side a: '%s', side b: '%s')",
                                getFullName(), node.c_str(), getSideNode(false).c_str(), getSideNode(true).c_str());
    // Append. The first call from a test program also discards whatever the parameters put
    // there, so an ini rule and a program rule never mix.
    if (!programmaticallyConfigured) {
        rules.clear();
        programmaticallyConfigured = true;
    }
    rules.add(interception);
}

const std::string& PacketTap::getSideNode(bool sideB)
{
    // The node behind a side is the node that holds the far end of the path of that side.
    auto& node = sideB ? nodeB : nodeA;
    if (node.empty()) {
        auto module = gate(sideB ? "b$o" : "a$o")->getPathEndGate()->getOwnerModule();
        auto containingNode = findContainingNode(module);
        node = (containingNode != nullptr ? containingNode : module)->getFullName();
    }
    return node;
}

cGate *PacketTap::forwardGate(const cGate *arrivalGate)
{
    // Frames in on side "a" leave on side "b" and vice versa.
    return gate(std::string(arrivalGate->getBaseName()) == "a" ? "b$o" : "a$o");
}

void PacketTap::enqueueForward(cPacket *packet, bool towardB)
{
    if (towardB) {
        queueB.insert(packet);
        pump("b$o", queueB, pumpB);
    }
    else {
        queueA.insert(packet);
        pump("a$o", queueA, pumpA);
    }
}

void PacketTap::pump(const char *outGateName, cPacketQueue& queue, cMessage *timer)
{
    if (queue.isEmpty())
        return;
    cGate *out = gate(outGateName);
    cChannel *channel = out->getTransmissionChannel();
    simtime_t finish = channel->getTransmissionFinishTime();
    if (finish <= simTime()) {
        // Channel is free: transmit the head, then schedule the next drain at the new
        // transmission-finish time if more frames are waiting.
        auto packet = queue.pop();
        scheduleCopies(packet, timer == pumpB);
        send(packet, out);
        numForwarded++;
        if (!queue.isEmpty())
            scheduleAt(out->getTransmissionChannel()->getTransmissionFinishTime(), timer);
    }
    else if (!timer->isScheduled()) {
        // Channel busy: try again the moment it frees up.
        scheduleAt(finish, timer);
    }
}

void PacketTap::scheduleCopies(cPacket *frame, bool towardB)
{
    // The copies count from the moment the original starts on the outgoing channel, so a
    // busy channel delays the original and its copies alike.
    auto it = pendingCopies.find(frame);
    if (it == pendingCopies.end())
        return;
    const auto& rule = *it->second;
    for (int k = 1; k <= rule.interception.numCopies; k++) {
        auto holder = new cPacket("copy");
        holder->setKind(towardB ? 1 : 0);
        holder->setContextPointer(const_cast<InterceptionRules::Rule *>(&rule));
        holder->encapsulate(frame->dup());
        scheduleAt(simTime() + k * rule.interception.spacing, holder);
        numCopied++;
    }
    pendingCopies.erase(it);
}

void PacketTap::handleMessage(cMessage *msg)
{
    if (msg == pumpA) { pump("a$o", queueA, pumpA); return; }
    if (msg == pumpB) { pump("b$o", queueB, pumpB); return; }

    if (msg->isSelfMessage() && std::string(msg->getName()) == "release") {
        // A delayed frame becoming due: unwrap and forward it.
        auto release = check_and_cast<cPacket *>(msg);
        bool towardB = release->getKind() == 1;
        auto held = check_and_cast<cPacket *>(release->decapsulate());
        delete release;
        enqueueForward(held, towardB);
        return;
    }

    if (msg->isSelfMessage() && std::string(msg->getName()) == "copy") {
        // A copy becoming due enters the tap as a frame of its own, in the same direction.
        auto holder = check_and_cast<cPacket *>(msg);
        bool towardB = holder->getKind() == 1;
        auto copyOf = static_cast<const InterceptionRules::Rule *>(holder->getContextPointer());
        auto copy = check_and_cast<cPacket *>(holder->decapsulate());
        delete holder;
        processFrame(copy, towardB, copyOf);
        return;
    }

    cGate *out = forwardGate(msg->getArrivalGate());
    bool towardB = std::string(out->getName()) == "b$o";
    processFrame(check_and_cast<cPacket *>(msg), towardB, nullptr);
}

void PacketTap::processFrame(cPacket *frame, bool towardB, const InterceptionRules::Rule *copyOf)
{
    // On the Ethernet PHY gate the message is an EthernetSignal (a cPacket) that
    // encapsulates the frame Packet; on a plain message gate it is the Packet itself.
    auto inner = dynamic_cast<Packet *>(frame);
    if (inner == nullptr)
        inner = dynamic_cast<Packet *>(frame->getEncapsulatedPacket());

    InterceptionRules::Rule *rule = nullptr;
    if (inner != nullptr)
        rule = rules.select({inner, getSideNode(!towardB), getSideNode(towardB), copyOf});
    if (rule != nullptr) {
        const std::string& action = rule->interception.action;
        const simtime_t& delayTime = rule->interception.delayTime;
        const auto& mutator = rule->interception.mutator;
        long numSelected = rules.getNumSelected();
        if (action == "drop") {
            EV_INFO << "PacketTap dropping " << inner->getName() << " (selected #" << numSelected << ")" << endl;
            numDropped++;
            delete frame;
            return;
        }
        if (action == "delay") {
            EV_INFO << "PacketTap delaying " << inner->getName() << " by " << delayTime << endl;
            // Hold, then re-enter the normal forward path after the delay.
            auto release = new cPacket("release");
            release->setKind(towardB ? 1 : 0);
            release->encapsulate(frame);
            scheduleAt(simTime() + delayTime, release);
            return;
        }
        if (action == "mutate") {
            EV_INFO << "PacketTap mutating " << inner->getName() << " (selected #" << numSelected << ")" << endl;
            if (mutator)
                mutator(inner);
            numMutated++;
            // fall through to forward the (now mutated) frame
        }
        else if (action == "corrupt") {
            // Mark the object on the wire, as the bit error model of a channel does: the
            // EthernetSignal, or the Packet itself on a plain message gate.
            EV_INFO << "PacketTap corrupting " << inner->getName() << " (selected #" << numSelected << ")" << endl;
            frame->setBitError(true);
            numCorrupted++;
        }
        else if (action == "duplicate") {
            EV_INFO << "PacketTap duplicating " << inner->getName() << " (selected #" << numSelected << ")" << endl;
            pendingCopies[frame] = rule;
        }
        // "pass": fall through to a plain forward.
    }

    enqueueForward(frame, towardB);
}

void PacketTap::finish()
{
    EV_INFO << "PacketTap: forwarded " << numForwarded << ", dropped " << numDropped
            << ", mutated " << numMutated << ", corrupted " << numCorrupted << ", copied " << numCopied
            << " (" << rules.getNumSelected() << " selected)" << endl;
}

} // namespace protocoltest
} // namespace inet
