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
#include "IRadioChannel.h"

class INET_API RadioSignalTransmissionBase : public virtual IRadioSignalTransmission
{
    protected:
        static int nextId;

    protected:
        const int id;
        const IRadio *transmitter;
        const cPacket *macFrame;
        const simtime_t startTime;
        const simtime_t endTime;
        const Coord startPosition;
        const Coord endPosition;

    public:
        RadioSignalTransmissionBase(const IRadio *transmitter, const cPacket *macFrame, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) :
            id(nextId++),
            transmitter(transmitter),
            macFrame(macFrame),
            startTime(startTime),
            endTime(endTime),
            startPosition(startPosition),
            endPosition(endPosition)
        {}

        virtual int getId() const { return id; }

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadio *getTransmitter() const { return transmitter; }
        virtual const cPacket *getMacFrame() const { return macFrame; }

        virtual const simtime_t getStartTime() const { return startTime; }
        virtual const simtime_t getEndTime() const { return endTime; }

        virtual const Coord getStartPosition() const { return startPosition; }
        virtual const Coord getEndPosition() const { return endPosition; }
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

        virtual double getMaxGain() const { return gain; }

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

        virtual double getMaxGain() const { return 1; }

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

        virtual double getMaxGain() const { return 1.5; }

        // TODO: compute gain based on positions/orientations
        virtual double getGain(Coord direction) const { return 1.5; }
};

class INET_API InterpolatingRadioAntenna : public RadioAntennaBase
{
    // TODO: compute antenna gain based on a linear interpolation between two elements of the antenna gain table using the antenna positions/orientations
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

class INET_API RadioSignalTransmitterBase : public cCompoundModule, public virtual IRadioSignalTransmitter
{
    public:
        virtual W getMaxPower() const { return W(qNaN); }
};

class INET_API RadioSignalReceiverBase : public cCompoundModule, public virtual IRadioSignalReceiver
{
    protected:
        virtual bool computeIsReceptionPossible(const IRadioSignalTransmission *transmission) const;
        virtual bool computeIsReceptionPossible(const IRadioSignalReception *reception) const = 0;
        virtual bool computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const;

    public:
        RadioSignalReceiverBase() {}

        virtual W getMinInterferencePower() const { return W(0); }
        virtual W getMinReceptionPower() const { return W(0); }
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

class INET_API RadioSignalPropagationBase : public cCompoundModule, public IRadioSignalPropagation
{
    protected:
        mps propagationSpeed;
        mutable long arrivalComputationCount;

    protected:
        virtual void initialize(int stage);
        virtual void finish();

    public:
        RadioSignalPropagationBase();
        RadioSignalPropagationBase(mps propagationSpeed);

        virtual mps getPropagationSpeed() const { return propagationSpeed; }
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
