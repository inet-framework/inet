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

#include "IRadioBackgroundNoise.h"
#include "IRadioSignalArrival.h"
#include "IRadioSignalAttenuation.h"
#include "IRadioSignalPropagation.h"
#include "IRadioSignalTransmitter.h"
#include "IRadioSignalReceiver.h"
#include "IRadioChannel.h"

// TODO: the current unique id generation for transmissions prevents other radio implementations that do not subclass from this base class
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

class INET_API RadioSignalTransmitterBase : public cCompoundModule, public virtual IRadioSignalTransmitter
{
    public:
        virtual W getMaxPower() const { return W(qNaN); }
};

class INET_API RadioSignalReceiverBase : public cCompoundModule, public virtual IRadioSignalReceiver
{
    protected:
        virtual bool computeIsReceptionPossible(const IRadioSignalTransmission *transmission) const;

        /**
         * Returns whether the transmission represented by the reception can be
         * received successfully or not. Part of the reception process, see class comment.
         */
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

        /**
         * Returns the physical properties of the reception including noise and
         * signal related measures, error probabilities, actual error counts, etc.
         * Part of the reception process, see class comment.
         */
        virtual const RadioReceptionIndication *computeReceptionIndication(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;

        /**
         * Returns whether the reception is free of any errors. Part of the reception
         * process, see class comment.
         */
        virtual bool computeIsReceptionSuccessful(const IRadioSignalReception *reception, const RadioReceptionIndication *indication) const;

        virtual const IRadioSignalNoise *computeNoise(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *receptions, const IRadioSignalNoise *backgroundNoise) const = 0;
        virtual double computeMinSNIR(const IRadioSignalReception *reception, const IRadioSignalNoise *noise) const = 0;

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

#endif
