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
}

VoIPSender::~VoIPSender()
{
	if(selfSender->isScheduled())
		cancelEvent(selfSender);

	delete selfSender;
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
	scale_talk    = par("scale_talk");
	shape_talk    = par("shape_talk");
	scale_sil     = par("scale_sil");
	shape_sil     = par("shape_sil");
	is_talk       = par("is_talk");
	IDtalk        = 0;
	nframes       = 0;
	nframes_tmp   = 0;
	IDframe       = 0;
	timestamp     = 0;
	size          = par("PacketSize");
	sampling_time = par("sampling_time");
	selfSender    = new cMessage("selfSender");
	localPort     = par("localPort");
	destPort      = par("destPort");
	destAddress   = IPvXAddressResolver().resolve(par("destAddress").stringValue());


    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

	EV << "VoIPSender::initialize - binding to port: local:" << localPort << " , dest:"<< destPort << endl;


	// calculating traffic starting time
	// TODO correct this conversion
	double startTime = round((double)par("startTime")*1000)/1000;
	double offset = startTime+simTime().dbl();

	scheduleAt(offset,selfSource);
	EV << "\t starting traffic in " << startTime << " ms" << endl;

	initialized_ = true;
}

void VoIPSender::handleMessage(cMessage *msg)
{
	if(msg->isSelfMessage())
	{
		if(!strcmp(msg->getName(),"selfSender"))
			sendVoIPPacket();
		else
			selectPeriodTime();
	}
}

void VoIPSender::talkspurt(double dur)
{
	IDtalk++;
	nframes=(ceil(dur/sampling_time));
	EV<<"TALKSPURT "<<IDtalk-1<<" Verranno inviati "<<nframes<<" frames\n\n";
	std::cout <<"TALKSPURT "<<IDtalk-1<<" Verranno inviati "<<nframes<<" frames\n\n";

	IDframe = 0;
	nframes_tmp=nframes;
	scheduleAt(simTime(), selfSender);
}

void VoIPSender::selectPeriodTime()
{
	if(!is_talk)
	{
		durSil=weibull(scale_sil, shape_sil);
		double durSil2 = round(durSil*1000)/1000;
		EV<<"PERIODO SILENZIO: "<<"Durata: "<<durSil<<"/" << durSil2<<" secondi\n\n";
		durSil = durSil2;
		scheduleAt(simTime()+durSil,selfSource);
		is_talk = true;
	}

	else
	{
		durTalk=weibull(scale_talk, shape_talk);
		double durTalk2 = round(durTalk*1000)/1000;
		EV<<"TALKSPURT: "<<IDtalk<<" Durata: "<<durTalk<< "/"<< durTalk2<< " secondi\n\n";
		durTalk = durTalk2;
		talkspurt(durTalk);
		scheduleAt(simTime()+durTalk,selfSource);
		is_talk = false;
	}
}

void VoIPSender::sendVoIPPacket()
{
	VoipPacket* packet = new VoipPacket("VoIP");
	packet->setIDtalk(IDtalk-1);
	packet->setNframes(nframes);
	packet->setIDframe(IDframe);
	packet->setTimestamp(simTime());
	//packet->setSize(size);
	packet->setByteLength(size);
	EV<<"TALKSPURT "<<IDtalk-1<<" Invio frame "<<IDframe<<"\n";
	std::cout << simTime() <<") TALKSPURT "<<IDtalk-1<<" Invio frame "<<IDframe<<"\n";

	socket.sendTo(packet,destAddress, destPort);
	--nframes_tmp;
	++IDframe;

	if(nframes_tmp > 0 ) scheduleAt(simTime()+sampling_time,selfSender);
}
