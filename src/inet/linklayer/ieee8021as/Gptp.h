//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#ifndef __INET_GPTP_H
#define __INET_GPTP_H

#include "inet/clock/contract/ClockTime.h"
#include "inet/clock/common/ClockTime.h"
#include "inet/clock/model/SettableClock.h"
#include "inet/common/INETDefs.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/clock/ClockUserModuleBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"

namespace inet {

class INET_API Gptp : public ClockUserModuleBase, public cListener
{
    //parameters:
    ModuleRefByPar<IInterfaceTable> interfaceTable;

    GptpNodeType gptpNodeType;
    int domainNumber = -1;
    int slavePortId = -1; // interface ID of slave port
    std::set<int> masterPortIds; // interface IDs of master ports
    clocktime_t correctionField;
    int64_t clockIdentity = 0;

    double gmRateRatio = 1.0;
    double receivedRateRatio = 1.0;

    clocktime_t originTimestamp; // last outgoing timestamp

    clocktime_t receivedTimeSync;

    clocktime_t syncInterval;
    clocktime_t pdelayInterval;

    uint16_t sequenceId = 0;
    /* Slave port - Variables is used for Peer Delay Measurement */
    uint16_t lastSentPdelayReqSequenceId = 0;
    clocktime_t peerDelay;
    clocktime_t peerRequestReceiptTimestamp;  // pdelayReqIngressTimestamp from peer (received in GptpPdelayResp)
    clocktime_t peerResponseOriginTimestamp; // pdelayRespEgressTimestamp from peer (received in GptpPdelayRespFollowUp)
    clocktime_t pdelayRespEventIngressTimestamp;  // receiving time of last GptpPdelayResp
    clocktime_t pdelayReqEventEgressTimestamp;   // sending time of last GptpPdelayReq
    clocktime_t pDelayReqProcessingTime;  // processing time between arrived PDelayReq and send of PDelayResp
    bool rcvdPdelayResp = false;

    clocktime_t sentTimeSyncSync;

    /* Slave port - Variables is used for Rate Ratio. All times are drifted based on constant drift */
    // clocktime_t sentTimeSync;
    clocktime_t newLocalTimeAtTimeSync;
    clocktime_t oldLocalTimeAtTimeSync;
    clocktime_t peerSentTimeSync;  // sending time of last received GptpSync
    clocktime_t oldPeerSentTimeSync = -1;  // sending time of previous received GptpSync
    clocktime_t syncIngressTimestamp;  // receiving time of last incoming GptpSync

    bool rcvdGptpSync = false;
    uint16_t lastReceivedGptpSyncSequenceId = 0xffff;

    // self timers:
    ClockEvent* selfMsgSync = nullptr;
    ClockEvent* selfMsgDelayReq = nullptr;
    ClockEvent* requestMsg = nullptr;

    // Statistics information: // TODO remove, and replace with emit() calls
    static simsignal_t localTimeSignal;
    static simsignal_t timeDifferenceSignal;
    static simsignal_t rateRatioSignal;
    static simsignal_t peerDelaySignal;
  public:
    static const MacAddress GPTP_MULTICAST_ADDRESS;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void handleSelfMessage(cMessage *msg);

  public:
    virtual ~Gptp();
    void sendPacketToNIC(Packet *packet, int portId);

    void sendSync();
    void sendFollowUp(int portId, const GptpSync *sync, clocktime_t preciseOriginTimestamp);
    void sendPdelayReq();
    void sendPdelayResp(GptpReqAnswerEvent *req);
    void sendPdelayRespFollowUp(int portId, const GptpPdelayResp* resp);

    void processSync(Packet *packet, const GptpSync* gptp);
    void processFollowUp(Packet *packet, const GptpFollowUp* gptp);
    void processPdelayReq(Packet *packet, const GptpPdelayReq* gptp);
    void processPdelayResp(Packet *packet, const GptpPdelayResp* gptp);
    void processPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp* gptp);

    clocktime_t getCalculatedDrift(IClock *clock, clocktime_t value) { return CLOCKTIME_ZERO; }
    void synchronize();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
};

}

#endif

