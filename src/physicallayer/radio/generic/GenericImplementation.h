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

#ifndef __INET_GENERICIMPLEMENTATION_H_
#define __INET_GENERICIMPLEMENTATION_H_

#include "ImplementationBase.h"

class INET_API RadioSignalArrival : public virtual IRadioSignalArrival
{
    protected:
        const simtime_t startPropagationTime;
        const simtime_t endPropagationTime;
        const simtime_t startTime;
        const simtime_t endTime;
        const Coord startPosition;
        const Coord endPosition;
        const EulerAngles startOrientation;
        const EulerAngles endOrientation;

    public:
        RadioSignalArrival(const simtime_t startPropagationTime, const simtime_t endPropagationTime, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation) :
            startPropagationTime(startPropagationTime),
            endPropagationTime(endPropagationTime),
            startTime(startTime),
            endTime(endTime),
            startPosition(startPosition),
            endPosition(endPosition),
            startOrientation(startOrientation),
            endOrientation(endOrientation)
        {}

        virtual void printToStream(std::ostream &stream) const {}

        virtual const simtime_t getStartPropagationTime() const { return startPropagationTime; }
        virtual const simtime_t getEndPropagationTime() const { return endPropagationTime; }

        virtual const simtime_t getStartTime() const { return startTime; }
        virtual const simtime_t getEndTime() const { return endTime; }

        virtual const Coord getStartPosition() const { return startPosition; }
        virtual const Coord getEndPosition() const { return endPosition; }

        virtual const EulerAngles getStartOrientation() const { return startOrientation; }
        virtual const EulerAngles getEndOrientation() const { return endOrientation; }
};

class INET_API IsotropicRadioAntenna : public RadioAntennaBase
{
    public:
        IsotropicRadioAntenna() :
            RadioAntennaBase()
        {}

        IsotropicRadioAntenna(IMobility *mobility) :
            RadioAntennaBase(mobility)
        {}

        virtual void printToStream(std::ostream &stream) const { stream << "isotropic antenna"; }

        virtual double getMaxGain() const { return 1; }

        virtual double computeGain(EulerAngles direction) const { return 1; }
};

class INET_API ConstantGainRadioAntenna : public RadioAntennaBase
{
    protected:
        double gain;

    protected:
        virtual void initialize(int stage);

    public:
        ConstantGainRadioAntenna() :
            RadioAntennaBase(),
            gain(sNaN)
        {}

        ConstantGainRadioAntenna(IMobility *mobility, double gain) :
            RadioAntennaBase(mobility),
            gain(gain)
        {}

        virtual void printToStream(std::ostream &stream) const { stream << "constant gain antenna"; }

        virtual double getMaxGain() const { return gain; }

        virtual double computeGain(EulerAngles direction) const { return gain; }
};

class INET_API DipoleRadioAntenna : public RadioAntennaBase
{
    protected:
        m length;

    protected:
        virtual void initialize(int stage);

    public:
        DipoleRadioAntenna() :
            RadioAntennaBase()
        {}

        DipoleRadioAntenna(IMobility *mobility, m length) :
            RadioAntennaBase(mobility),
            length(length)
        {}

        virtual void printToStream(std::ostream &stream) const { stream << "dipole antenna"; }

        virtual m getLength() const { return length; }

        virtual double getMaxGain() const { return 1.5; }

        virtual double computeGain(EulerAngles direction) const;
};

class INET_API InterpolatingRadioAntenna : public RadioAntennaBase
{
    public:
        InterpolatingRadioAntenna() :
            RadioAntennaBase()
        {}

        InterpolatingRadioAntenna(IMobility *mobility) :
            RadioAntennaBase(mobility)
        {}

        virtual void printToStream(std::ostream &stream) const { stream << "interpolating antenna"; }

        // TODO: compute max gain
        virtual double getMaxGain() const { return 1; }

        // TODO: compute antenna gain based on a linear interpolation between two elements of the antenna gain table using the antenna positions/orientations
        virtual double computeGain(EulerAngles direction) const { return 1; }
};

class INET_API RadioSignalListeningDecision : public IRadioSignalListeningDecision, public cObject
{
    protected:
        const IRadioSignalListening *listening;
        const bool isListeningPossible_;

    public:
        RadioSignalListeningDecision(const IRadioSignalListening *listening, bool isListeningPossible_) :
            listening(listening),
            isListeningPossible_(isListeningPossible_)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalListening *getListening() const { return listening; }

        virtual bool isListeningPossible() const { return isListeningPossible_; }

};

class INET_API RadioSignalReceptionDecision : public IRadioSignalReceptionDecision, public cObject
{
    protected:
        const IRadioSignalReception *reception;
        const RadioReceptionIndication *indication;
        const bool isSynchronizationPossible_;
        const bool isSynchronizationAttempted_;
        const bool isSynchronizationSuccessful_;
        const bool isReceptionPossible_;
        const bool isReceptionAttempted_;
        const bool isReceptionSuccessful_;

    public:
        RadioSignalReceptionDecision(const IRadioSignalReception *reception, const RadioReceptionIndication *indication, bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful) :
            reception(reception),
            indication(indication),
            isSynchronizationPossible_(false),
            isSynchronizationAttempted_(false),
            isSynchronizationSuccessful_(false),
            isReceptionPossible_(isReceptionPossible),
            isReceptionAttempted_(isReceptionAttempted),
            isReceptionSuccessful_(isReceptionSuccessful)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalReception *getReception() const { return reception; }

        virtual const RadioReceptionIndication *getIndication() const { return indication; }

        virtual bool isReceptionPossible() const { return isReceptionPossible_; }

        virtual bool isReceptionAttempted() const { return isReceptionAttempted_; }

        virtual bool isReceptionSuccessful() const { return isReceptionSuccessful_; }

        virtual bool isSynchronizationPossible() const { return isSynchronizationPossible_; }

        virtual bool isSynchronizationAttempted() const { return isSynchronizationAttempted_; }

        virtual bool isSynchronizationSuccessful() const { return isSynchronizationSuccessful_; }
};

class INET_API ImmediateRadioSignalPropagation : public RadioSignalPropagationBase
{
    public:
        ImmediateRadioSignalPropagation();
        ImmediateRadioSignalPropagation(mps propagationSpeed);

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalArrival *computeArrival(const IRadioSignalTransmission *transmission, IMobility *mobility) const;
};

class INET_API ConstantSpeedRadioSignalPropagation : public RadioSignalPropagationBase
{
    protected:
        const int mobilityApproximationCount;

    protected:
        virtual const Coord computeArrivalPosition(const simtime_t startTime, const Coord startPosition, IMobility *mobility) const;

    public:
        ConstantSpeedRadioSignalPropagation();
        ConstantSpeedRadioSignalPropagation(mps propagationSpeed, int mobilityApproximationCount);

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalArrival *computeArrival(const IRadioSignalTransmission *transmission, IMobility *mobility) const;
};

#endif
