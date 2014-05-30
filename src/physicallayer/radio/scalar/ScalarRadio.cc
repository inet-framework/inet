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

#include "ScalarRadio.h"
#include "ScalarImplementation.h"
#include "RadioControlInfo_m.h"

Define_Module(ScalarRadio);

ScalarRadio::ScalarRadio() :
    Radio()
{
}

ScalarRadio::ScalarRadio(RadioMode radioMode, const IRadioAntenna *antenna, const IRadioSignalTransmitter *transmitter, const IRadioSignalReceiver *receiver, IRadioChannel *channel) :
    Radio(radioMode, antenna, transmitter, receiver, channel)
{
}

void ScalarRadio::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE)
    {
        RadioConfigureCommand *configureCommand = check_and_cast<RadioConfigureCommand *>(message->getControlInfo());
        bps newBitrate = configureCommand->getBitrate();
        if (!isNaN(newBitrate.get())) {
            ScalarRadioSignalTransmitter *scalarTransmitter = const_cast<ScalarRadioSignalTransmitter *>(check_and_cast<const ScalarRadioSignalTransmitter *>(transmitter));
            scalarTransmitter->setBitrate(newBitrate);
        }
        delete message;
    }
    else
        Radio::handleUpperCommand(message);
}
