//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/AnimatePacket.h"

namespace inet {
namespace queueing {

void animate(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions, Action action)
{
    ASSERT(startGate);
    ASSERT(endGate);
    packet->setIsUpdate(sendOptions.isUpdate);
    packet->setTransmissionId(sendOptions.transmissionId_);
    if (sendOptions.isUpdate && sendOptions.transmissionId_ == -1)
        throw cRuntimeError("No transmissionId specified in SendOptions for a transmission update");
    packet->setDuration(SIMTIME_ZERO);
    packet->setRemainingDuration(SIMTIME_ZERO);
    packet->setArrival(endGate->getOwnerModule()->getId(), endGate->getId(), simTime());
    packet->setSentFrom(startGate->getOwnerModule(), startGate->getId(), simTime());

#ifdef INET_WITH_SELFDOC
    if (SelfDoc::generateSelfdoc) {
        auto from = startGate->getOwnerModule();
        auto fromName = from->getComponentType()->getFullName();
        auto to = endGate->getOwnerModule();
        auto toName = to->getComponentType()->getFullName();
        auto ctrl = packet->getControlInfo();
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("module", fromName)
                    << ", " << SelfDoc::keyVal("action", action == PUSH ? "PUSH_OUT" : "PULLED_OUT")
                    << ", " << SelfDoc::val("details") << " : {"
                    << SelfDoc::keyVal("gate", SelfDoc::gateInfo(startGate))
                    << ", "<< SelfDoc::keyVal("msg", opp_typename(typeid(*packet)))
                    << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(packet->getKind(), startGate->getProperties(), "messageKinds", endGate->getProperties(), "messageKinds"))
                    << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                    << ", " << SelfDoc::tagsToJson("tags", packet)
                    << ", " << SelfDoc::keyVal("destModule", toName)
                    << " } }"
                    ;
            globalSelfDoc.insert(os.str());
        }
        {
            std::ostringstream os;
            os << "=SelfDoc={ " << SelfDoc::keyVal("module", toName)
                    << ", " << SelfDoc::keyVal("action", action == PUSH ? "PUSHED_IN" : "PULL_IN")
                    << ", " << SelfDoc::val("details") << " : {"
                    << SelfDoc::keyVal("gate", SelfDoc::gateInfo(endGate))
                    << ", " << SelfDoc::keyVal("msg", opp_typename(typeid(*packet)))
                    << ", " << SelfDoc::keyVal("kind", SelfDoc::kindToStr(packet->getKind(), endGate->getProperties(), "messageKinds", startGate->getProperties(), "messageKinds"))
                    << ", " << SelfDoc::keyVal("ctrl", ctrl ? opp_typename(typeid(*ctrl)) : "")
                    << ", " << SelfDoc::tagsToJson("tags", packet)
                    << ", " << SelfDoc::keyVal("srcModule", fromName)
                    << " } }"
                    ;
            globalSelfDoc.insert(os.str());
        }
    }
#endif // INET_WITH_SELFDOC

    auto envir = getEnvir();
    auto gate = startGate;
    if (gate->getNextGate() != nullptr) {
        envir->beginSend(packet, sendOptions);
        while (gate->getNextGate() != nullptr && gate != endGate) {
            ChannelResult result;
            result.duration = sendOptions.duration_;
            result.remainingDuration = sendOptions.remainingDuration;
            envir->messageSendHop(packet, gate, result);
            gate = gate->getNextGate();
        }
        envir->endSend(packet);
    }
    envir->pausePoint();
}

void animatePacket(Packet *packet, cGate *startGate, cGate *endGate, Action action)
{
    SendOptions sendOptions;
    sendOptions.duration_ = 0;
    sendOptions.remainingDuration = 0;
    animate(packet, startGate, endGate, sendOptions, action);
}

void animatePacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId, Action action)
{
    simtime_t duration = s(packet->getDataLength() / datarate).get();
    SendOptions sendOptions;
    sendOptions.duration_ = duration;
    sendOptions.remainingDuration = duration;
    sendOptions.transmissionId(transmissionId);
    animatePacketStart(packet, startGate, endGate, datarate, sendOptions, action);
}

void animatePacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions, Action action)
{
    animate(packet, startGate, endGate, sendOptions, action);
}

void animatePacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId, Action action)
{
    SendOptions sendOptions;
    sendOptions.updateTx(transmissionId, 0);
    animatePacketEnd(packet, startGate, endGate, sendOptions, action);
}

void animatePacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions, Action action)
{
    animate(packet, startGate, endGate, sendOptions, action);
}

void animatePacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId, Action action)
{
    SendOptions sendOptions;
    sendOptions.transmissionId(transmissionId);
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, sendOptions, action);
}

void animatePacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions, Action action)
{
    animate(packet, startGate, endGate, sendOptions, action);
}

void animatePush(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions)
{
    animate(packet, startGate, endGate, sendOptions, PUSH);
}

void animatePushPacket(Packet *packet, cGate *startGate, cGate *endGate)
{
    animatePacket(packet, startGate, endGate, PUSH);
}

void animatePushPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId)
{
    animatePacketStart(packet, startGate, endGate, datarate, transmissionId, PUSH);
}

void animatePushPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions)
{
    animatePacketStart(packet, startGate, endGate, datarate, sendOptions, PUSH);
}

void animatePushPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId)
{
    animatePacketEnd(packet, startGate, endGate, transmissionId, PUSH);
}

void animatePushPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions)
{
    animatePacketEnd(packet, startGate, endGate, sendOptions, PUSH);
}

void animatePushPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId)
{
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, transmissionId, PUSH);
}

void animatePushPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions)
{
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, sendOptions, PUSH);
}

void animatePull(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions)
{
    animate(packet, startGate, endGate, sendOptions, PULL);
}

void animatePullPacket(Packet *packet, cGate *startGate, cGate *endGate)
{
    animatePacket(packet, startGate, endGate, PULL);
}

void animatePullPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId)
{
    animatePacketStart(packet, startGate, endGate, datarate, transmissionId, PULL);
}

void animatePullPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions)
{
    animatePacketStart(packet, startGate, endGate, datarate, sendOptions, PULL);
}

void animatePullPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId)
{
    animatePacketEnd(packet, startGate, endGate, transmissionId, PULL);
}

void animatePullPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions)
{
    animatePacketEnd(packet, startGate, endGate, sendOptions, PULL);
}

void animatePullPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId)
{
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, transmissionId, PULL);
}

void animatePullPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions)
{
    animatePacketProgress(packet, startGate, endGate, datarate, position, extraProcessableLength, sendOptions, PULL);
}

} // namespace queueing
} // namespace inet

