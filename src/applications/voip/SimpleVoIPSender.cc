/*
 * VoIPSender.cc
 *
 *  Created on: 25/gen/2011
 *      Author: Adriano
 */

#include <cmath>

#include "SimpleVoIPSender.h"

#include "SimpleVoIPPacket_m.h"


#define round(x) floor((x) + 0.5)

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

    durTalk = 0;
    durSil = 0;
    selfSource = new cMessage("selfSource");
    scaleTalk = par("scaleTalk");
    shapeTalk = par("shapeTalk");
    scaleSil = par("scaleSil");
    shapeSil = par("shapeSil");
    isTalk = false;
    talkspurtID = 0;
    talkspurtNumPackets = 0;
    packetID = 0;
    timestamp = 0;
    talkFrameSize = par("talkFrameSize");
    packetizationInterval = par("packetizationInterval");
    selfSender = new cMessage("selfSender");
    localPort = par("localPort");
    destPort = par("destPort");
    destAddress = IPvXAddressResolver().resolve(par("destAddress").stringValue());


    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

    EV << "VoIPSender::initialize - binding to port: local:" << localPort << " , dest:" << destPort << endl;


    // calculating traffic starting time
    // TODO correct this conversion
    //FIXME why need the round() ???
    simtime_t startTime = round(par("startTime").doubleValue() * 1000.0) / 1000.0;
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

void SimpleVoIPSender::talkspurt(double dur)
{
    talkspurtID++;
    talkspurtNumPackets = (ceil(dur/packetizationInterval));
    EV << "TALKSPURT " << talkspurtID-1 << " Verranno inviati " << talkspurtNumPackets << " packets\n\n";     //FIXME Translate!!!

    packetID = 0;
    // FIXME: why do we schedule a message for the current simulation time? why don't we rather call the method directly?
    scheduleAt(simTime(), selfSender);
}

void SimpleVoIPSender::selectPeriodTime()
{
    if (stopTime != 0 && simTime() >= stopTime)
        return;

    if (isTalk)
    {
        durSil = weibull(scaleSil, shapeSil);
        double durSil2 = round(durSil*1000)/1000;
        EV << "PERIODO SILENZIO: " << "Durata: " << durSil << "/" << durSil2 << " secondi\n\n";     //FIXME Translate!!!
        durSil = durSil2;
        simtime_t endSilent = simTime() + durSil;
        if (stopTime != 0 && endSilent > stopTime)
            endSilent = stopTime;
        scheduleAt(endSilent, selfSource);
        isTalk = false;
    }
    else
    {
        durTalk = weibull(scaleTalk, shapeTalk);
        double durTalk2 = round(durTalk*1000)/1000;
        EV << "TALKSPURT: " << talkspurtID << " Durata: " << durTalk << "/" << durTalk2 << " secondi\n\n";     //FIXME Translate!!!
        durTalk = durTalk2;
        simtime_t endTalk = simTime() + durTalk;
        if (stopTime != 0 && endTalk > stopTime)
        {
            endTalk = stopTime;
            durTalk = SIMTIME_DBL(stopTime - simTime());
        }
        talkspurt(durTalk);
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
    packet->setVoipTimestamp(simTime());
    packet->setVoiceDuration(packetizationInterval);
    packet->setByteLength(talkFrameSize);
    EV << "TALKSPURT " << talkspurtID-1 << " Invio frame " << packetID << "\n";     //FIXME Translate!!!

    socket.sendTo(packet, destAddress, destPort);
    ++packetID;

    if (packetID < talkspurtNumPackets)
        scheduleAt(simTime()+packetizationInterval, selfSender);
}

