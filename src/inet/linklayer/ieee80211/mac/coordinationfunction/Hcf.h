//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_HCF_H
#define __INET_HCF_H

#include <optional>

#include "inet/linklayer/ieee80211/mac/channelaccess/Edca.h"
#include "inet/linklayer/ieee80211/mac/channelaccess/Hcca.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IAckHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/BlockAckVariant.h"
#include "inet/linklayer/ieee80211/mac/contract/IBlockAckAgreementHandlerCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#include "inet/linklayer/ieee80211/mac/contract/ICtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQosAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQosMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/ITx.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceHandler.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.h"
#include "inet/linklayer/ieee80211/mac/originator/QosAckHandler.h"
#include "inet/linklayer/ieee80211/mac/originator/QosRecoveryProcedure.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/linklayer/ieee80211/mac/protectionmechanism/SingleProtectionMechanism.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/linklayer/ieee80211/mac/recipient/CtsProcedure.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/TransmissionTimingSnapshot.h"
#include "inet/linklayer/ieee80211/mac/contract/AmpduReception.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Mac;
class Ieee80211Mib;

/**
 * Implements IEEE 802.11 Hybrid Coordination Function.
 */
class INET_API Hcf : public ICoordinationFunction, public IFrameSequenceHandler::ICallback, public IChannelAccess::ICallback, public ITx::IAggregateCallback, public IProcedureCallback, public IBlockAckAgreementHandlerCallback, public ModeSetListener
{
  private:
    BlockAckVariant blockAckVariant = BlockAckVariant::BASIC;
    bool validateBlockAckRequest(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& bar) const;
    struct InterframeHistory {
        physicallayer::TransmissionTimingSnapshot timing;
        bool mpdu = false;
        bool ack = false;
        bool blockAck = false;
        bool ht = false;
        bool noSifsResponse = false;
        MacAddress receiver;
        std::optional<simtime_t> extension;
        std::shared_ptr<const AmpduTopology> aggregate;
    };
    struct PendingReception {
        bool completed = false;
        bool sifs = false;
        bool rifs = false;
        bool overlapped = false;
        bool precedingIndividual = false;
        bool precedingAggregate = false;
        std::shared_ptr<const AmpduTopology> previousAggregate;
        MacAddress precedingReceiver;
        physicallayer::TransmissionTimingSnapshot timing;
        bool ht = false;
        std::optional<simtime_t> extension;
    };
    opp_component_ptr<physicallayer::IRadio> interframeRadio;
    InterframeHistory interframeHistory;
    std::map<int, PendingReception> pendingReceptions;
    bool completedTransmissionWasAck = false;
    bool completedTransmissionWasBlockAck = false;
    std::shared_ptr<const AmpduTopology> transmittedBlockAckAggregate;
    void clearInterframeHistory();
    bool receiveInterframe(Packet *packet, const Ptr<const Ieee80211MacHeader>& header,
            BlockAckReceiveContext *context = nullptr);

  public:
    Packet *prepareAggregateTransmission() override;
    void aggregateTransmissionCancelled() override;
    void frameTransmissionStarted(Packet *packet) override;
    static simsignal_t edcaCollisionDetectedSignal;
    static simsignal_t blockAckAgreementAddedSignal;
    static simsignal_t blockAckAgreementDeletedSignal;

  protected:
    Ieee80211Mac *mac = nullptr;
    Ieee80211Mib *mib = nullptr;
    IRateControl *dataAndMgmtRateControl = nullptr;

    cMessage *startRxTimer = nullptr;
    cMessage *inactivityTimer = nullptr;
    cMessage *dataLifetimeTimer = nullptr;
    cMessage *addbaResponseTimer = nullptr;
    struct OwnedAction {
        MacAddress peer;
        Tid tid = -1;
        bool originator = false;
        enum Kind { REQUEST, RESPONSE, DELBA } kind = REQUEST;
        bool attempted = false;
        bool rtsRecoveryOwned = false;
    };
    std::map<int64_t, OwnedAction> ownedActions;
    std::set<int64_t> obsoleteActions;
    bool handlingAgreementAction = false;
    bool finishingSequence = false;
    bool updatingProcedures = false;

    // Transmission and Reception
    IRx *rx = nullptr;
    ITx *tx = nullptr;

    IQosRateSelection *rateSelection = nullptr;

    // Channel Access Methods
    Edca *edca = nullptr;
    Hcca *hcca = nullptr;

    // MAC Data Service
    IOriginatorMacDataService *originatorDataService = nullptr;
    IRecipientQosMacDataService *recipientDataService = nullptr;

    // MAC Procedures
    IRecipientAckProcedure *recipientAckProcedure = nullptr;
    IOriginatorQoSAckPolicy *originatorAckPolicy = nullptr;
    IRecipientQosAckPolicy *recipientAckPolicy = nullptr;
    IRtsProcedure *rtsProcedure = nullptr;
    IRtsPolicy *rtsPolicy = nullptr;
    ICtsProcedure *ctsProcedure = nullptr;
    ICtsPolicy *ctsPolicy = nullptr;
    IOriginatorBlockAckProcedure *originatorBlockAckProcedure = nullptr;
    IRecipientBlockAckProcedure *recipientBlockAckProcedure = nullptr;

    // Block Ack Agreement Handlers
    IOriginatorBlockAckAgreementHandler *originatorBlockAckAgreementHandler = nullptr;
    IOriginatorBlockAckAgreementPolicy *originatorBlockAckAgreementPolicy = nullptr;
    IRecipientBlockAckAgreementHandler *recipientBlockAckAgreementHandler = nullptr;
    IRecipientBlockAckAgreementPolicy *recipientBlockAckAgreementPolicy = nullptr;

    // Tx Opportunity
    TxopProcedure *hccaTxop = nullptr;

    // Queues
    InProgressFrames *hccaInProgressFrame = nullptr;

    // Frame sequence handler
    IFrameSequenceHandler *frameSequenceHandler = nullptr;

    // Protection mechanisms
    SingleProtectionMechanism *singleProtectionMechanism = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void forEachChild(cVisitor *v) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, bool value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;
    virtual void refreshDisplay() const override;

    void startFrameSequence(AccessCategory ac);
    void handleInternalCollision(std::vector<Edcaf *> internallyCollidedEdcafs);

    // Tests can consume received frames here without an upper protocol stack.
    virtual void sendUp(const std::vector<Packet *>& completeFrames);
    FrameSequenceContext *buildContext(AccessCategory ac);
    static simtime_t encodeDuration(simtime_t duration, bool holder);
    simtime_t computeImmediateResponseDuration(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header);
    DataPreparationResult prepareDataTransmission(FrameSequenceContext *context);
    bool prepareAggregateData(FrameSequenceContext *context, const PreparedDataTransmission& selected);
    void processAggregateOutcome(const Ptr<const Ieee80211CompressedBlockAck>& response);
    bool validatePreparedData(const PreparedDataTransmission& prepared, simtime_t start) const;
    simtime_t computePreparedDataCost(const PreparedDataTransmission& prepared, bool protect) const;
    bool canAdmitPreparedData(const FrameSequenceContext *context, const PreparedDataTransmission& prepared,
            simtime_t ifs, bool protect, simtime_t elapsedIfs = 0) const;
    bool isInitialDataException(const FrameSequenceContext *context, const PreparedDataTransmission& prepared) const;
    simtime_t forecastSingleProtectionSuccessor(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header,
            TxopProcedure *txop, InProgressFrames *frames, simtime_t completionTime);
    static simtime_t computeMultipleProtectionDuration(simtime_t limit, simtime_t remaining,
            simtime_t reservation, simtime_t ppduDuration, simtime_t exchangeDuration, bool reservationEstablished);
    simtime_t estimateZeroLimitExchange(Packet *packet, const Ptr<const Ieee80211MacHeader>& header,
            TxopProcedure *txop, simtime_t ppduDuration);
    virtual bool hasFrameToTransmit();
    virtual bool hasFrameToTransmit(AccessCategory ac);
    bool hasEligibleFrameToTransmit(AccessCategory ac) const;
    virtual bool isReceptionInProgress();
    void scheduleDataLifetimeTimer();
    void expireDataFrames(bool preserveSequence = true);
    std::set<const Packet *> getReferencedDataFrames() const;
    bool isFrameEligible(Packet *packet, bool numbered) const;
    bool isAMsduPermitted(Packet *packet) const;
    bool hasQueuedData(MacAddress peer, Tid tid) const;
    bool hasNumberedData(MacAddress peer, Tid tid) const;
    bool hasObsoleteActionInFlight(MacAddress peer, Tid tid, bool originator) const;
    void updateAgreementProcedures();
    void advanceOriginatorWindows();
    void completeTransfers();
    void updateContention();
    void scheduleAddbaResponseTimer();
    void actionTerminated(Packet *packet, bool acknowledged);
    void invalidateActions(MacAddress peer, Tid tid, bool originator);
    void retireAgreement(MacAddress peer, Tid tid, bool originator, bool associationReset = false);
    void beginTeardown(MacAddress peer, Tid tid, bool originator, int reasonCode);
    void discardAgreementData(MacAddress peer, Tid tid, bool all, int64_t highestTransmittedDataIndex = -1);
    void firstDataDiscarded(const Ptr<const Ieee80211DataHeader>& header);
    bool dispatchAgreementAction(const Ptr<const Ieee80211MgmtHeader>& header);
    bool isProfileAllowed(MacAddress peer, bool originator) const;

    // Recipient
    virtual void recipientProcessReceivedFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, bool interframe = false,
            const BlockAckReceiveContext *context = nullptr);
    void sendImplicitBlockAck(Packet *envelope, const Ptr<const Ieee80211DataHeader>& header,
            RecipientBlockAckAgreement *agreement);
    virtual void recipientProcessReceivedControlFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);
    virtual void recipientProcessReceivedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& header);
    virtual void recipientProcessTransmittedControlResponseFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);

    // Originator
    virtual void originatorProcessTransmittedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& mgmtHeader, AccessCategory ac);
    virtual void originatorProcessTransmittedControlFrame(const Ptr<const Ieee80211MacHeader>& controlHeader, AccessCategory ac);
    virtual void originatorProcessTransmittedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, AccessCategory ac);
    virtual void originatorProcessReceivedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& header, const Ptr<const Ieee80211MacHeader>& lastTransmittedHeader, AccessCategory ac);
    virtual void originatorProcessReceivedControlFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, Packet *lastTransmittedPacket, const Ptr<const Ieee80211MacHeader>& lastTransmittedHeader, AccessCategory ac);
    virtual void originatorProcessReceivedDataFrame(const Ptr<const Ieee80211DataHeader>& header, const Ptr<const Ieee80211MacHeader>& lastTransmittedHeader, AccessCategory ac);

    virtual void setFrameMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, const physicallayer::IIeee80211Mode *mode) const;
    virtual bool isSentByUs(const Ptr<const Ieee80211MacHeader>& header) const;
    virtual bool isForUs(const Ptr<const Ieee80211MacHeader>& header) const;

  protected:
    // IFrameSequenceHandler::ICallback
    virtual void originatorProcessRtsProtectionFailed(Packet *packet) override;
    virtual void originatorProcessTransmittedFrame(Packet *packet) override;
    virtual void originatorProcessReceivedFrame(Packet *packet, Packet *lastTransmittedPacket) override;
    virtual void originatorProcessFailedFrame(Packet *packet) override;
    virtual void frameSequenceStarted() override;
    virtual void frameSequenceFinished() override;
    virtual bool transmitFrame(Packet *packet, simtime_t ifs) override;
    virtual void scheduleStartRxTimer(simtime_t timeout) override;

    // IChannelAccess::ICallback
    virtual void channelGranted(IChannelAccess *channelAccess) override;

    // ITx::ICallback
    virtual void transmissionComplete(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) override;

    // IProcedureCallback
    virtual void transmitControlResponseFrame(Packet *responsePacket, const Ptr<const Ieee80211MacHeader>& responseHeader, Packet *receivedPacket, const Ptr<const Ieee80211MacHeader>& receivedHeader) override;
    virtual void processMgmtFrame(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader) override;

    // IProcedureCallback
    virtual void scheduleInactivityTimer() override;

    std::string getFrameSequenceInfo() const;

  public:
    virtual ~Hcf();
    void resetPeer(MacAddress peer);

    // ICoordinationFunction
    virtual void processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;
    virtual void processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) override;
    virtual void processLowerAggregate(Packet *packet, AmpduReception reception);
    virtual void corruptedFrameReceived() override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
