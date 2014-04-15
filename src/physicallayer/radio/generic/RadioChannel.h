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
#include "RadioChannelBase.h"
#include "IRadioSignalArrival.h"
#include "IRadioSignalPropagation.h"
#include "IRadioSignalAttenuation.h"
#include "IRadioBackgroundNoise.h"

class INET_API RadioChannel : public RadioChannelBase, public IRadioChannel
{
    protected:
        // TODO: compute from longest frame duration, maximum mobility speed and signal propagation time
        simtime_t minInterferenceTime;
        simtime_t maxTransmissionDuration;
        m maxCommunicationRange;
        m maxInterferenceRange;

        const IRadioSignalPropagation *propagation;
        const IRadioSignalAttenuation *attenuation;
        const IRadioBackgroundNoise *backgroundNoise;

        std::vector<const IRadio *> radios;
        std::vector<const IRadioSignalTransmission *> transmissions;
        // TODO: is it a cache or part of the transmission?
        std::vector<std::vector<const IRadioSignalArrival *> > arrivals;

    protected:
        // TODO: virtual W computeMaxTransmissionPower() const = 0;
        // TODO: virtual W computeMinReceptionPower() const = 0;

        virtual const simtime_t computeMinInterferenceTime() const;
        virtual const simtime_t computeMaxTransmissionDuration() const;

        virtual m computeMaxCommunicationRange() const;
        virtual m computeMaxInterferenceRange() const;

        virtual bool isInCommunicationRange(const IRadioSignalTransmission *transmission, const Coord startPosition, const Coord endPosition) const;
        virtual bool isInInterferenceRange(const IRadioSignalTransmission *transmission, const Coord startPosition, const Coord endPosition) const;

        virtual bool isInterferingTransmission(const IRadioSignalTransmission *transmission, const IRadioSignalListening *listening) const;
        virtual bool isInterferingTransmission(const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception) const;

        virtual void removeNonInterferingTransmissions();

        virtual const IRadioSignalReception *computeReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual const std::vector<const IRadioSignalTransmission *> *computeInterferingTransmissions(const IRadioSignalListening *listening, const std::vector<const IRadioSignalTransmission *> *transmissions) const;
        virtual const std::vector<const IRadioSignalTransmission *> *computeInterferingTransmissions(const IRadioSignalReception *reception, const std::vector<const IRadioSignalTransmission *> *transmissions) const;
        virtual const std::vector<const IRadioSignalReception *> *computeInterferingReceptions(const IRadioSignalListening *listening, const std::vector<const IRadioSignalTransmission *> *transmissions) const;
        virtual const std::vector<const IRadioSignalReception *> *computeInterferingReceptions(const IRadioSignalReception *reception, const std::vector<const IRadioSignalTransmission *> *transmissions) const;
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission, const std::vector<const IRadioSignalTransmission *> *transmissions) const;
        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadio *radio, const IRadioSignalListening *listening, const std::vector<const IRadioSignalTransmission *> *transmissions) const;

    public:
        RadioChannel() :
            minInterferenceTime(sNaN),
            maxTransmissionDuration(sNaN),
            maxCommunicationRange(m(sNaN)),
            maxInterferenceRange(m(sNaN)),
            propagation(NULL),
            attenuation(NULL),
            backgroundNoise(NULL)
        {}

        RadioChannel(const IRadioSignalPropagation *propagation, const IRadioSignalAttenuation *attenuation, const IRadioBackgroundNoise *backgroundNoise) :
            minInterferenceTime(sNaN),
            maxTransmissionDuration(sNaN),
            maxCommunicationRange(m(sNaN)),
            maxInterferenceRange(m(sNaN)),
            propagation(propagation),
            attenuation(attenuation),
            backgroundNoise(backgroundNoise)
        {}

        virtual ~RadioChannel();

        virtual const IRadioSignalPropagation *getPropagation() const { return propagation; }
        virtual const IRadioSignalAttenuation *getAttenuation() const { return attenuation; }
        virtual const IRadioBackgroundNoise *getBackgroundNoise() const { return backgroundNoise; }

        virtual void addRadio(const IRadio *radio) { radios.push_back(radio); }
        virtual void removeRadio(const IRadio *radio) { radios.erase(std::remove(radios.begin(), radios.end(), radio)); }

        virtual void transmitToChannel(const IRadio *radio, const IRadioSignalTransmission *transmission);
        virtual void sendToChannel(IRadio *radio, const IRadioFrame *frame);

        virtual const IRadioSignalReceptionDecision *receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const;
        virtual const IRadioSignalListeningDecision *listenOnChannel(const IRadio *radio, const IRadioSignalListening *listening) const;

        virtual bool isPotentialReceiver(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual bool isReceptionAttempted(const IRadio *radio, const IRadioSignalTransmission *transmission) const;

        virtual const IRadioSignalArrival *getArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        // KLUDGE: to keep fingerprint
        virtual void setArrival(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalArrival *arrival);
};

#endif
