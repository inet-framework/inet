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

#ifndef __INET_RADIOCHANNEL_H_
#define __INET_RADIOCHANNEL_H_

#include <vector>
#include <algorithm>
#include <fstream>
#include "IRadioChannel.h"
#include "IRadioSignalArrival.h"
#include "IRadioSignalPropagation.h"
#include "IRadioSignalAttenuation.h"
#include "IRadioBackgroundNoise.h"

class INET_API RadioChannel : public cSimpleModule, public cListener, public IRadioChannel
{
    protected:
        /**
         * Caches the intermediate computation results related to a reception.
         */
        class ReceptionCacheEntry
        {
            public:
                bool isRadioFrameSent;
                const IRadioSignalArrival *arrival;
                const IRadioSignalReception *reception;
                const IRadioSignalReceptionDecision *decision;

            public:
                ReceptionCacheEntry() :
                    isRadioFrameSent(false),
                    arrival(NULL),
                    reception(NULL),
                    decision(NULL)
                {}
        };

        /**
         * Caches the intermediate computation results related to a transmission.
         */
        class TransmissionCacheEntry
        {
            public:
                simtime_t interferenceEndTime;
                std::vector<ReceptionCacheEntry> *receptionCacheEntries;

            public:
                TransmissionCacheEntry() :
                    receptionCacheEntries(NULL)
                {}
        };

        enum RangeFilterKind
        {
            RANGE_FILTER_ANYWHERE,
            RANGE_FILTER_INTERFERENCE_RANGE,
            RANGE_FILTER_COMMUNICATION_RANGE,
        };

    protected:
        /** @name Parameters that control the behavior of the radio channel. */
        //@{
        /**
         * The propagation model of transmissions is never NULL.
         */
        const IRadioSignalPropagation *propagation;
        /**
         * The attenuation model of transmissions is never NULL.
         */
        const IRadioSignalAttenuation *attenuation;
        /**
         * The radio channel background noise model or NULL if unspecified.
         */
        const IRadioBackgroundNoise *backgroundNoise;
        /**
         * The maximum transmission power among the radio transmitters is in the
         * range [0, +infinity] or NaN if unspecified.
         */
        W maxTransmissionPower;
        /**
         * The minimum interference power among the radio receivers is in the
         * range [0, +infinity] or NaN if unspecified.
         */
        W minInterferencePower;
        /**
         * The minimum reception power among the radio receivers is in the range
         * [0, +infinity] or NaN if unspecified.
         */
        W minReceptionPower;
        /**
         * The maximum gain among the radio antennas is in the range [1, +infinity].
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
         * potentially successfully received is in the range [0, +infinity] or
         * NaN if unspecified.
         */
        m maxCommunicationRange;
        /**
         * The maximum interference range where a transmission is still considered
         * to some effect on other transmissions is in the range [0, +infinity]
         * or NaN if unspecified.
         */
        m maxInterferenceRange;
        /**
         * The radio channel doesn't send radio frames to a radio if it's outside
         * the provided range.
         */
        RangeFilterKind rangeFilter;
        /**
         * True means the radio channel doesn't send radio frames to a radio if
         * it's neither in receiver nor in transceiver mode.
         */
        bool radioModeFilter;
        /**
         * True means the radio channel doesn't send radio frames to a radio if
         * it listens on the channel in incompatible mode (e.g. different carrier
         * frequency and bandwidth, different modulation, etc.)
         */
        bool listeningFilter;
        /**
         * True means the radio channel doesn't send radio frames to a radio if
         * it the destination mac address differs.
         */
        // TODO: complete implementation
        bool macAddressFilter;
        /**
         * Record all transmissions and receptions into a separate trace file.
         * The file is at ${resultdir}/${configname}-${runnumber}.tlog
         */
        bool recordCommunication;
        //@}

        /** @name Timer */
        //@{
        /**
         * The message used to purge internal state and cache.
         */
        cMessage *removeNonInterferingTransmissionsTimer;
        //@}

        /** @name State */
        //@{
        /**
         * The list of radios that transmit and receive radio signals on the
         * radio channel.
         */
        std::vector<const IRadio *> radios;
        /**
         * The list of ongoing transmissions on the radio channel.
         */
        // TODO: consider using an interval graph for receptions (per receiver radio)
        std::vector<const IRadioSignalTransmission *> transmissions;
        //@}

        /** @name Cache */
        //@{
        /**
         * The smallest radio id of all radios.
         */
        int baseRadioId;
        /**
         * The smallest transmission id of all ongoing transmissions.
         */
        int baseTransmissionId;
        /**
         * Caches pre-computed radio signal information for transmissions and
         * radios. The outer vector is indexed by transmission id (offset with
         * base transmission id) and the inner vector is indexed by radio id.
         * Values that are no longer needed are removed from the beginning only.
         * May contain NULL values for not yet pre-computed information.
         */
        mutable std::vector<TransmissionCacheEntry> cache;
        //@}

        /** @name Logging */
        //@{
        /**
         * The output file where communication log is written to.
         */
        std::ofstream communicationLog;
        //@}

        /** @name Statistics */
        //@{
        /**
         * Total number of transmissions.
         */
        mutable long transmissionCount;
        /**
         * Total number of reception computations.
         */
        mutable long receptionComputationCount;
        /**
         * Total number of reception decision computations.
         */
        mutable long receptionDecisionComputationCount;
        /**
         * Total number of listening decision computations.
         */
        mutable long listeningDecisionComputationCount;
        /**
         * Total number of radio signal reception cache queries.
         */
        mutable long cacheReceptionGetCount;
        /**
         * Total number of radio signal reception cache hits.
         */
        mutable long cacheReceptionHitCount;
        /**
         * Total number of radio signal reception decision cache queries.
         */
        mutable long cacheDecisionGetCount;
        /**
         * Total number of radio signal reception decision cache hits.
         */
        mutable long cacheDecisionHitCount;
        //@}

    protected:
        /** @name Module */
        //@{
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void finish();
        virtual void handleMessage(cMessage *message);
        //@}

        /** @name Cache */
        //@{
        virtual TransmissionCacheEntry *getTransmissionCacheEntry(const IRadioSignalTransmission *transmission) const;
        virtual ReceptionCacheEntry *getReceptionCacheEntry(const IRadio *radio, const IRadioSignalTransmission *transmission) const;

        virtual const IRadioSignalArrival *getCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual void setCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalArrival *arrival) const;
        virtual void removeCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const;

        virtual const IRadioSignalReception *getCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual void setCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception) const;
        virtual void removeCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;

        virtual const IRadioSignalReceptionDecision *getCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual void setCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReceptionDecision *decision) const;
        virtual void removeCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const;

        virtual void invalidateCachedDecisions(const IRadioSignalTransmission *transmission);
        virtual void invalidateCachedDecision(const IRadioSignalReceptionDecision *decision);
        //@}

        /** @name Limits */
        //@{
        virtual W computeMaxTransmissionPower() const;
        virtual W computeMinInterferencePower() const;
        virtual W computeMinReceptionPower() const;
        virtual double computeMaxAntennaGain() const;

        virtual const simtime_t computeMinInterferenceTime() const;
        virtual const simtime_t computeMaxTransmissionDuration() const;

        virtual m computeMaxRange(W maxTransmissionPower, W minReceptionPower) const;
        virtual m computeMaxCommunicationRange() const;
        virtual m computeMaxInterferenceRange() const;

        virtual void updateLimits();
        //@}

        /** @name Reception */
        //@{
        virtual bool isInCommunicationRange(const IRadioSignalTransmission *transmission, const Coord startPosition, const Coord endPosition) const;
        virtual bool isInInterferenceRange(const IRadioSignalTransmission *transmission, const Coord startPosition, const Coord endPosition) const;

        virtual bool isInterferingTransmission(const IRadioSignalTransmission *transmission, const IRadioSignalListening *listening) const;
        virtual bool isInterferingTransmission(const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception) const;

        virtual void removeNonInterferingTransmissions();

        virtual const IRadioSignalReception *computeReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual const std::vector<const IRadioSignalReception *> *computeInterferingReceptions(const IRadioSignalListening *listening, const std::vector<const IRadioSignalTransmission *> *transmissions) const;
        virtual const std::vector<const IRadioSignalReception *> *computeInterferingReceptions(const IRadioSignalReception *reception, const std::vector<const IRadioSignalTransmission *> *transmissions) const;
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission, const std::vector<const IRadioSignalTransmission *> *transmissions) const;
        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadio *radio, const IRadioSignalListening *listening, const std::vector<const IRadioSignalTransmission *> *transmissions) const;

        virtual const IRadioSignalReception *getReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual const IRadioSignalReceptionDecision *getReceptionDecision(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const;
        //@}

    public:
        RadioChannel();
        RadioChannel(const IRadioSignalPropagation *propagation, const IRadioSignalAttenuation *attenuation, const IRadioBackgroundNoise *backgroundNoise, const simtime_t minInterferenceTime, const simtime_t maxTransmissionDuration, m maxCommunicationRange, m maxInterferenceRange);
        virtual ~RadioChannel();

        virtual const IRadioSignalPropagation *getPropagation() const { return propagation; }
        virtual const IRadioSignalAttenuation *getAttenuation() const { return attenuation; }
        virtual const IRadioBackgroundNoise *getBackgroundNoise() const { return backgroundNoise; }

        virtual void addRadio(const IRadio *radio);
        virtual void removeRadio(const IRadio *radio);

        virtual void transmitToChannel(const IRadio *radio, const IRadioSignalTransmission *transmission);
        virtual void sendToChannel(IRadio *radio, const IRadioFrame *frame);

        virtual IRadioFrame *transmitPacket(const IRadio *radio, cPacket *macFrame);
        virtual cPacket *receivePacket(const IRadio *radio, IRadioFrame *radioFrame);

        virtual const IRadioSignalReceptionDecision *receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const;
        virtual const IRadioSignalListeningDecision *listenOnChannel(const IRadio *radio, const IRadioSignalListening *listening) const;

        virtual bool isPotentialReceiver(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual bool isReceptionAttempted(const IRadio *radio, const IRadioSignalTransmission *transmission) const;

        virtual const IRadioSignalArrival *getArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const;

        virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object);
};

#endif
