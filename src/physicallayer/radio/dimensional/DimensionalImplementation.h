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

#include "ImplementationBase.h"
#include "Mapping.h"

class INET_API DimensionalRadioSignalTransmission : public RadioSignalTransmissionBase
{
    protected:
        const Mapping *power;
        const Hz carrierFrequency; // TODO: shouldn't be here

    public:
        DimensionalRadioSignalTransmission(const IRadio *radio, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition, const Mapping *power, Hz carrierFrequency) :
            RadioSignalTransmissionBase(radio, startTime, endTime, startPosition, endPosition),
            power(power),
            carrierFrequency(carrierFrequency)
        {}

        virtual ~DimensionalRadioSignalTransmission() { delete power; }
        virtual const Mapping *getPower() const { return power; }
        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
};

class INET_API DimensionalRadioSignalLoss : public IRadioSignalLoss
{
    protected:
        const ConstMapping *factor;

    public:
        DimensionalRadioSignalLoss(const ConstMapping *factor) :
            factor(factor)
        {}

        virtual ~DimensionalRadioSignalLoss() { delete factor; }

        virtual const ConstMapping *getFactor() const { return factor; }
};

class INET_API DimensionalRadioSignalReception : public RadioSignalReceptionBase
{
    protected:
        const Mapping *power;
        const Hz carrierFrequency; // TODO: shouldn't be here

    public:
        DimensionalRadioSignalReception(const IRadio *radio, const IRadioSignalTransmission *transmission, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition, const Mapping *power, Hz carrierFrequency) :
            RadioSignalReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition),
            power(power),
            carrierFrequency(carrierFrequency)
        {}

        virtual ~DimensionalRadioSignalReception() { delete power; }

        virtual const Mapping *getPower() const { return power; }
        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
};

class INET_API DimensionalRadioSignalAttenuationBase : public virtual IRadioSignalAttenuation
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
        virtual const IRadioSignalReception *computeReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
};


class INET_API DimensionalRadioSignalFreeSpaceAttenuation : public RadioSignalFreeSpaceAttenuationBase, public DimensionalRadioSignalAttenuationBase
{
    public:
        DimensionalRadioSignalFreeSpaceAttenuation(double alpha) :
            RadioSignalFreeSpaceAttenuationBase(alpha)
        {}

        virtual const IRadioSignalLoss *computeLoss(const IRadioSignalTransmission *transmission, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) const;
};

class INET_API DimensionalRadioBackgroundNoise : public IRadioBackgroundNoise
{
    protected:
        const W power;

    public:
        DimensionalRadioBackgroundNoise(W power) :
            power(power)
        {}

    public:
        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalListening *listening) const;
        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalReception *reception) const;
};

class INET_API DimensionalRadioSignalTransmitter : public RadioSignalTransmitterBase
{
    protected:
        bps bitrate;
        W power;
        Hz carrierFrequency;
        Hz bandwidth;

    public:
        DimensionalRadioSignalTransmitter() :
            bitrate(sNaN),
            power(W(sNaN)),
            carrierFrequency(Hz(sNaN)),
            bandwidth(Hz(sNaN))
        {}

        DimensionalRadioSignalTransmitter(bps bitrate, W power, Hz carrierFrequency, Hz bandwidth) :
            bitrate(bitrate),
            power(power),
            carrierFrequency(carrierFrequency),
            bandwidth(bandwidth)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalTransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const;
};

class INET_API DimensionalRadioSignalReceiver : public SNIRRadioSignalReceiverBase
{
    protected:
        W energyDetection;
        W sensitivity;

    protected:
        virtual bool computeIsReceptionPossible(const IRadioSignalReception *reception) const;
        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *receptions, const IRadioSignalNoise *backgroundNoise) const;
        virtual double computeSNIRMin(const IRadioSignalReception *reception, const IRadioSignalNoise *noise) const;

    public:
        DimensionalRadioSignalReceiver() :
            SNIRRadioSignalReceiverBase(),
            energyDetection(W(sNaN)),
            sensitivity(W(sNaN))
        {}

        DimensionalRadioSignalReceiver(double snirThreshold, W energyDetection, W sensitivity) :
            SNIRRadioSignalReceiverBase(snirThreshold),
            energyDetection(energyDetection),
            sensitivity(sensitivity)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;
        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;
};

#endif
