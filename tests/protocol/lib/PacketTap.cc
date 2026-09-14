//
// Protocol Test Framework for INET -- Phase 6: inline MITM packet tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "PacketTap.h"

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
    // A test program may have already driven us via configure(); if so, keep that and
    // ignore the parameters (configure() always wins, whatever the init order).
    if (programmaticallyConfigured)
        return;
    // The parameters describe a single rule, which is what an ini file can express.
    Rule rule;
    rule.matchExpression = par("matchExpression").stdstringValue();
    rule.minPacketBytes = par("minPacketBytes").intValue();
    rule.action = par("action").stdstringValue();
    rule.occurrence = par("occurrence").intValue();
    rule.delayTime = par("delayTime");
    rules.push_back(std::move(rule));
    compileRule(rules.back());
}

void PacketTap::compileRule(Rule& rule)
{
    if (!rule.matchExpression.empty()) {
        rule.filter = std::make_shared<PacketFilter>();
        rule.filter->setExpression(rule.matchExpression.c_str());
        rule.hasFilter = true;
    }
    else
        rule.hasFilter = false;
}

void PacketTap::configure(const std::string& matchExpr, long minBytes, int occ, int fromOcc,
                          const std::string& act, simtime_t delay, std::function<void(Packet *)> mut)
{
    // Append. The first call from a test program also discards whatever the parameters put
    // there, so an ini rule and a program rule never mix.
    if (!programmaticallyConfigured) {
        rules.clear();
        programmaticallyConfigured = true;
    }
    Rule rule;
    rule.matchExpression = matchExpr;
    rule.minPacketBytes = minBytes;
    rule.occurrence = occ;
    rule.fromOccurrence = fromOcc;
    rule.action = act;
    rule.delayTime = delay;
    rule.mutator = std::move(mut);
    rules.push_back(std::move(rule));
    compileRule(rules.back());
}

cGate *PacketTap::forwardGate(const cGate *arrivalGate)
{
    // Frames in on side "a" leave on side "b" and vice versa.
    return gate(std::string(arrivalGate->getBaseName()) == "a" ? "b$o" : "a$o");
}

PacketTap::Rule *PacketTap::selectRule(Packet *packet)
{
    // The rules are tried in order and the first that matches applies. A rule counts its
    // own occurrences, so two rules on one tap can name the second and the third frame of
    // the same kind without either counting the other's.
    for (auto& rule : rules) {
        if (rule.hasFilter) {
            bool matched = false;
            try {
                matched = rule.filter->matches(packet);
            }
            catch (const std::exception&) {
                // An expression that doesn't apply to this frame (e.g. a tcp.* test on an
                // ARP frame) is simply a non-match, never an error.
                matched = false;
            }
            if (!matched)
                continue;
        }
        if (rule.minPacketBytes > 0 && packet->getByteLength() < rule.minPacketBytes)
            continue;
        rule.numSelected++;
        bool wanted = rule.fromOccurrence > 0 ? rule.numSelected >= rule.fromOccurrence
                                              : (rule.occurrence == 0 || rule.numSelected == rule.occurrence);
        if (wanted) {
            numSelected++;
            return &rule;
        }
    }
    return nullptr;
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
        send(queue.pop(), out);
        numForwarded++;
        if (!queue.isEmpty())
            scheduleAt(out->getTransmissionChannel()->getTransmissionFinishTime(), timer);
    }
    else if (!timer->isScheduled()) {
        // Channel busy: try again the moment it frees up.
        scheduleAt(finish, timer);
    }
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

    cGate *out = forwardGate(msg->getArrivalGate());
    bool towardB = std::string(out->getName()) == "b$o";

    // On the Ethernet PHY gate the message is an EthernetSignal (a cPacket) that
    // encapsulates the frame Packet; on a plain message gate it is the Packet itself.
    auto frame = check_and_cast<cPacket *>(msg);
    auto inner = dynamic_cast<Packet *>(frame);
    if (inner == nullptr)
        inner = dynamic_cast<Packet *>(frame->getEncapsulatedPacket());

    Rule *rule = inner != nullptr ? selectRule(inner) : nullptr;
    if (rule != nullptr) {
        const std::string& action = rule->action;
        const simtime_t& delayTime = rule->delayTime;
        const auto& mutator = rule->mutator;
        if (action == "drop") {
            EV_INFO << "PacketTap dropping " << inner->getName() << " (selected #" << numSelected << ")" << endl;
            numDropped++;
            delete msg;
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
        // "pass": fall through to a plain forward.
    }

    enqueueForward(frame, towardB);
}

void PacketTap::finish()
{
    EV_INFO << "PacketTap: forwarded " << numForwarded << ", dropped " << numDropped
            << ", mutated " << numMutated << " (" << numSelected << " selected)" << endl;
}

} // namespace protocoltest
} // namespace inet
