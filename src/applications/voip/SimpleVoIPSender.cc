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
#include "SimpleVoIPSender.h"
#include "SimpleVoIPPacket_m.h"


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
    EV << "VoIP Sender initialize: stage " << stage << endl;

    // avoid multiple initializations
    if (stage != 3)
        return;

    talkspurtDuration = 0;
    silenceDuration = 0;
    selfSource = new cMessage("selfSource");
    isTalk = false;
    talkspurtID = 0;
    talkspurtNumPackets = 0;
    packetID = 0;
    timestamp = 0;
    talkPacketSize = par("talkPacketSize");
    packetizationInterval = par("packetizationInterval").doubleValue();
    selfSender = new cMessage("selfSender");
    localPort = par("localPort");
    destPort = par("destPort");
    destAddress = IPvXAddressResolver().resolve(par("destAddress").stringValue());


    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

    EV << "VoIPSender::initialize - binding to port: local:" << localPort << " , dest:" << destPort << endl;


    // calculating traffic starting time
    simtime_t startTime = par("startTime").doubleValue();
    stopTime = par("stopTime").doubleValue();
    if (stopTime != 0 && stopTime < startTime)
        throw cRuntimeError("Invalid startTime/stopTime settings: startTime %g s greater than stopTime %g s", SIMTIME_DBL(startTime), SIMTIME_DBL(stopTime));

    scheduleAt(startTime, selfSource);
    EV << "\t starting traffic in " << startTime << " s" << endl;
}

void SimpleVoIPSender::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
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
    if (selfSender->isScheduled())
    {
        // silence was too short, detected overlapping talkspurts
        simtime_t delta = selfSender->getArrivalTime() - curTime;
        startTime += delta;
        dur -= SIMTIME_DBL(delta);
        cancelEvent(selfSender);
    }

    talkspurtID++;
    packetID = 0;
    talkspurtNumPackets = (ceil(dur / packetizationInterval));
    EV << "TALKSPURT " << talkspurtID-1 << " will be sent " << talkspurtNumPackets << " packets\n\n";

    scheduleAt(startTime + packetizationInterval, selfSender);
}

void SimpleVoIPSender::selectPeriodTime()
{
    if (stopTime != 0 && simTime() >= stopTime)
        return;

    if (isTalk)
    {
        silenceDuration = par("silenceDuration").doubleValue();
        EV << "SILENCE: " << "Duration: " << silenceDuration << " seconds\n\n";
        simtime_t endSilent = simTime() + silenceDuration;
        if (stopTime != 0 && endSilent > stopTime)
            endSilent = stopTime;
        scheduleAt(endSilent, selfSource);
        isTalk = false;
    }
    else
    {
        talkspurtDuration = par("talkspurtDuration").doubleValue();
        EV << "TALKSPURT: " << talkspurtID << " Duration: " << talkspurtDuration << " seconds\n\n";
        simtime_t endTalk = simTime() + talkspurtDuration;
        if (stopTime != 0 && endTalk > stopTime)
        {
            endTalk = stopTime;
            talkspurtDuration = stopTime - simTime();
        }
        talkspurt(talkspurtDuration);
        scheduleAt(endTalk, selfSource);
        isTalk = true;
    }
}

void SimpleVoIPSender::sendVoIPPacket()
{
    SimpleVoIPPacket* packet = new SimpleVoIPPacket("VoIP");
    packet->setTalkspurtID(talkspurtID-1);
    packet->setTalkspurtNumPackets(talkspurtNumPackets);
    packet->setPacketID(packetID);
    packet->setVoipTimestamp(simTime() - packetizationInterval);    // start time of voice in this packet
    packet->setVoiceDuration(packetizationInterval);
    packet->setByteLength(talkPacketSize);
    EV << "TALKSPURT " << talkspurtID-1 << " sending packet " << packetID << "\n";

    socket.sendTo(packet, destAddress, destPort);
    ++packetID;

    if (packetID < talkspurtNumPackets)
        scheduleAt(simTime() + packetizationInterval, selfSender);
}

