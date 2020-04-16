//
// Copyright (C) 2011 Adriano (University of Pisa)
// Copyright (C) 2012 Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <cmath>

#include "inet/applications/voip/SimpleVoipPacket_m.h"
#include "inet/applications/voip/SimpleVoipSender.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

Define_Module(SimpleVoipSender);

SimpleVoipSender::SimpleVoipSender()
{
}

SimpleVoipSender::~SimpleVoipSender()
{
    cancelAndDelete(selfSender);
    cancelAndDelete(selfSource);
}

void SimpleVoipSender::initialize(int stage)
{
    EV_TRACE << "VoIP Sender initialize: stage " << stage << endl;

    cSimpleModule::initialize(stage);

    // avoid multiple initializations
    if (stage == INITSTAGE_LOCAL) {
        talkspurtDuration = 0;
        silenceDuration = 0;
        selfSource = new cMessage("selfSource");
        isTalk = false;
        talkspurtID = 0;
        talkspurtNumPackets = 0;
        packetID = 0;
        talkPacketSize = par("talkPacketSize");
        packetizationInterval = par("packetizationInterval");
        selfSender = new cMessage("selfSender");
        localPort = par("localPort");
        destPort = par("destPort");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);

        EV_INFO << "VoIPSender::initialize - binding to port: local:" << localPort << " , dest:" << destPort << endl;

        // calculating traffic starting time
        simtime_t startTime(par("startTime"));
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime settings: startTime %g s greater than stopTime %g s", SIMTIME_DBL(startTime), SIMTIME_DBL(stopTime));

        scheduleAt(startTime, selfSource);
        EV_INFO << "\t starting traffic in " << startTime << " s" << endl;
    }
}

void SimpleVoipSender::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == selfSender)
            sendVoIPPacket();
        else
            selectTalkOrSilenceInterval();
    }
    else
        throw cRuntimeError("Unknown incoming message: '%s' on gate '%s'", msg->getClassName(), msg->getArrivalGate()->getFullName());
}

void SimpleVoipSender::talkspurt(simtime_t dur)
{
    simtime_t curTime = simTime();
    simtime_t startTime = curTime;
    if (selfSender->isScheduled()) {
        // silence was too short, detected overlapping talkspurts
        simtime_t delta = selfSender->getArrivalTime() - curTime;
        startTime += delta;
        dur -= SIMTIME_DBL(delta);
        cancelEvent(selfSender);
    }

    talkspurtID++;
    packetID = 0;
    talkspurtNumPackets = (ceil(dur / packetizationInterval));
    EV_DEBUG << "TALKSPURT " << talkspurtID - 1 << " will be sent " << talkspurtNumPackets << " packets\n\n";

    scheduleAt(startTime + packetizationInterval, selfSender);
}

void SimpleVoipSender::selectTalkOrSilenceInterval()
{
    simtime_t now = simTime();
    if (stopTime >= SIMTIME_ZERO && now >= stopTime)
        return;

    if (isTalk) {
        silenceDuration = par("silenceDuration");
        EV_DEBUG << "SILENCE: " << "Duration: " << silenceDuration << " seconds\n\n";
        simtime_t endSilence = now + silenceDuration;
        if (stopTime >= SIMTIME_ZERO && endSilence > stopTime)
            endSilence = stopTime;
        scheduleAt(endSilence, selfSource);
        isTalk = false;
    }
    else {
        talkspurtDuration = par("talkspurtDuration");
        EV_DEBUG << "TALKSPURT: " << talkspurtID << " Duration: " << talkspurtDuration << " seconds\n\n";
        simtime_t endTalk = now + talkspurtDuration;
        if (stopTime >= SIMTIME_ZERO && endTalk > stopTime) {
            endTalk = stopTime;
            talkspurtDuration = stopTime - now;
        }
        talkspurt(talkspurtDuration);
        scheduleAt(endTalk, selfSource);
        isTalk = true;
    }
}

void SimpleVoipSender::sendVoIPPacket()
{
    if (destAddress.isUnspecified())
        destAddress = L3AddressResolver().resolve(par("destAddress"));

    Packet *packet = new Packet("VoIP");
    const auto& voice = makeShared<SimpleVoipPacket>();
    voice->setTalkspurtID(talkspurtID - 1);
    voice->setTalkspurtNumPackets(talkspurtNumPackets);
    voice->setPacketID(packetID);
    voice->setVoipTimestamp(simTime() - packetizationInterval);    // start time of voice in this packet
    voice->setVoiceDuration(packetizationInterval);
    voice->setTotalLengthField(talkPacketSize);
    voice->setChunkLength(B(talkPacketSize));
    packet->insertAtBack(voice);

    EV_INFO << "TALKSPURT " << talkspurtID - 1 << " sending packet " << packetID << "\n";

    socket.sendTo(packet, destAddress, destPort);
    ++packetID;

    if (packetID < talkspurtNumPackets)
        scheduleAt(simTime() + packetizationInterval, selfSender);
}

} // namespace inet

