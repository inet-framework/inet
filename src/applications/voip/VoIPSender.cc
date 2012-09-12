/*
 * VoIPSender.cc
 *
 *  Created on: 25/gen/2011
 *      Author: Adriano
 */
#include "VoIPSender.h"
#include "cmath"

#define round(x) floor((x) + 0.5)

Define_Module(VoIPSender);

VoIPSender::VoIPSender()
{
    initialized_ = false;
    selfSender = NULL;
    selfSource = NULL;
}

VoIPSender::~VoIPSender()
{
    cancelAndDelete(selfSender);
    cancelAndDelete(selfSource);
}

void VoIPSender::initialize(int stage)
{
    EV<<"VoIP Sender initialize: stage "<<stage<< " - initialize=" << initialized_ << endl;

    // avoid multiple initializations
    if (stage!=3 || initialized_)
        return;

    durTalk       = 0;
    durSil        = 0;
    selfSource    = new cMessage("selfSource");
    scaleTalk    = par("scaleTalk");
    shapeTalk    = par("shapeTalk");
    scaleSil     = par("scaleSil");
    shapeSil     = par("shapeSil");
    isTalk       = false;
    talkID        = 0;
    nframes       = 0;
    nframes_tmp   = 0;
    frameID       = 0;
    timestamp     = 0;
    talkFrameSize = par("talkFrameSize");
    samplingTime  = par("samplingTime");
    selfSender    = new cMessage("selfSender");
    localPort     = par("localPort");
    destPort      = par("destPort");
    destAddress   = IPvXAddressResolver().resolve(par("destAddress").stringValue());


    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

    EV << "VoIPSender::initialize - binding to port: local:" << localPort << " , dest:"<< destPort << endl;


    // calculating traffic starting time
    // TODO correct this conversion
    //FIXME why need the round() ???
    simtime_t startTime = round(par("startTime").doubleValue() * 1000.0) / 1000.0;
    stopTime = par("stopTime").doubleValue();
    if (stopTime != 0 && stopTime < startTime)
        throw cRuntimeError("Invalid startTime/stopTime settings: startTime %g s greater than stopTime %g s", SIMTIME_DBL(startTime), SIMTIME_DBL(stopTime));

    scheduleAt(startTime, selfSource);
    EV << "\t starting traffic in " << startTime << " s" << endl;

    initialized_ = true;
}

void VoIPSender::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        if (msg == selfSender)
            sendVoIPPacket();
        else
            selectPeriodTime();
    }
}

void VoIPSender::talkspurt(double dur)
{
    talkID++;
    nframes=(ceil(dur/samplingTime));
    EV<<"TALKSPURT "<<talkID-1<<" Verranno inviati "<<nframes<<" frames\n\n";     //FIXME Translate!!!

    frameID = 0;
    nframes_tmp=nframes;
    // FIXME: why do we schedule a message for the current simulation time? why don't we rather call the method directly?
    scheduleAt(simTime(), selfSender);
}

void VoIPSender::selectPeriodTime()
{
    if (stopTime != 0 && simTime() >= stopTime)
        return;

    if(isTalk)
    {
        durSil=weibull(scaleSil, shapeSil);
        double durSil2 = round(durSil*1000)/1000;
        EV<<"PERIODO SILENZIO: "<<"Durata: "<<durSil<<"/" << durSil2<<" secondi\n\n";     //FIXME Translate!!!
        durSil = durSil2;
        simtime_t endSilent = simTime() + durSil;
        if (stopTime != 0 && endSilent > stopTime)
            endSilent = stopTime;
        scheduleAt(endSilent, selfSource);
        isTalk = false;
    }
    else
    {
        durTalk=weibull(scaleTalk, shapeTalk);
        double durTalk2 = round(durTalk*1000)/1000;
        EV<<"TALKSPURT: "<<talkID<<" Durata: "<<durTalk<< "/"<< durTalk2<< " secondi\n\n";     //FIXME Translate!!!
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

void VoIPSender::sendVoIPPacket()
{
    VoipPacket* packet = new VoipPacket("VoIP");
    packet->setTalkID(talkID-1);
    packet->setNframes(nframes);
    packet->setFrameID(frameID);
    packet->setVoipTimestamp(simTime());
    packet->setVoiceDuration(samplingTime);
    packet->setByteLength(talkFrameSize);
    EV<<"TALKSPURT "<<talkID-1<<" Invio frame "<<frameID<<"\n";     //FIXME Translate!!!

    socket.sendTo(packet,destAddress, destPort);
    --nframes_tmp;
    ++frameID;

    if(nframes_tmp > 0 ) scheduleAt(simTime()+samplingTime,selfSender);
}

