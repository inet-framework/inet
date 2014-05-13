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

#ifndef __INET_RADIO_H_
#define __INET_RADIO_H_

#include "IRadioChannel.h"
#include "IRadioAntenna.h"
#include "RadioBase.h"
#include "RadioFrame.h"

// TODO: merge with RadioBase
// TODO: do we really need to support interference-free simultaneous reception of multiple transmissions?
// TODO: if we don't send all radio frames to all radios how will they update their reception state?
// TODO: in case of a listening state change (e.g. radio state, carrier frequency) what about the ongoing transmissions?
// TODO: there are a few things that can affect how we send/receive transmissions:
// TODO: 1. we may send transmissions to all receivers or to potential receivers only
// TODO:    receivers may be filtered based on their distance/reception state/listening state
// TODO: 2. we may want to add/remove receptions for ongoing transmissions based on the same criteria
// TODO: 3. we may also directly call receivers at reception starts/ends from the radio channel
class INET_API Radio : public RadioBase, public virtual IRadio
{
    protected:
        static int nextId;

    protected:
        const int id;
        const IRadioAntenna *antenna;
        const IRadioSignalTransmitter *transmitter;
        const IRadioSignalReceiver *receiver;
        IRadioChannel *channel;

        cMessage *endTransmissionTimer;
        cMessage *endReceptionTimer;

        simtime_t lastReceptionStateChange;

    protected:
        virtual void initialize(int stage);

        virtual void handleMessageWhenUp(cMessage *message);
        virtual void handleSelfMessage(cMessage *message);
        virtual void handleUpperCommand(cMessage *command);
        virtual void handleLowerCommand(cMessage *command);
        virtual void handleUpperFrame(cPacket *macFrame);
        virtual void handleLowerFrame(RadioFrame *radioFrame);
        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

        virtual void startTransmission(cPacket *macFrame);
        virtual void endTransmission();

        virtual void startReception(RadioFrame *radioFrame);
        virtual void endReception(cMessage *message);

        virtual bool isListeningPossible();
        virtual void updateTransceiverState();

    public:
        Radio();
        Radio(RadioMode radioMode, const IRadioAntenna *antenna, const IRadioSignalTransmitter *transmitter, const IRadioSignalReceiver *receiver, IRadioChannel *channel);
        virtual ~Radio();

        virtual int getId() const { return id; }

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioAntenna *getAntenna() const { return antenna; }
        virtual const IRadioSignalTransmitter *getTransmitter() const { return transmitter; }
        virtual const IRadioSignalReceiver *getReceiver() const { return receiver; }
        virtual const IRadioChannel *getChannel() const { return channel; }

        virtual void setRadioMode(RadioMode radioMode);

        virtual const IRadioSignalTransmission *getTransmissionInProgress() const;
        virtual const IRadioSignalTransmission *getReceptionInProgress() const;
};

#endif
