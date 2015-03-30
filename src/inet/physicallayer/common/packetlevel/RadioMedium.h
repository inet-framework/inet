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
#include "inet/common/TrailFigure.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/environment/contract/IMaterialRegistry.h"
#include "inet/physicallayer/contract/packetlevel/ISNIR.h"
#include "inet/physicallayer/contract/packetlevel/INeighborCache.h"
#include "inet/physicallayer/contract/packetlevel/ICommunicationCache.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/contract/packetlevel/IInterference.h"
#include "inet/physicallayer/contract/packetlevel/IPropagation.h"
#include "inet/physicallayer/contract/packetlevel/IAnalogModel.h"
#include "inet/physicallayer/contract/packetlevel/IBackgroundNoise.h"
#include "inet/physicallayer/common/packetlevel/CommunicationLog.h"
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
    /** @name Parameters that control the behavior of the radio medium. */
    //@{
    /**
     * The propagation model of the medium is never nullptr.
     */
    const IPropagation *propagation;
    /**
     * The path loss model of the medium is never nullptr.
     */
    const IPathLoss *pathLoss;
    /**
     * The obstacle loss model of the medium or nullptr if unused.
     */
    const IObstacleLoss *obstacleLoss;
    /**
     * The analog model of the medium is never nullptr.
     */
    const IAnalogModel *analogModel;
    /**
     * The background noise model or nullptr if unused.
     */
    const IBackgroundNoise *backgroundNoise;
    /**
     * The physical environment model or nullptr if unused.
     */
    const IPhysicalEnvironment *environment;
    /**
     * The physical material of the medium.
     */
    const IMaterial *material;
    /**
     * The maximum speed among the radios is in the range [0, +infinity) or
     * NaN if unspecified.
     */
    mps maxSpeed;
    /**
     * The constraint area minimum among the radios is in the range [-infinity,
     * +infinity] or NaN if unspecified.
     */
    Coord constraintAreaMin;
    /**
     * The constraint area maximum among the radios is in the range [-infinity,
     * +infinity] or NaN if unspecified.
     */
    Coord constraintAreaMax;
    /**
     * The maximum transmission power among the radio transmitters is in the
     * range [0, +infinity) or NaN if unspecified.
     */
    W maxTransmissionPower;
    /**
     * The minimum interference power among the radio receivers is in the
     * range [0, +infinity) or NaN if unspecified.
     */
    W minInterferencePower;
    /**
     * The minimum reception power among the radio receivers is in the range
     * [0, +infinity) or NaN if unspecified.
     */
    W minReceptionPower;
    /**
     * The maximum gain among the radio antennas is in the range [1, +infinity).
     */
    double maxAntennaGain;
    /**
     * The minimum overlapping in time needed to consider two transmissions
     * interfering.
     */
    // TODO: maybe compute from longest frame duration, maximum mobility speed and signal propagation time
    simtime_t minInterferenceTime;
    /**
     * The maximum transmission duration of a radio signal.
     */
    // TODO: maybe compute from maximum bit length and minimum bitrate
    simtime_t maxTransmissionDuration;
    /**
     * The maximum communication range where a transmission can still be
     * potentially successfully received is in the range [0, +infinity) or
     * NaN if unspecified.
     */
    m maxCommunicationRange;
    /**
     * The maximum interference range where a transmission is still considered
     * to some effect on other transmissions is in the range [0, +infinity)
     * or NaN if unspecified.
     */
    m maxInterferenceRange;
    /**
     * The radio medium doesn't send radio frames to a radio if it's outside
     * the provided range.
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
     * frequency and bandwidth, different modulation, etc.)
     */
    bool listeningFilter;
    /**
     * True means the radio medium doesn't send radio frames to a radio if
     * it the destination mac address differs.
     */
    bool macAddressFilter;
    /**
     * Records all transmissions and receptions into a separate trace file.
     * The file is at ${resultdir}/${configname}-${runnumber}.tlog
     */
    bool recordCommunicationLog;
    /**
     * Displays ongoing communications on the canvas.
     */
    bool displayCommunication;
    /**
     * Determines ongoing communication figure: 3D spheres or 2D circles on the X-Y plane.
     */
    bool drawCommunication2D;
    /**
     * Leaves graphical trail of successful communication between radios.
     */
    bool leaveCommunicationTrail;
    /**
     * Update canvas interval when ongoing communication exists.
     */
    simtime_t updateCanvasInterval;
    //@}

    /** @name Timer */
    //@{
    /**
     * The message used to update the canvas when ongoing communication exists.
     */
    cMessage *updateCanvasTimer;
    /**
     * The message used to purge internal state and cache.
     */
    cMessage *removeNonInterferingTransmissionsTimer;
    //@}

    /** @name State */
    //@{
    /**
     * The list of radios that can transmit and receive radio signals on the
     * radio medium. The radios follow each other in the order of their unique
     * id. Radios are only removed from the beginning. This list may contain
     * NULL pointers.
     */
    std::vector<const IRadio *> radios;
    /**
     * The list of ongoing transmissions on the radio medium. The transmissions
     * follow each other in the order of their unique id. Transmissions are only
     * removed from the beginning. This list doesn't contain NULL pointers.
     */
    std::vector<const ITransmission *> transmissions;
    //@}

    /** @name Cache */
    //@{
    /**
     * Caches communication for all radios.
     */
    mutable ICommunicationCache *communicationCache;
    /**
     * Caches neighbors for all radios or nullptr if turned off.
     */
    mutable INeighborCache *neighborCache;
    //@}

    /** @name Logging */
    //@{
    /**
     * The output communication log is written to.
     */
    CommunicationLog communicationLog;
    //@}

    /** @name Graphics */
    //@{
    /**
     * The list figures representing ongoing communications.
     */
    cGroupFigure *communicationLayer;
    /**
     * The list of trail figures representing successful communications.
     */
    TrailFigure *communicationTrail;
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
    mutable long sendCount;
    /**
     * Total number of reception computations.
     */
    mutable long receptionComputationCount;
    /**
     * Total number of interference computations.
     */
    mutable long interferenceComputationCount;
    /**
     * Total number of reception decision computations.
     */
    mutable long receptionDecisionComputationCount;
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
    //@}

  protected:
    /** @name Module */
    //@{
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *message) override;
    //@}

    /** @name Limits */
    //@{
    virtual mps computeMaxSpeed() const;

    virtual W computeMaxTransmissionPower() const;
    virtual W computeMinInterferencePower() const;
    virtual W computeMinReceptionPower() const;
    virtual double computeMaxAntennaGain() const;

    virtual const simtime_t computeMinInterferenceTime() const;
    virtual const simtime_t computeMaxTransmissionDuration() const;

    virtual m computeMaxRange(W maxTransmissionPower, W minReceptionPower) const;
    virtual m computeMaxCommunicationRange() const;
    virtual m computeMaxInterferenceRange() const;
    virtual Coord computeConstraintAreaMin() const;
    virtual Coord computeConstreaintAreaMax() const;
    virtual void updateLimits();
    //@}

    /** @name Transmission */
    //@{
    /**
     * Adds a new transmission to the radio medium.
     */
    virtual void addTransmission(const IRadio *transmitter, const ITransmission *transmission);

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
    virtual const IReceptionDecision *computeReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const;
    virtual const IListeningDecision *computeListeningDecision(const IRadio *receiver, const IListening *listening, const std::vector<const ITransmission *> *transmissions) const;

    virtual const IArrival *getArrival(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const IListening *getListening(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const IReception *getReception(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const IInterference *getInterference(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const IInterference *getInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const;
    virtual const INoise *getNoise(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const ISNIR *getSNIR(const IRadio *receiver, const ITransmission *transmission) const override;
    virtual const IReceptionDecision *getReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const override;
    //@}

    /** @name Graphics */
    //@{
    virtual void updateCanvas();
    virtual void scheduleUpdateCanvasTimer();
    //@}

  public:
    RadioMedium();
    virtual ~RadioMedium();

    virtual void printToStream(std::ostream &stream) const override;

    virtual W getMinInterferencePower() const override { return minInterferencePower; }
    virtual W getMinReceptionPower() const override { return minReceptionPower; }
    virtual double getMaxAntennaGain() const override { return maxAntennaGain; }
    virtual mps getMaxSpeed() const { return maxSpeed; }
    virtual m getMaxInterferenceRange(const IRadio *radio) const;
    virtual m getMaxCommunicationRange(const IRadio *radio) const;
    virtual Coord getConstraintAreaMin() const { return constraintAreaMin; }
    virtual Coord getConstraintAreaMax() const { return constraintAreaMax; }

    virtual const IMaterial *getMaterial() const override { return material; }
    virtual const IPropagation *getPropagation() const override { return propagation; }
    virtual const IPathLoss *getPathLoss() const override { return pathLoss; }
    virtual const IObstacleLoss *getObstacleLoss() const override { return obstacleLoss; }
    virtual const IAnalogModel *getAnalogModel() const override { return analogModel; }
    virtual const IBackgroundNoise *getBackgroundNoise() const override { return backgroundNoise; }

    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;

    virtual void sendToRadio(IRadio *trasmitter, const IRadio *receiver, const IRadioFrame *frame);

    virtual IRadioFrame *transmitPacket(const IRadio *transmitter, cPacket *macFrame) override;
    virtual cPacket *receivePacket(const IRadio *receiver, IRadioFrame *radioFrame) override;

    virtual const IListeningDecision *listenOnMedium(const IRadio *receiver, const IListening *listening) const override;

    virtual bool isReceptionAttempted(const IRadio *receiver, const ITransmission *transmission) const override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, long value) override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RADIOMEDIUM_H

