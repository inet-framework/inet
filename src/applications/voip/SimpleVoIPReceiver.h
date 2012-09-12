/*
 * VoIPReceiver.h
 *
 *  Created on: 01/feb/2011
 *      Author: Adriano
 */


#ifndef VOIPRECEIVER_H_
#define VOIPRECEIVER_H_

#include <string.h>
#include <list>

#include "INETDefs.h"

#include "IPvXAddressResolver.h"
#include "UDPSocket.h"

class SimpleVoIPPacket;

class SimpleVoIPReceiver : public cSimpleModule
{
    class TaggedSample : public cObject
    {
      public:
        double sample;
        unsigned int id;
        // the emitting cComponent (module)
        cComponent* module;
    };

    UDPSocket socket;

    ~SimpleVoIPReceiver();

    // FIXME: avoid _ characters
    int         emodel_Ie;
    int         emodel_Bpl;
    int         emodel_A;
    double      emodel_Ro;

    typedef std::list<SimpleVoIPPacket*> PacketsList;
    // FIXME: welcome to Microsoft naming conventions... mFooBar -> fooBar
    PacketsList  packetsList;
    PacketsList  playoutQueue;
    unsigned int currentTalkspurt;
    unsigned int bufferSpace;
    simtime_t    playoutDelay;

    simsignal_t packetLossRateSignal;
    simsignal_t packetDelaySignal;
    simsignal_t playoutDelaySignal;
    simsignal_t playoutLossRateSignal;
    simsignal_t mosSignal;
    simsignal_t taildropLossRateSignal;

    TaggedSample* taggedSample;

    virtual void finish();

  protected:
    virtual int numInitStages() const {return 4;}
    void initialize(int stage);
    void handleMessage(cMessage *msg);
    double eModel(double delay, double loss);
    void evaluateTalkspurt(bool finish);
};


#endif /* VOIPRECEIVER_H_ */
