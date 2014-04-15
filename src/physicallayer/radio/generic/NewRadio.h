//
// Copyright (C) 2013 OpenSim Ltd
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

#ifndef __INET_NEWRADIO_H_
#define __INET_NEWRADIO_H_

#include "Radio.h"

// TODO: allow changing channel during reception
class INET_API NewRadio : public Radio
{
    protected:
        cMessage *endTransmissionTimer;
        cMessage *endReceptionTimer;
        typedef std::vector<cMessage *> EndReceptionTimers;
        EndReceptionTimers endReceptionTimers;

    public:
        NewRadio();
        virtual ~NewRadio();

        virtual void setRadioMode(RadioMode radioMode);

        virtual void setOldRadioChannel(int newRadioChannel);

        virtual const IRadioSignalTransmission *getTransmissionInProgress() const;

        virtual const IRadioSignalTransmission *getReceptionInProgress() const;

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
};

#endif
