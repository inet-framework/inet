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
    matchExpression = par("matchExpression").stdstringValue();
    minPacketBytes = par("minPacketBytes").intValue();
    action = par("action").stdstringValue();
    occurrence = par("occurrence").intValue();
    delayTime = par("delayTime");
    compileFilter();
}

void PacketTap::compileFilter()
{
    if (!matchExpression.empty()) {
        filter.setExpression(matchExpression.c_str());
        hasFilter = true;
    }
    else
        hasFilter = false;
}

void PacketTap::configure(const std::string& matchExpr, long minBytes, int occ,
                          const std::string& act, simtime_t delay, std::function<void(Packet *)> mut)
{
    matchExpression = matchExpr;
    minPacketBytes = minBytes;
    occurrence = occ;
    action = act;
    delayTime = delay;
    mutator = std::move(mut);
    numSelected = 0;
    compileFilter();
    programmaticallyConfigured = true;
}

cGate *PacketTap::forwardGate(const cGate *arrivalGate)
{
    // Frames in on side "a" leave on side "b" and vice versa.
    return gate(std::string(arrivalGate->getBaseName()) == "a" ? "b$o" : "a$o");
}

bool PacketTap::isSelected(Packet *packet)
{
    if (hasFilter) {
        bool matched = false;
        try {
            matched = filter.matches(packet);
        }
        catch (const std::exception&) {
            // An expression that doesn't apply to this frame (e.g. a tcp.* test on an
            // ARP frame) is simply a non-match, never an error.
            matched = false;
        }
        if (!matched)
            return false;
    }
    if (minPacketBytes > 0 && packet->getByteLength() < minPacketBytes)
        return false;
    numSelected++;
    return occurrence == 0 || numSelected == occurrence;
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

    if (inner != nullptr && isSelected(inner)) {
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
