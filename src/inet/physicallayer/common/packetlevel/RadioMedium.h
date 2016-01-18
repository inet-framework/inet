//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_RADIOMEDIUM_H
#define __INET_RADIOMEDIUM_H

#include <algorithm>
#include "inet/common/IntervalTree.h"
#include "inet/common/figures/TrailFigure.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/environment/contract/IMaterialRegistry.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/contract/packetlevel/IMediumLimitCache.h"
#include "inet/physicallayer/contract/packetlevel/INeighborCache.h"
#include "inet/physicallayer/contract/packetlevel/ICommunicationCache.h"
#include "inet/physicallayer/common/packetlevel/CommunicationLog.h"
#include "inet/physicallayer/common/packetlevel/MediumVisualizer.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {

namespace physicallayer {

/**
 * The default implementation of the radio medium interface.
 */
// TODO: add tests for various optimization configurations
class INET_API RadioMedium : public cSimpleModule, public cListener, public IRadioMedium
{
  protected:
    enum RangeFilterKind {
        RANGE_FILTER_ANYWHERE,
        RANGE_FILTER_INTERFERENCE_RANGE,
        RANGE_FILTER_COMMUNICATION_RANGE,
    };

  protected:
    /** @name Parameters and other models that control the behavior of the radio medium. */
    //@{
    /**
     * The propagation model is never nullptr.
     */
    const IPropagation *propagation;
    /**
     * The path loss model is never nullptr.
     */
    const IPathLoss *pathLoss;
    /**
     * The obstacle loss model or nullptr if unused.
     */
    const IObstacleLoss *obstacleLoss;
    /**
     * The analog model is never nullptr.
     */
    const IAnalogModel *analogModel;
    /**
     * The background noise model or nullptr if unused.
     */
    const IBackgroundNoise *backgroundNoise;
    /**
     * The physical environment model or nullptr if unused.
     */
    const IPhysicalEnvironment *physicalEnvironment;
    /**
     * The physical material of the medium is never nullptr.
     */
    const IMaterial *material;
    /**
     * The radio medium doesn't send radio frames to a radio if it's outside
     * the range provided by the selected range filter.
     */
    RangeFilterKind rangeFilter;
    /**
     * True means the radio medium doesn't send radio frames to a radio if
     * it's neither in receiver nor in transceiver mode.
     */
    bool radioModeFilter;
    /**
     * True means the radio medium doesn't send radio frames to a radio if
     * it listens on the medium in incompatible mode (e.g. different carrier
     * frequency and/or bandwidth, different modulation, etc.)
     */
    bool listeningFilter;
    /**
     * True means the radio medium doesn't send radio frames to a radio if
     * the mac address of the destination is different.
     */
    bool macAddressFilter;
    /**
     * Records all transmissions and receptions into a separate trace file.
     * The communication log file can be found at:
     * ${resultdir}/${configname}-${runnumber}.tlog
     */
    bool recordCommunicationLog;
    //@}

    /** @name Timer */
    //@{
    /**
     * The message that is used to purge the internal state of the medium.
     */
    cMessage *removeNonInterferingTransmissionsTimer;
    //@}

    /** @name State */
    //@{
    /**
     * The list of radios that can transmit and receive radio signals on the
     * radio medium. The radios follow each other in the order of their unique
     * id. Radios are only removed from the beginning. This list may contain
     * nullptr values.
     */
    std::vector<const IRadio *> radios;
    /**
     * The list of ongoing transmissions on the radio medium. The transmissions
     * follow each other in the order of their unique id. Transmissions are only
     * removed from the beginning. This list doesn't contain nullptr values.
     */
    std::vector<const ITransmission *> transmissions;
    //@}

    /** @name Cache */
    //@{
    /**
     * Caches various medium limits among all radios.
     */
    mutable IMediumLimitCache *mediumLimitCache;
    /**
     * Caches neighbors for all radios or nullptr if turned off.
     */
    mutable INeighborCache *neighborCache;
    /**
     * Caches intermediate results of the ongoing communication for all radios.
     */
    mutable ICommunicationCache *communicationCache;
    //@}

    /** @name Logging */
    //@{
    /**
     * The communication log output recorder.
     */
    CommunicationLog communicationLog;
    //@}

    /** @name Graphics */
    //@{
    /**
     * The visualizer for the communication on the medium.
     */
    MediumVisualizer *mediumVisualizer;
    //@}

    /** @name Statistics */
    //@{
    /**
     * Total number of transmissions.
     */
    mutable long transmissionCount;
    /**
     * Total number of radio frame sends.
     */
    mutable long radioFrameSendCount;
    /**
     * Total number of reception computations.
     */
    mutable long receptionComputationCount;
    /**
     * Total number of interference computations.
     */
    mutable long interferenceComputationCount;
    /**
     * Total number of data reception decision computations.
     */
    mutable long receptionDecisionComputationCount;
    /**
     * Total number of data reception result computations.
     */
    mutable long receptionResultComputationCount;
    /**
     * Total number of listening decision computations.
     */
    mutable long listeningDecisionComputationCount;
    /**
     * Total number of reception cache queries.
     */
    mutable long cacheReceptionGetCount;
    /**
     * Total number of reception cache hits.
     */
    mutable long cacheReceptionHitCount;
    /**
     * Total number of interference cache queries.
     */
    mutable long cacheInterferenceGetCount;
    /**
     * Total number of interference cache hits.
     */
    mutable long cacheInterferenceHitCount;
    /**
     * Total number of noise cache queries.
     */
    mutable long cacheNoiseGetCount;
    /**
     * Total number of noise cache hits.
     */
    mutable long cacheNoiseHitCount;
    /**
     * Total number of SNIR cache queries.
     */
    mutable long cacheSNIRGetCount;
    /**
     * Total number of SNIR cache hits.
     */
    mutable long cacheSNIRHitCount;
    /**
     * Total number of reception decision cache queries.
     */
    mutable long cacheDecisionGetCount;
    /**
     * Total number of reception decision cache hits.
     */
    mutable long cacheDecisionHitCount;
    /**
     * Total number of reception result cache queries.
     */
    mutable long cacheResultGetCount;
    /**
     * Total number of reception result cache hits.
     */
    mutable long cacheResultHitCount;
    //@}

  protected:
    /** @name Module */
    //@{
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *message) override;
    //@}

    /** @name Transmission */
    //@{
    /**
     * Adds a new transmission to the radio medium.
     */
    virtual void addTransmission(const IRadio *transmitter, const ITransmission *transmission);

    /**
     * Creates a new radio frame for the transmitter.
     */
    virtual IRadioFrame *createTransmitterRadioFrame(const IRadio *radio, cPacket *macFrame);

    /**
     * Creates a new radio frame for a receiver.
     */
    virtual IRadioFrame *createReceiverRadioFrame(const ITransmission *transmission);

    /**
     * Sends a copy of the provided radio frame to all affected receivers on
     * the radio medium.
     */
    virtual void sendToAffectedRadios(IRadio *transmitter, const IRadioFrame *frame);

    /**
     * Sends a copy of the provided radio frame to all receivers on the radio medium.
     */
    virtual void sendToAllRadios(IRadio *transmitter, const IRadioFrame *frame);
    //@}

    /** @name Reception */
    //@{
    virtual bool isRadioMacAddress(const IRadio *radio, const MACAddress address) const;

    /**
     * Returns true if the radio can potentially receive the transmission
     * successfully. If this function returns false then the radio medium
     * doesn't send a radio frame to this receiver.
     */
    virtual bool isPotentialReceiver(const IRadio *receiver, const ITransmission *transmission) const;

    virtual bool isInCommunicationRange(const ITransmission *transmission, const Coord startPosition, const Coord endPosition) const;
    virtual bool isInInterferenceRange(const ITransmission *transmission, const Coord startPosition, const Coord endPosition) const;

    virtual bool isInterferingTransmission(const ITransmission *transmission, const IListening *listening) const;
    virtual bool isInterferingTransmission(const ITransmission *transmission, const IReception *reception) const;

    /**
     * Removes all cached data related to past transmissions that don't have
     * any effect on any ongoing transmission. Note that it's possible that a
     * transmission is in the past but it's still needed to compute the total
     * interference for another.
     */
    virtual void removeNonInterferingTransmissions();

    virtual const std::vector<const IReception *> *computeInterferingReceptions(const IListening *listening, const std::vector<const ITransmission *> *transmissions) const;
    virtual const std::vector<const IReception *> *computeInterferingReceptions(const IReception *reception, const std::vector<const ITransmission *> *transmissions) const;

    virtual const IReception *computeReception(const IRadio *receiver, const ITransmission *transmission) const;
    virtual const IInterference *computeInterference(const IRadio *receiver, const IListening *listening, const std::vector<const ITransmission *> *transmissions) const;
    virtual const IInterference *computeInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const;
    virtual const IReceptionDecision *computeReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, IRadioSignal::SignalPart part, const std::vector<const ITransmission *> *transmissions) const;
    virtual const IReceptionResult *computeReceptionResult(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const;
    virtual const IListeningDecision *computeListeningDecision(const IRadio *receiver, const IListening *listening, const std::vector<const ITransmission *> *transmissions) const;
    //@}

  public:
    RadioMedium();
    virtual ~RadioMedium();

    virtual std::ostream& printToStream(std::ostream &stream, int level) const override;

    virtual const IMaterial *getMaterial() const override { return material; }
    virtual const IPropagation *getPropagation() const override { return propagation; }
    virtual const IPathLoss *getPathLoss() const override { return pathLoss; }
    virtual const IObstacleLoss *getObstacleLoss() const override { return obstacleLoss; }
    virtual const IAnalogModel *getAnalogModel() const override { return analogModel; }
    virtual const IBackgroundNoise *getBackgroundNoise() const override { return backgroundNoise; }
    virtual const IPhysicalEnvironment *getPhysicalEnvironment() const override { return physicalEnvironment; }

    virtual const IMediumLimitCache *getMediumLimitCache() const { return mediumLimitCache; }
    virtual const INeighborCache *getNeighborCache() const { return neighborCache; }
    virtual const ICommunicationCache *getCommunicationCache() const { return communicationCache; }

    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;

    virtual void sendToRadio(IRadio *trasmitter, const IRadio *receiver, const IRadioFrame *frame);

    virtual IRadioFrame *transmitPacket(const IRadio *transmitter, cPacket *macFrame) override;
    virtual cPacket *receivePacket(const IRadio *receiver, IRadioFrame *radioFrame) override;

    virtual const IListeningDecision *listenOnMedium(const IRadio *receiver, const IListening *listening) const override;

    virtual const IArrival *getArrival(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const IListening *getListening(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const IReception *getReception(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const IInterference *getInterference(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const IInterference *getInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const;
    virtual const INoise *getNoise(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const ISNIR *getSNIR(const IRadio *receiver, const ITransmission *transmission) const override;

    virtual bool isReceptionPossible(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const override;
    virtual bool isReceptionAttempted(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const override;
    virtual bool isReceptionSuccessful(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const override;
    virtual const IReceptionDecision *getReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, IRadioSignal::SignalPart part) const override;
    virtual const IReceptionResult *getReceptionResult(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, long value DETAILS_ARG) override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RADIOMEDIUM_H

