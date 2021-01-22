//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#ifndef __IEEE8021AS_ETHERGPTP_H_
#define __IEEE8021AS_ETHERGPTP_H_

#include "gPtp.h"
#include "gPtpPacket_m.h"
#include "tableGPTP.h"
#include "Clock.h"
#include <omnetpp.h>
#include <string>

//#include "inet/linklayer/ethernet/common/EthernetMacBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
//#include "inet/linklayer/base/MacBase.h"

namespace inet {

enum gPtpNodeType {
    MASTER_NODE = 11,
    BRIDGE_NODE = 12,
    SLAVE_NODE  = 13
};

enum gPtpPortType {
    MASTER_PORT  = 2,
    SLAVE_PORT   = 1,
    PASSIVE_PORT = 0
};

class EtherGPTP : public cSimpleModule
{
    opp_component_ptr<TableGPTP> tableGptp;
    opp_component_ptr<Clock> clockGptp;
    int portType;
    int nodeType;
    int stepCounter;
    SimTime rateRatio;

    // errorTime is time difference between MAC transmition
    // or receiving time and etherGPTP time
    cMessage* requestMsg = nullptr;

    SimTime receivedTimeAtHandleMessage;
    SimTime residenceTime;

    SimTime scheduleSync;
    SimTime schedulePdelay;
    SimTime schedulePdelayResp;

    SimTime syncInterval;
    SimTime pdelayInterval;

    cMessage* selfMsgSync = nullptr;
    cMessage* selfMsgFollowUp = nullptr;
    cMessage* selfMsgDelayReq = nullptr;
    cMessage* selfMsgDelayResp = nullptr;

    /* Slave port - Variables is used for Peer Delay Measurement */
    SimTime peerDelay;
    SimTime receivedTimeResponder;
    SimTime receivedTimeRequester;
    SimTime transmittedTimeResponder;
    SimTime transmittedTimeRequester;
    double PDelayRespInterval;
    double FollowUpInterval;

    SimTime sentTimeSyncSync;

    /* Slave port - Variables is used for Rate Ratio. All times are drifted based on constant drift */
    SimTime sentTimeSync;
    SimTime receivedTimeSyncAfterSync;
    SimTime receivedTimeSyncBeforeSync;
    SimTime neighborDrift;

//    SimTime sentTimeAfterSync;
//    SimTime sentTimeBeforeSync;
//    SimTime sentTimeFollowUp;
//    SimTime receivedTimeFollowUp;
//    SimTime sentTimeFollowUp1;
//    SimTime sentTimeFollowUp2;
//    SimTime receivedTimeFollowUp1;
//    SimTime receivedTimeFollowUp2;


    /* Statistics information */
    cOutVector vLocalTime;
    cOutVector vMasterTime;
    cOutVector vTimeDifference;
    cOutVector vTimeDifferenceGMafterSync;
    cOutVector vTimeDifferenceGMbeforeSync;
    cOutVector vRateRatio;
    cOutVector vPeerDelay;

protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

  public:
    EtherGPTP();
    virtual ~EtherGPTP();

    void masterPort(cMessage *msg);
    void slavePort(cMessage *msg);

    void sendSync(SimTime value);
    void sendFollowUp();
    void sendPdelayReq();
    void sendPdelayResp();
    void sendPdelayRespFollowUp();

    void processSync(const GPtpSync* gptp);
    void processFollowUp(const GPtpFollowUp* gptp);
    void processPdelayReq(const GPtpPdelayReq* gptp);
    void processPdelayResp(const GPtpPdelayResp* gptp);
    void processPdelayRespFollowUp(const GPtpPdelayRespFollowUp* gptp);

    void handleTableGptpCall(cMessage *msg);
};

}

#endif
