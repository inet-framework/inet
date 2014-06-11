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

#ifndef __INET_DIMENSIONALIMPLEMENTATION_H_
#define __INET_DIMENSIONALIMPLEMENTATION_H_

#include "FlatImplementationBase.h"
#include "MappingBase.h"
#include "MappingUtils.h"

class INET_API DimensionalRadioSignalTransmission : public FlatRadioSignalTransmissionBase
{
    protected:
        const bps bitrate;
        const ConstMapping *power;

    public:
        DimensionalRadioSignalTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, const IModulation *modulation, int headerBitLength, int payloadBitLength, Hz carrierFrequency, Hz bandwidth, bps bitrate, const ConstMapping *power) :
            FlatRadioSignalTransmissionBase(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, payloadBitLength, carrierFrequency, bandwidth),
            bitrate(bitrate),
            power(power)
        {}

        virtual ~DimensionalRadioSignalTransmission() { delete power; }
        virtual bps getBitrate() const { return bitrate; }
        virtual const ConstMapping *getPower() const { return power; }
};

class INET_API DimensionalRadioSignalReception : public FlatRadioSignalReceptionBase
{
    protected:
        const ConstMapping *power;

    public:
        DimensionalRadioSignalReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power) :
            FlatRadioSignalReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, carrierFrequency, bandwidth),
            power(power)
        {}

        virtual ~DimensionalRadioSignalReception() { delete power; }

        virtual const ConstMapping *getPower() const { return power; }
        virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const;
};

class INET_API DimensionalRadioSignalNoise : public FlatRadioSignalNoiseBase
{
    protected:
        const ConstMapping *power;

    public:
        DimensionalRadioSignalNoise(simtime_t startTime, simtime_t endTime, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power) :
            FlatRadioSignalNoiseBase(startTime, endTime, carrierFrequency, bandwidth),
            power(power)
        {}

        virtual ~DimensionalRadioSignalNoise() { delete power; }
        virtual void printToStream(std::ostream &stream) const { stream << "dimensional noise"; }
        virtual const ConstMapping *getPower() const { return power; }
        virtual W computeMaxPower(simtime_t startTime, simtime_t endTime) const;
};

class INET_API DimensionalRadioSignalAttenuation : public RadioSignalAttenuationBase
{
    protected:
        class INET_API LossConstMapping : public SimpleConstMapping
        {
            private:
                LossConstMapping &operator=(const LossConstMapping&);

            protected:
                const double factor;

            public:
                LossConstMapping(const DimensionSet &dimensions, double factor) :
                    SimpleConstMapping(dimensions),
                    factor(factor)
                {}

                LossConstMapping(const LossConstMapping &other) :
                    SimpleConstMapping(other),
                    factor(other.factor)
                {}

                virtual double getValue(const Argument &position) const { return factor; }

                virtual ConstMapping *constClone() const { return new LossConstMapping(*this); }
        };

    public:
        virtual void printToStream(std::ostream &stream) const { stream << "dimensional radio signal attenuation"; }
        virtual const IRadioSignalReception *computeReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
};

class INET_API DimensionalRadioBackgroundNoise : public cModule, public IRadioBackgroundNoise
{
    protected:
        W power;

    protected:
        virtual void initialize(int stage);

    public:
        DimensionalRadioBackgroundNoise() :
            power(W(sNaN))
        {}

    public:
        virtual void printToStream(std::ostream &stream) const { stream << "dimensional background noise"; }
        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalListening *listening) const;
};

class INET_API DimensionalRadioSignalTransmitter : public FlatRadioSignalTransmitterBase
{
    protected:
        bps bitrate;
        W power;

    protected:
        virtual void initialize(int stage);

    public:
        DimensionalRadioSignalTransmitter() :
            FlatRadioSignalTransmitterBase(),
            bitrate(sNaN),
            power(W(sNaN))
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalTransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const;
};

class INET_API DimensionalRadioSignalReceiver : public SNIRRadioSignalReceiverBase
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
        DimensionalRadioSignalReceiver() :
            SNIRRadioSignalReceiverBase(),
            modulation(NULL),
            energyDetection(W(sNaN)),
            sensitivity(W(sNaN)),
            carrierFrequency(Hz(sNaN)),
            bandwidth(Hz(sNaN))
        {}

        virtual ~DimensionalRadioSignalReceiver() { delete modulation; }

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

#endif
