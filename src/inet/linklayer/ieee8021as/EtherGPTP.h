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
#include "inet/linklayer/ethernet/EtherFrame_m.h"

#include "inet/linklayer/ethernet/EtherMACBase.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/base/MACBase.h"

using namespace omnetpp;

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

enum gPtpMessageType {
    SYNC    = 1010,
    FOLLOW_UP = 1011,
    PDELAY_REQ = 1014,
    PDELAY_RESP = 1012,
    PDELAY_RESP_FOLLOW_UP = 1013
};

class EtherGPTP : public cSimpleModule
{
    TableGPTP* tableGptp;
    Clock* clockGptp;
    int portType;
    int nodeType;
    int stepCounter;
    SimTime rateRatio;

    // errorTime is time difference between MAC transmition
    // or receiving time and etherGPTP time
    cMessage* requestMsg;

    SimTime receivedTimeAtHandleMessage;
    SimTime residenceTime;

    SimTime scheduleSync;
    SimTime schedulePdelay;
    SimTime schedulePdelayResp;

    SimTime syncInterval;
    SimTime pdelayInterval;

    cMessage* selfMsgSync;
    cMessage* selfMsgFollowUp;
    cMessage* selfMsgDelayReq;
    cMessage* selfMsgDelayResp;

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
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

  public:
    EtherGPTP();

    void masterPort(cMessage *msg);
    void sendSync(SimTime value);
    void sendFollowUp();
    void processPdelayReq(gPtp_PdelayReq* gptp);
    void sendPdelayResp();
    void sendPdelayRespFollowUp();

    void slavePort(cMessage *msg);
    void sendPdelayReq();
    void processSync(gPtp_Sync* gptp);
    void processFollowUp(gPtp_FollowUp* gptp);
    void processPdelayResp(gPtp_PdelayResp* gptp);
    void processPdelayRespFollowUp(gPtp_PdelayRespFollowUp* gptp);
};

#endif
