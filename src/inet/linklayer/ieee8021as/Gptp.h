//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#ifndef __INET_GPTP_H
#define __INET_GPTP_H

#include <vector>

#include "inet/clock/common/ClockTime.h"
#include "inet/clock/contract/ClockTime.h"
#include "inet/clock/model/SettableClock.h"
#include "inet/common/INETDefs.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/clock/ClockUserModuleBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class INET_API Gptp : public ClockUserModuleBase, public cListener
{
    // parameters:
    ModuleRefByPar<IInterfaceTable> interfaceTable;

    // Configuration
    GptpNodeType gptpNodeType;
    int domainNumber = -1;
    int slavePortId = -1;        // interface ID of slave port
    std::set<int> masterPortIds; // interface IDs of master ports
    uint64_t clockIdentity = 0;
    clocktime_t syncInterval;
    clocktime_t pdelayInterval;
    clocktime_t pDelayReqProcessingTime; // processing time between arrived
                                         // PDelayReq and send of PDelayResp

    // Rate Ratios
    double gmRateRatio = 1.0;
    double receivedRateRatio = 1.0;
    double neighborRateRatio = 1.0; // the rate ratio to the neighbor

    uint16_t sequenceId = 0;

    // == Propagation Delay Measurement Procedure ==
    // Timestamps corresponding to the PDelayRequest and PDelayResponse mechanism
    clocktime_t pDelayReqEgressTimestamp = -1;  // egress time of pdelay_req at initiator (this node)
    clocktime_t pDelayReqIngressTimestamp = -1; // ingress time of pdelay_req at responder
    clocktime_t pDelayRespEgressTimestamp = -1; // egress time of pdelay_resp at responder (received in PDelayRespFollowUp)
    clocktime_t pDelayRespEgressTimestampLast = -1; // egress time of previous pdelay_resp at responder (received in PDelayRespFollowUp)
    clocktime_t pDelayRespIngressTimestamp = -1;     // ingress time of pdelay_resp at initiator (this node)
    clocktime_t pDelayRespIngressTimestampLast = -1; // ingress time of previous pdelay_resp at initiator (this node)

    bool rcvdPdelayResp = false;
    uint16_t lastSentPdelayReqSequenceId = 0;

    clocktime_t meanLinkDelay = CLOCKTIME_ZERO; // mean propagation delay between this node and the responder

    // == Sync Procedure ==

    // Holds the (received) correction field value, used for the next FollowUp message
    // Unsure why this is also configurable with a parameter (TODO: Check)
    clocktime_t correctionField = CLOCKTIME_ZERO;

    clocktime_t preciseOriginTimestamp; // timestamp when the last sync message was generated at the GM

    clocktime_t syncEgressTimestampMaster = -1;     // egress time of Sync at master
    clocktime_t syncEgressTimestampMasterLast = -1; // egress time of previous Sync at master
    clocktime_t syncIngressTimestamp = -1;          // ingress time of Sync at slave (this node)
    clocktime_t syncIngressTimestampLast = -1;      // ingress time of previous Sync at slave (this node)

    bool rcvdGptpSync = false;
    uint16_t lastReceivedGptpSyncSequenceId = 0xffff;

    clocktime_t newLocalTimeAtTimeSync;
    clocktime_t timeDiffAtTimeSync; // new local time - old local time

    // self timers:
    ClockEvent *selfMsgSync = nullptr;
    ClockEvent *selfMsgDelayReq = nullptr;
    ClockEvent *requestMsg = nullptr;

    // Statistics information: // TODO remove, and replace with emit() calls
    static simsignal_t localTimeSignal;
    static simsignal_t timeDifferenceSignal;
    static simsignal_t rateRatioSignal;
    static simsignal_t peerDelaySignal;

  public:
    static const MacAddress GPTP_MULTICAST_ADDRESS;

  protected:
    virtual int numInitStages() const

        override
    {
        return NUM_INIT_STAGES;
    }

    virtual void initialize(int stage)

        override;

    virtual void handleMessage(cMessage *msg)

        override;

    virtual void handleSelfMessage(cMessage *msg);

    void handleDelayOrSendFollowUp(const GptpBase *gptp, omnetpp::cComponent *source);

    const GptpBase *extractGptpHeader(Packet *packet);

  public:
    virtual ~

        Gptp();

  protected:
    void sendPacketToNIC(Packet *packet, int portId);

    void sendSync();

    void sendFollowUp(int portId, const GptpSync *sync, const clocktime_t &syncEgressTimestampOwn);

    void sendPdelayReq();

    void sendPdelayResp(GptpReqAnswerEvent *req);

    void sendPdelayRespFollowUp(int portId, const GptpPdelayResp *resp);

    void processSync(Packet *packet, const GptpSync *gptp);

    void processFollowUp(Packet *packet, const GptpFollowUp *gptp);

    void processPdelayReq(Packet *packet, const GptpPdelayReq *gptp);

    void processPdelayResp(Packet *packet, const GptpPdelayResp *gptp);

    void processPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp *gptp);

    clocktime_t getCalculatedDrift(IClock *clock, clocktime_t value) { return CLOCKTIME_ZERO; }

    void synchronize();

    inline void adjustLocalTimestamp(clocktime_t &time) { time += timeDiffAtTimeSync; }

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details)

        override;
};

} // namespace inet

#endif
