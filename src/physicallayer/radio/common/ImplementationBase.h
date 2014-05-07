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

#ifndef __INET_IMPLEMENTATIONBASE_H_
#define __INET_IMPLEMENTATIONBASE_H_

#include "IRadioSignalLoss.h"
#include "IRadioBackgroundNoise.h"
#include "IRadioSignalArrival.h"
#include "IRadioSignalAttenuation.h"
#include "IRadioSignalPropagation.h"
#include "IRadioSignalTransmitter.h"
#include "IRadioSignalReceiver.h"

// TODO: revise all names here and also in contract.h
// TODO: optimize interface in terms of constness, use of references, etc.
// TODO: add proper destructors with freeing resources
// TODO: add delete operator calls where appropriate and do proper memory management
// TODO: create a new interface for stream like transmitters/receivers (transmissionStart, transmissionEnd, receptionStart, receptionEnd)
// TODO: !!! extend radio decider interface to allow a separate decision for the detection of preambles during synchronization
// TODO: !!! extend radio decider interface to provide reception state for listeners? and support for carrier sensing for MACs
// TODO: avoid the need for subclassing radio and radio channel to be able to have only one parameterizable radio and radio channel NED types
// TODO: add classification of radios into grid cells to be able provide an approximation of the list of radios within communication range quickly
// TODO: add time parameters to specify the amount of time needed to switch between radio modes
// TODO: extend attenuation model with obstacles, is it a separate model or just another attenuation model?
// TODO: refactor optimizing radio channel to allow turning on and off optimization via runtime parameters instead of subclassing
// TODO: extend interface to allow CUDA optimizations e.g. with adding Pi(x, y, z, t, f, b) and SNIRi, etc. multiple nested loops to compute the minimum SNIR for all transmissions at all receiver radios at once
// TODO: add a skeleton for sampled radio signals or maybe support for GNU radio?
// TODO: using random numbers during computing the radio signal reception decisision is fundamentally wrong because the computation is not purely functional anymore and thus prevents deterministic concurrent parallel computing (because the order of random number generation matters)
// TODO: to solve this issue we might use multiple random number generators (one for each worker) and/or request receivers to provide an upper limit for the count of random numbers needed to be able to deterministically provide them

class INET_API RadioSignalTransmissionBase : public virtual IRadioSignalTransmission
{
    protected:
        static int nextId;

    protected:
        const int id;
        const IRadio *transmitter;
        const eventnumber_t eventNumber;
        const simtime_t startTime;
        const simtime_t endTime;
        const Coord startPosition;
        // TODO: FIXME: we don't know the end position at the time of the transmission begin, we can only have a guess
        // TODO: should we separate transmission begin and end? or should it be an approximation only?
        const Coord endPosition;
        const mps propagationSpeed;

    public:
        RadioSignalTransmissionBase(const IRadio *transmitter, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) :
            id(nextId++),
            transmitter(transmitter),
            eventNumber(simulation.getEventNumber()),
            startTime(startTime),
            endTime(endTime),
            startPosition(startPosition),
            endPosition(endPosition),
            propagationSpeed(mps(SPEED_OF_LIGHT))
        {}

        virtual int getId() const { return id; }

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadio *getTransmitter() const { return transmitter; }

        virtual const simtime_t getStartTime() const { return startTime; }
        virtual const simtime_t getEndTime() const { return endTime; }
        virtual const simtime_t getDuration() const { return endTime - startTime; }

        virtual const Coord getStartPosition() const { return startPosition; }
        virtual const Coord getEndPosition() const { return endPosition; }
        virtual mps getPropagationSpeed() const { return propagationSpeed; }

        // KLUDGE: delete
        virtual eventnumber_t getEventNumber() const { return eventNumber; }
};

class INET_API RadioSignalArrival : public virtual IRadioSignalArrival
{
    protected:
        const simtime_t startPropagationTime;
        const simtime_t endPropagationTime;
        const simtime_t startTime;
        const simtime_t endTime;
        const Coord startPosition;
        const Coord endPosition;

    public:
        RadioSignalArrival(const simtime_t startPropagationTime, const simtime_t endPropagationTime, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) :
            startPropagationTime(startPropagationTime),
            endPropagationTime(endPropagationTime),
            startTime(startTime),
            endTime(endTime),
            startPosition(startPosition),
            endPosition(endPosition)
        {}

        virtual void printToStream(std::ostream &stream) const {}

        virtual const simtime_t getStartPropagationTime() const { return startPropagationTime; }
        virtual const simtime_t getEndPropagationTime() const { return endPropagationTime; }

        virtual const simtime_t getStartTime() const { return startTime; }
        virtual const simtime_t getEndTime() const { return endTime; }
        virtual const simtime_t getDuration() const { return endTime - startTime; }

        virtual const Coord getStartPosition() const { return startPosition; }
        virtual const Coord getEndPosition() const { return endPosition; }
};

class INET_API RadioSignalListeningBase : public IRadioSignalListening
{
    protected:
        const IRadio *receiver;
        const simtime_t startTime;
        const simtime_t endTime;
        const Coord startPosition;
        const Coord endPosition;

    public:
        RadioSignalListeningBase(const IRadio *receiver, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) :
            receiver(receiver),
            startTime(startTime),
            endTime(endTime),
            startPosition(startPosition),
            endPosition(endPosition)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadio *getReceiver() const { return receiver; }

        virtual const simtime_t getStartTime() const { return startTime; }
        virtual const simtime_t getEndTime() const { return endTime; }
        virtual const simtime_t getDuration() const { return endTime - startTime; }

        virtual const Coord getStartPosition() const { return startPosition; }
        virtual const Coord getEndPosition() const { return endPosition; }
};

class INET_API RadioSignalReceptionBase : public virtual IRadioSignalReception
{
    protected:
        const IRadio *receiver;
        const IRadioSignalTransmission *transmission;
        const simtime_t startTime;
        const simtime_t endTime;
        const Coord startPosition;
        const Coord endPosition;

    public:
        RadioSignalReceptionBase(const IRadio *receiver, const IRadioSignalTransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) :
            receiver(receiver),
            transmission(transmission),
            startTime(startTime),
            endTime(endTime),
            startPosition(startPosition),
            endPosition(endPosition)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadio *getReceiver() const { return receiver; }
        virtual const IRadioSignalTransmission *getTransmission() const { return transmission; }

        virtual const simtime_t getStartTime() const { return startTime; }
        virtual const simtime_t getEndTime() const { return endTime; }
        virtual const simtime_t getDuration() const { return endTime - startTime; }

        virtual const Coord getStartPosition() const { return startPosition; }
        virtual const Coord getEndPosition() const { return endPosition; }
};

class INET_API RadioSignalNoiseBase : public IRadioSignalNoise
{
    protected:
        const simtime_t startTime;
        const simtime_t endTime;

    public:
        RadioSignalNoiseBase(simtime_t startTime, simtime_t endTime) :
            startTime(startTime),
            endTime(endTime)
        {}

        virtual const simtime_t getStartTime() const { return startTime; }
        virtual const simtime_t getEndTime() const { return endTime; }
        virtual const simtime_t getDuration() const { return endTime - startTime; }
};

class INET_API RadioAntennaBase : public IRadioAntenna, public cCompoundModule
{
    protected:
        IMobility *mobility;

    protected:
        virtual void initialize(int stage);

    public:
        RadioAntennaBase() :
            mobility(NULL)
        {}

        RadioAntennaBase(IMobility *mobility) :
            mobility(mobility)
        {}

        virtual IMobility *getMobility() const { return mobility; }
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

        virtual double getGain(Coord direction) const { return gain; }
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

        virtual double getGain(Coord direction) const { return 1; }
};

class INET_API DipoleRadioAntenna : public RadioAntennaBase
{
    protected:
        const m length;

    public:
        DipoleRadioAntenna(IMobility *mobility, m length) :
            RadioAntennaBase(mobility),
            length(length)
        {}

        virtual m getLength() const { return length; }
        virtual double getGain(Coord direction) const { return 1; }
};

class INET_API RadioSignalFreeSpaceAttenuationBase : public virtual IRadioSignalAttenuation, public cCompoundModule
{
    protected:
        double alpha;

    protected:
        virtual void initialize(int stage);

        virtual double computePathLoss(const IRadioSignalTransmission *transmission, simtime_t receptionStartTime, simtime_t receptionEndTime, Coord receptionStartPosition, Coord receptionEndPosition, Hz carrierFrequency) const;

    public:
        RadioSignalFreeSpaceAttenuationBase() :
            alpha(sNaN)
        {}

        RadioSignalFreeSpaceAttenuationBase(double alpha) :
            alpha(alpha)
        {}

        virtual double getAlpha() const { return alpha; }
};

class INET_API CompoundAttenuationBase : public IRadioSignalAttenuation
{
    protected:
        const std::vector<const IRadioSignalAttenuation *> *elements;

    public:
        CompoundAttenuationBase(const std::vector<const IRadioSignalAttenuation *> *elements) :
            elements(elements)
        {}
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
        const bool isReceptionPossible_;
        const bool isReceptionAttempted_;
        const bool isReceptionSuccessful_;
        double snir;

    public:
        RadioSignalReceptionDecision(const IRadioSignalReception *reception, bool isReceptionPossible, bool isReceptionSuccessful, double snir) :
            reception(reception),
            isReceptionPossible_(isReceptionPossible),
            isReceptionAttempted_(false),
            isReceptionSuccessful_(isReceptionSuccessful),
            snir(snir)
        {}

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioSignalReception *getReception() const { return reception; }

        virtual bool isReceptionPossible() const { return isReceptionPossible_; }

        virtual bool isReceptionAttempted() const { return isReceptionAttempted_; }

        virtual bool isReceptionSuccessful() const { return isReceptionSuccessful_; }

        virtual bool isPacketErrorless() const { return isReceptionSuccessful_; }

        virtual int getBitErrorCount() const { return -1; }

        virtual int getSymbolErrorCount() const { return -1; }

        virtual double getPER() const { return NaN; }

        virtual double getBER() const { return NaN; }

        virtual double getSER() const { return NaN; }

        virtual const W getRSSI() const { return W(NaN); }

        virtual double getSNIR() const { return snir; }
};

class INET_API RadioSignalReceiverBase : public cCompoundModule, public virtual IRadioSignalReceiver
{
    protected:
        virtual bool computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const;

    public:
        RadioSignalReceiverBase()
        {}
};

class INET_API SNIRRadioSignalReceiverBase : public RadioSignalReceiverBase
{
    protected:
        double snirThreshold;

    protected:
        virtual void initialize(int stage);

        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *receptions, const IRadioSignalNoise *backgroundNoise) const = 0;
        virtual double computeSNIRMin(const IRadioSignalReception *reception, const IRadioSignalNoise *noise) const = 0;

    public:
        SNIRRadioSignalReceiverBase() :
            RadioSignalReceiverBase(),
            snirThreshold(sNaN)
        {}

        SNIRRadioSignalReceiverBase(double snirThreshold) :
            RadioSignalReceiverBase(),
            snirThreshold(snirThreshold)
        {}

        virtual double getSNIRThreshold() const { return snirThreshold; }
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;
};

class INET_API ImmediateRadioSignalPropagation : public cCompoundModule, public IRadioSignalPropagation
{
    public:
        virtual mps getPropagationSpeed() const { return mps(POSITIVE_INFINITY); }

        virtual const IRadioSignalArrival *computeArrival(const IRadioSignalTransmission *transmission, IMobility *mobility) const;
};

class INET_API ConstantSpeedRadioSignalPropagation : public cCompoundModule, public IRadioSignalPropagation
{
    protected:
        const mps propagationSpeed;
        const int mobilityApproximationCount;
        mutable long arrivalComputationCount;

    protected:
        virtual void finish();

        virtual const Coord computeArrivalPosition(const simtime_t startTime, const Coord startPosition, IMobility *mobility) const;

    public:
        ConstantSpeedRadioSignalPropagation() :
            propagationSpeed(SPEED_OF_LIGHT),
            mobilityApproximationCount(0),
            arrivalComputationCount(0)
        {}

        ConstantSpeedRadioSignalPropagation(mps propagationSpeed, int mobilityApproximationCount) :
            propagationSpeed(propagationSpeed),
            mobilityApproximationCount(mobilityApproximationCount),
            arrivalComputationCount(0)
        {}

        virtual mps getPropagationSpeed() const { return propagationSpeed; }

        virtual const IRadioSignalArrival *computeArrival(const IRadioSignalTransmission *transmission, IMobility *mobility) const;
};

#endif
