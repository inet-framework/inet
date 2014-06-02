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

#ifndef __INET_IDEALIMPLEMENTATION_H_
#define __INET_IDEALIMPLEMENTATION_H_

#include "ImplementationBase.h"

class INET_API IdealRadioSignalTransmission : public RadioSignalTransmissionBase
{
    protected:
        const m maxCommunicationRange;
        const m maxInterferenceRange;
        const m maxDetectionRange;

    public:
        IdealRadioSignalTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, m maxCommunicationRange, m maxInterferenceRange, m maxDetectionRange) :
            RadioSignalTransmissionBase(transmitter, macFrame, startTime, endTime, startPosition, endPosition),
            maxCommunicationRange(maxCommunicationRange),
            maxInterferenceRange(maxInterferenceRange),
            maxDetectionRange(maxDetectionRange)
        {}

        virtual m getMaxCommunicationRange() const { return maxCommunicationRange; }
        virtual m getMaxInterferenceRange() const { return maxInterferenceRange; }
        virtual m getMaxDetectionRange() const { return maxDetectionRange; }
};

class INET_API IdealRadioSignalListening : public RadioSignalListeningBase
{
    public:
        IdealRadioSignalListening(const IRadio *radio, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) :
            RadioSignalListeningBase(radio, startTime, endTime, startPosition, endPosition)
        {}
};

class INET_API IdealRadioSignalReception : public RadioSignalReceptionBase
{
    public:
        enum Power
        {
            POWER_RECEIVABLE,
            POWER_INTERFERING,
            POWER_DETECTABLE,
            POWER_UNDETECTABLE
        };

    protected:
        const Power power;

    public:
        IdealRadioSignalReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const Power power) :
            RadioSignalReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition),
            power(power)
        {}

        virtual Power getPower() const { return power; }
};

class INET_API IdealRadioSignalAttenuationBase : public IRadioSignalAttenuation
{
    public:
        virtual const IRadioSignalReception *computeReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
};

class INET_API IdealRadioSignalFreeSpaceAttenuation : public IdealRadioSignalAttenuationBase, public cCompoundModule
{
    public:
        IdealRadioSignalFreeSpaceAttenuation() {}

        virtual void printToStream(std::ostream &stream) const { stream << "ideal free space attenuation"; }
};

class INET_API IdealRadioSignalTransmitter : public RadioSignalTransmitterBase
{
    protected:
        bps bitrate;
        m maxCommunicationRange;
        m maxInterferenceRange;
        m maxDetectionRange;

    protected:
        virtual void initialize(int stage);

    public:
        IdealRadioSignalTransmitter() :
            bitrate(sNaN),
            maxCommunicationRange(sNaN),
            maxInterferenceRange(sNaN),
            maxDetectionRange(sNaN)
        {}

        IdealRadioSignalTransmitter(bps bitrate, m maxCommunicationRange, m maxInterferenceRange, m maxDetectionRange) :
            bitrate(bitrate),
            maxCommunicationRange(maxCommunicationRange),
            maxInterferenceRange(maxInterferenceRange),
            maxDetectionRange(maxDetectionRange)
        {}

        virtual void printToStream(std::ostream &stream) const;
        virtual const IRadioSignalTransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const;
};

class INET_API IdealRadioSignalReceiver : public RadioSignalReceiverBase
{
    protected:
        bool ignoreInterference;

    protected:
        virtual void initialize(int stage);
        virtual bool computeIsReceptionPossible(const IRadioSignalReception *reception) const;
        virtual bool computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const;

    public:
        IdealRadioSignalReceiver() :
            ignoreInterference(false)
        {}

        IdealRadioSignalReceiver(bool ignoreInterference) :
            ignoreInterference(ignoreInterference)
        {}

        virtual void printToStream(std::ostream &stream) const;
        virtual const IRadioSignalListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;
        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;
};

#endif
