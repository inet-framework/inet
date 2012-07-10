/*
 * VoIPSender.h
 *
 *  Created on: 25/gen/2011
 *      Author: Adriano
 */



#ifndef VOIPSENDER_H_
#define VOIPSENDER_H_

#include <string.h>
#include <omnetpp.h>

#include "UDPSocket.h"
#include "IPvXAddressResolver.h"
#include "VoipPacket_m.h"

class VoIPSender : public cSimpleModule
{
    UDPSocket socket;
	//has the sender been initialized?
	bool initialized_;

	//source
	double 	  durTalk;
	double 	  durSil;
	double 	  scale_talk;
	double 	  shape_talk;
	double 	  scale_sil;
	double 	  shape_sil;
	bool      is_talk;
	cMessage* selfSource;
	//sender
	// FIXME questi non dovrebbero essere interi
	int    IDtalk;
	int    nframes;
	int    IDframe;
	int    nframes_tmp;
	int    size;
	double sampling_time;

	// ----------------------------

	cMessage *selfSender;

	simtime_t timestamp;
	int localPort;
	int destPort;
	IPvXAddress destAddress;


	void talkspurt(double dur);
	void selectPeriodTime();
	void sendVoIPPacket();

public:
	~VoIPSender();
	VoIPSender();



protected:

	virtual int numInitStages() const {return 4;}
	void initialize(int stage);
	void handleMessage(cMessage *msg);

};

#endif /* VOIPSENDER_H_ */

