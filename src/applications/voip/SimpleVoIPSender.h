/*
 * VoIPSender.h
 *
 *  Created on: 25/gen/2011
 *      Author: Adriano
 */



#ifndef VOIPSENDER_H_
#define VOIPSENDER_H_

#include <string.h>

#include "INETDefs.h"

#include "UDPSocket.h"
#include "IPvXAddressResolver.h"

class SimpleVoIPSender : public cSimpleModule
{
    UDPSocket socket;

    //source
    simtime_t talkspurtDuration;
    simtime_t silenceDuration;
    bool      isTalk;

    // FIXME: be more specific with the name of this self message
    cMessage* selfSource;
    //sender
    // FIXME questi non dovrebbero essere interi     //FIXME Translate!!!
    int    talkspurtID;
    int    talkspurtNumPackets;
    int    packetID;
    int    talkPacketSize;
    simtime_t packetizationInterval;

    // ----------------------------
    // FIXME: it is unclear what is this self message used for
    cMessage *selfSender;

    simtime_t timestamp;
    int localPort;
    int destPort;
    IPvXAddress destAddress;
    simtime_t stopTime;

    void talkspurt(simtime_t dur);
    void selectPeriodTime();
    void sendVoIPPacket();

  public:
    ~SimpleVoIPSender();
    SimpleVoIPSender();

  protected:
    virtual int numInitStages() const {return 4;}
    void initialize(int stage);
    void handleMessage(cMessage *msg);
};

#endif /* VOIPSENDER_H_ */

