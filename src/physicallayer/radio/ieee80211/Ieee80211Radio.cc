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

#include "Ieee80211Radio.h"
#include "Ieee80211Consts.h"
#include "ScalarImplementation.h"
#include "RadioControlInfo_m.h"

Define_Module(Ieee80211Radio);

Ieee80211Radio::Ieee80211Radio() :
    ScalarRadio()
{
}

Ieee80211Radio::Ieee80211Radio(RadioMode radioMode, const IRadioAntenna *antenna, const IRadioSignalTransmitter *transmitter, const IRadioSignalReceiver *receiver, IRadioChannel *channel) :
    ScalarRadio(radioMode, antenna, transmitter, receiver, channel)
{
}

void Ieee80211Radio::initialize(int stage)
{
    ScalarRadio::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_LAYER)
        setOldRadioChannel(par("channelNumber"));
}

void Ieee80211Radio::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE)
    {
        RadioConfigureCommand *configureCommand = check_and_cast<RadioConfigureCommand *>(message->getControlInfo());
        int newChannelNumber = configureCommand->getChannelNumber();
        if (newChannelNumber != -1)
            setOldRadioChannel(newChannelNumber);
        ScalarRadio::handleUpperCommand(message);
    }
    else
        ScalarRadio::handleUpperCommand(message);
}

void Ieee80211Radio::setOldRadioChannel(int newRadioChannel)
{
    Hz carrierFrequency = Hz(CENTER_FREQUENCIES[newRadioChannel + 1]);
    ScalarRadioSignalTransmitter *scalarTransmitter = const_cast<ScalarRadioSignalTransmitter *>(check_and_cast<const ScalarRadioSignalTransmitter *>(transmitter));
    ScalarRadioSignalReceiver *scalarReceiver = const_cast<ScalarRadioSignalReceiver *>(check_and_cast<const ScalarRadioSignalReceiver *>(receiver));
    scalarTransmitter->setCarrierFrequency(carrierFrequency);
    scalarReceiver->setCarrierFrequency(carrierFrequency);
    Radio::setOldRadioChannel(newRadioChannel);
    emit(listeningChangedSignal, 0);
}
