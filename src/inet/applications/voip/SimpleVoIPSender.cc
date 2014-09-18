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
#include "inet/applications/voip/SimpleVoIPSender.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/applications/voip/SimpleVoIPPacket_m.h"

namespace inet {

Define_Module(SimpleVoIPSender);

SimpleVoIPSender::SimpleVoIPSender()
{
    selfSender = NULL;
    selfSource = NULL;
}

SimpleVoIPSender::~SimpleVoIPSender()
{
    cancelAndDelete(selfSender);
    cancelAndDelete(selfSource);
}

void SimpleVoIPSender::initialize(int stage)
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
        packetizationInterval = par("packetizationInterval").doubleValue();
        selfSender = new cMessage("selfSender");
        localPort = par("localPort");
        destPort = par("destPort");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        destAddress = L3AddressResolver().resolve(par("destAddress").stringValue());

        socket.setOutputGate(gate("udpOut"));
        socket.bind(localPort);

        EV_INFO << "VoIPSender::initialize - binding to port: local:" << localPort << " , dest:" << destPort << endl;

        // calculating traffic starting time
        simtime_t startTime = par("startTime").doubleValue();
        stopTime = par("stopTime").doubleValue();
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime settings: startTime %g s greater than stopTime %g s", SIMTIME_DBL(startTime), SIMTIME_DBL(stopTime));

        scheduleAt(startTime, selfSource);
        EV_INFO << "\t starting traffic in " << startTime << " s" << endl;
    }
}

void SimpleVoIPSender::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == selfSender)
            sendVoIPPacket();
        else
            selectPeriodTime();
    }
}

void SimpleVoIPSender::talkspurt(simtime_t dur)
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

void SimpleVoIPSender::selectPeriodTime()
{
    simtime_t now = simTime();
    if (stopTime >= SIMTIME_ZERO && now >= stopTime)
        return;

    if (isTalk) {
        silenceDuration = par("silenceDuration").doubleValue();
        EV_DEBUG << "SILENCE: " << "Duration: " << silenceDuration << " seconds\n\n";
        simtime_t endSilent = now + silenceDuration;
        if (stopTime >= SIMTIME_ZERO && endSilent > stopTime)
            endSilent = stopTime;
        scheduleAt(endSilent, selfSource);
        isTalk = false;
    }
    else {
        talkspurtDuration = par("talkspurtDuration").doubleValue();
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

void SimpleVoIPSender::sendVoIPPacket()
{
    SimpleVoIPPacket *packet = new SimpleVoIPPacket("VoIP");
    packet->setTalkspurtID(talkspurtID - 1);
    packet->setTalkspurtNumPackets(talkspurtNumPackets);
    packet->setPacketID(packetID);
    packet->setVoipTimestamp(simTime() - packetizationInterval);    // start time of voice in this packet
    packet->setVoiceDuration(packetizationInterval);
    packet->setByteLength(talkPacketSize);
    EV_INFO << "TALKSPURT " << talkspurtID - 1 << " sending packet " << packetID << "\n";

    socket.sendTo(packet, destAddress, destPort);
    ++packetID;

    if (packetID < talkspurtNumPackets)
        scheduleAt(simTime() + packetizationInterval, selfSender);
}

} // namespace inet

