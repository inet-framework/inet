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

#ifndef __INET_SCALARIMPLEMENTATION_H_
#define __INET_SCALARIMPLEMENTATION_H_

#include "FlatImplementationBase.h"
#include "FreeSpacePathLoss.h"
#include "GenericImplementation.h"
#include "IModulation.h"
#include "Radio.h"

namespace radio
{

class INET_API ScalarRadioSignalTransmission : public FlatRadioSignalTransmissionBase
{
    protected:
        const bps bitrate;
        const W power;

    public:
        ScalarRadioSignalTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, const IModulation *modulation, int headerBitLength, int payloadBitLength, Hz carrierFrequency, Hz bandwidth, bps bitrate, W power) :
            FlatRadioSignalTransmissionBase(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, payloadBitLength, carrierFrequency, bandwidth),
            bitrate(bitrate),
            power(power)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual bps getBitrate() const { return bitrate; }
        virtual W getPower() const { return power; }
};

class INET_API ScalarRadioSignalReception : public FlatRadioSignalReceptionBase
{
    protected:
        const W power;

    public:
        ScalarRadioSignalReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, Hz carrierFrequency, Hz bandwidth, W power) :
            FlatRadioSignalReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, carrierFrequency, bandwidth),
            power(power)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual W getPower() const { return power; }
};

class INET_API ScalarRadioSignalNoise : public FlatRadioSignalNoiseBase
{
    protected:
        const std::map<simtime_t, W> *powerChanges;

    public:
        ScalarRadioSignalNoise(simtime_t startTime, simtime_t endTime, Hz carrierFrequency, Hz bandwidth, const std::map<simtime_t, W> *powerChanges) :
            FlatRadioSignalNoiseBase(startTime, endTime, carrierFrequency, bandwidth),
            powerChanges(powerChanges)
        {}

        virtual ~ScalarRadioSignalNoise() { delete powerChanges; }
        virtual void printToStream(std::ostream &stream) const { stream << "scalar noise"; }
        virtual const std::map<simtime_t, W> *getPowerChanges() const { return powerChanges; }
        virtual W computeMaxPower(simtime_t startTime, simtime_t endTime) const;
};

class INET_API ScalarRadioSignalAttenuation : public RadioSignalAttenuationBase
{
    public:
        virtual void printToStream(std::ostream &stream) const { stream << "scalar radio signal attenuation"; }
        virtual const IRadioSignalReception *computeReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
};

class INET_API ScalarRadioBackgroundNoise : public cModule, public IRadioBackgroundNoise
{
    protected:
        W power;

    protected:
        virtual void initialize(int stage);

    public:
        ScalarRadioBackgroundNoise() :
            power(W(sNaN))
        {}

        virtual void printToStream(std::ostream &stream) const { stream << "scalar background noise"; }

        virtual W getPower() const { return power; }

        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalListening *listening) const;
};

class INET_API ScalarRadioSignalListeningDecision : public RadioSignalListeningDecision
{
    protected:
        const W powerMax;

    public:
        ScalarRadioSignalListeningDecision(const IRadioSignalListening *listening, bool isListeningPossible, W powerMax) :
            RadioSignalListeningDecision(listening, isListeningPossible),
            powerMax(powerMax)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual W getPowerMax() const { return powerMax; }
};

class INET_API ScalarRadioSignalTransmitter: public FlatRadioSignalTransmitterBase
{
    protected:
        bps bitrate;
        W power;

    protected:
        virtual void initialize(int stage);

    public:
        ScalarRadioSignalTransmitter() :
            FlatRadioSignalTransmitterBase(),
            bitrate(sNaN),
            power(W(sNaN))
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalTransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const;

        virtual bps getBitrate() const { return bitrate; }
        virtual void setBitrate(bps bitrate) { this->bitrate = bitrate; }

        virtual W getPower() const { return power; }
        virtual void setPower(W power) { this->power = power; }
};

class INET_API ScalarRadioSignalReceiver : public SNIRRadioSignalReceiverBase
{
    protected:
        const IModulation *modulation;
        W energyDetection;
        W sensitivity;
        Hz carrierFrequency;
        Hz bandwidth;

    protected:
        virtual void initialize(int stage);

        virtual bool computeIsReceptionPossible(const IRadioSignalTransmission *transmission) const;
        virtual bool computeIsReceptionPossible(const IRadioSignalReception *reception) const;
        virtual bool computeIsReceptionSuccessful(const IRadioSignalReception *reception, const RadioReceptionIndication *indication) const;

        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *receptions, const IRadioSignalNoise *backgroundNoise) const;
        virtual double computeMinSNIR(const IRadioSignalReception *reception, const IRadioSignalNoise *noise) const;
        virtual bool computeHasBitError(double minSNIR, int bitLength, double bitrate) const;

    public:
        ScalarRadioSignalReceiver() :
            SNIRRadioSignalReceiverBase(),
            modulation(NULL),
            energyDetection(W(sNaN)),
            sensitivity(W(sNaN)),
            carrierFrequency(Hz(sNaN)),
            bandwidth(Hz(sNaN))
        {}

        virtual ~ScalarRadioSignalReceiver() { delete modulation; }

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;

        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;

        virtual const IModulation *getModulation() const { return modulation; }

        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual void setCarrierFrequency(Hz carrierFrequency) { this->carrierFrequency = carrierFrequency; }

        virtual Hz getBandwidth() const { return bandwidth; }
        virtual void setBandwidth(Hz bandwidth) { this->bandwidth = bandwidth; }
};

}

#endif
