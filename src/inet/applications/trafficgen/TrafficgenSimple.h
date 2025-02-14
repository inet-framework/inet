/*
 * Trafficgen.h
 *
 *  Created on: 24.08.2015
 *      Author: weinrank
 */

#ifndef __INET_TRAFFICGENSIMPLE_H
#define __INET_TRAFFICGENSIMPLE_H

#include "inet/common/INETDefs.h"
#include "inet/applications/base/ApplicationBase.h"
#include "TrafficgenMessage_m.h"

namespace inet {

#define EVD EV << "[" << __FILE__ << "][" << __FUNCTION__ << "] "

enum IndicationCode {
    TRAFFICGEN_TIMER_START_SENDING,
    TRAFFICGEN_TIMER_STOP_SENDING,
    TRAFFICGEN_TIMER_SEND_PACKET
};

class INET_API TrafficgenSimple : public cSimpleModule {
    protected:
        int id;
        std::string name;
        int priority;
        int packetCount;
        bool active;
        bool reliable;
        bool ordered;

        simtime_t startTime;
        simtime_t stopTime;

        // timer
        cMessage *timerSendPacket;
        cMessage *timerStartSending;
        cMessage *timerStopSending;

        // stats
        simtime_t statRuntime;
        int sentPktCount;
        static simsignal_t sentPktSignal;

    public:
        TrafficgenSimple();
        virtual ~TrafficgenSimple();

    protected:
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void handleMessage(cMessage *msg) override;
        virtual void finish() override;

        void handleTrafficControlMessage(TrafficgenControl *msg);
        void sendInit();
        void sendData();
        void sendFinish();

        void setStatusString(const char *s);
};

}



#endif /* __INET_TRAFFICGENSIMPLE_H_ */
