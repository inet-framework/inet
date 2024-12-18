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
#include "inet/clock/servo/ServoClockBase.h"
#include "inet/common/INETDefs.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/clock/ClockUserModuleBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class INET_API Gptp : public ClockUserModuleBase, public cListener
{
  protected:
    // parameters:
    ModuleRefByPar<IInterfaceTable> interfaceTable;

    // Configuration
    GptpNodeType gptpNodeType;
    int domainNumber = -1;
    int slavePortId = -1;        // interface ID of slave port
    std::set<int> masterPortIds; // interface IDs of master ports
    std::set<int> bmcaPortIds;   // interface IDs of bmca ports
    std::set<int> passivePortIds; // interface IDs of passive ports (only relevant for BMCA)
    uint64_t clockIdentity = 0;
    clocktime_t syncInterval;
    clocktime_t pdelayInterval;
    clocktime_t announceInterval;
    clocktime_t pDelayReqProcessingTime; // processing time between arrived
                                         // PDelayReq and send of PDelayResp
    bool useNrr = false;                 // use neighbor rate ratio
    GmRateRatioCalculationMethod gmRateRatioCalculationMethod = GmRateRatioCalculationMethod::NONE;

    // Rate Ratios
    double gmRateRatio = 1.0;
    double receivedRateRatio = 1.0;
    double neighborRateRatio = 1.0;

    uint16_t sequenceId = 0;
    // == Propagation Delay Measurement Procedure ==
    // Timestamps corresponding to the PDelayRequest and PDelayResponse mechanism
    clocktime_t pDelayReqEgressTimestamp = -1;  // egress time of pdelay_req at initiator (this node)
    clocktime_t pDelayReqIngressTimestamp = -1; // ingress time of pdelay_req at responder
    clocktime_t pDelayRespEgressTimestamp =
        -1; // egress time of pdelay_resp at responder (received in PDelayRespFollowUp)
    clocktime_t pDelayRespEgressTimestampSetStart =
        -1; // egress time of previous pdelay_resp at responder (received in PDelayRespFollowUp)
    clocktime_t pDelayRespIngressTimestamp = -1; // ingress time of pdelay_resp at initiator (this node)
    clocktime_t pDelayRespIngressTimestampSetStart =
        -1;                           // ingress time of previous pdelay_resp at initiator (this node)
    int nrrCalculationSetMaximum = 1; // TODO: Make this a settable parameter
    int nrrCalculationSetCurrent = 0;

    bool rcvdPdelayResp = false;
    uint16_t lastSentPdelayReqSequenceId = 0;

    clocktime_t meanLinkDelay = CLOCKTIME_ZERO; // mean propagation delay between this node and the responder

    // == Sync Procedure ==

    // Holds the (received) correction field value, used for the next FollowUp message
    // Unsure why this is also configurable with a parameter (TODO: Check)
    clocktime_t correctionField = CLOCKTIME_ZERO;

    clocktime_t preciseOriginTimestamp = -1;     // timestamp when the last sync message was generated at the GM
    clocktime_t preciseOriginTimestampLast = -1; // timestamp when the last sync message was generated at the GM

    clocktime_t peerSentTimeSync = -1;     // egress time of Sync at master (this node)
    clocktime_t peerSentTimeSyncLast = -1; // egress time of previous Sync at master (this node)

    clocktime_t syncIngressTimestamp = -1;     // ingress time of Sync at slave (this node)
    clocktime_t syncIngressTimestampLast = -1; // ingress time of previous Sync at slave (this node)

    bool rcvdGptpSync = false;
    uint16_t lastReceivedGptpSyncSequenceId = 0xffff;

    clocktime_t newLocalTimeAtTimeSync;

    std::map<int, clocktime_t> gptpSyncTime; // store each gptp sync time

    // BMCA
    BmcaPriorityVector localPriorityVector;
    GptpAnnounce *bestAnnounce = nullptr;
    std::map<int, ClockEvent *> announceTimeouts;
    std::map<int, GptpAnnounce *> receivedAnnounces;

    // self timers:
    ClockEvent *selfMsgSync = nullptr;
    ClockEvent *selfMsgDelayReq = nullptr;
    ClockEvent *selfMsgAnnounce = nullptr;

    std::set<GptpReqAnswerEvent *> reqAnswerEvents;

    // Statistics information: // TODO remove, and replace with emit() calls
    static simsignal_t localTimeSignal;
    static simsignal_t timeDifferenceSignal;
    static simsignal_t gmRateRatioSignal;
    static simsignal_t receivedRateRatioSignal;
    static simsignal_t neighborRateRatioSignal;
    static simsignal_t peerDelaySignal;
    static simsignal_t residenceTimeSignal;
    static simsignal_t correctionFieldIngressSignal;
    static simsignal_t correctionFieldEgressSignal;

    // Packet receive signals:
    std::map<uint16_t, clocktime_t> ingressTimeMap; // <sequenceId,ingressTime

  public:
    static const MacAddress GPTP_MULTICAST_ADDRESS;
    static simsignal_t gptpSyncSuccessfulSignal;

    uint8_t getDomainNumber() const { return this->domainNumber; };

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void initialize(int stage) override;

    virtual void initBmca();

    virtual void handleMessage(cMessage *msg) override;

    virtual void handleSelfMessage(cMessage *msg);

    virtual void handleClockJump(ServoClockBase::ClockJumpDetails *clockJumpDetails);

    void handleTransmissionStartedSignal(const GptpBase *gptp, omnetpp::cComponent *source);

    const GptpBase *extractGptpHeader(Packet *packet);

  public:
    virtual ~Gptp();

  protected:
    void sendPacketToNIC(Packet *packet, int portId);

    virtual void sendAnnounce();

    virtual void sendSync();

    virtual void sendFollowUp(int portId, const GptpSync *sync, const clocktime_t &syncEgressTimestampOwn);

    void sendPdelayReq();

    void sendPdelayResp(GptpReqAnswerEvent *req);

    void sendPdelayRespFollowUp(int portId, const GptpPdelayResp *resp);

    void processAnnounce(Packet *pPacket, const GptpAnnounce *pAnnounce);

    virtual void processSync(Packet *packet, const GptpSync *gptp);

    virtual void processFollowUp(Packet *packet, const GptpFollowUp *gptp);

    void processPdelayReq(Packet *packet, const GptpPdelayReq *gptp);

    void processPdelayResp(Packet *packet, const GptpPdelayResp *gptp);

    void processPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp *gptp);

    virtual void synchronize();

    inline void adjustLocalTimestamp(clocktime_t &timestamp, clocktime_t difference)
    {
        if (timestamp != -1) {
            timestamp += difference;
        }
        else {
            EV_INFO << "Timestamp is -1, cannot adjust it." << endl;
        }
    }

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
    void calculateGmRatio();
    void executeBmca();
    void initPorts();
    void scheduleMessageOnTopologyChange();
    void handleAnnounceTimeout(cMessage *pMessage);
    bool isGM() const { return gptpNodeType == MASTER_NODE || (gptpNodeType == BMCA_NODE && slavePortId == -1); };
};

} // namespace inet

#endif
