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
        std::vector<cMessage *> endReceptionTimers;

    protected:
        virtual void initialize(int stage);

        virtual void handleMessageWhenUp(cMessage *message);
        virtual void handleSelfMessage(cMessage *message);
        virtual void handleUpperCommand(cMessage *command);
        virtual void handleLowerCommand(cMessage *command);
        virtual void handleUpperFrame(cPacket *frame);
        virtual void handleLowerFrame(RadioFrame *radioFrame);
        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

        virtual void cancelAndDeleteEndReceptionTimers();
        virtual void updateTransceiverState();

    public:
        Radio() :
            id(nextId++),
            antenna(NULL),
            transmitter(NULL),
            receiver(NULL),
            channel(NULL),
            endTransmissionTimer(NULL),
            endReceptionTimer(NULL)
        {}

        Radio(RadioMode radioMode, const IRadioAntenna *antenna, const IRadioSignalTransmitter *transmitter, const IRadioSignalReceiver *receiver, IRadioChannel *channel) :
            id(nextId++),
            antenna(antenna),
            transmitter(transmitter),
            receiver(receiver),
            channel(channel),
            endTransmissionTimer(NULL),
            endReceptionTimer(NULL)
        {
            channel->addRadio(this);
        }

        virtual ~Radio();

        virtual int getId() const { return id; }

        virtual void printToStream(std::ostream &stream) const;

        virtual const IRadioAntenna *getAntenna() const { return antenna; }
        virtual const IRadioSignalTransmitter *getTransmitter() const { return transmitter; }
        virtual const IRadioSignalReceiver *getReceiver() const { return receiver; }
        virtual const IRadioChannel *getChannel() const { return channel; }

        virtual IRadioFrame *transmitPacket(cPacket *packet, const simtime_t startTime);
        virtual cPacket *receivePacket(IRadioFrame *frame);


        virtual void setRadioMode(RadioMode radioMode);
        virtual void setOldRadioChannel(int newRadioChannel);

        virtual const IRadioSignalTransmission *getTransmissionInProgress() const;
        virtual const IRadioSignalTransmission *getReceptionInProgress() const;
};

#endif
