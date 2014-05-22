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
    ScalarRadio(),
    channelNumber(-1)
{
}

Ieee80211Radio::Ieee80211Radio(RadioMode radioMode, const IRadioAntenna *antenna, const IRadioSignalTransmitter *transmitter, const IRadioSignalReceiver *receiver, IRadioChannel *channel) :
    ScalarRadio(radioMode, antenna, transmitter, receiver, channel),
    channelNumber(-1)
{
}

void Ieee80211Radio::initialize(int stage)
{
    ScalarRadio::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_LAYER)
        setChannelNumber(par("channelNumber"));
}

void Ieee80211Radio::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE)
    {
        RadioConfigureCommand *configureCommand = check_and_cast<RadioConfigureCommand *>(message->getControlInfo());
        int newChannelNumber = configureCommand->getChannelNumber();
        if (newChannelNumber != -1)
            setChannelNumber(newChannelNumber);
        ScalarRadio::handleUpperCommand(message);
    }
    else
        ScalarRadio::handleUpperCommand(message);
}

void Ieee80211Radio::setChannelNumber(int newChannelNumber)
{
    if (channelNumber != newChannelNumber)
    {
        Hz carrierFrequency = Hz(CENTER_FREQUENCIES[newChannelNumber + 1]);
        ScalarRadioSignalTransmitter *scalarTransmitter = const_cast<ScalarRadioSignalTransmitter *>(check_and_cast<const ScalarRadioSignalTransmitter *>(transmitter));
        ScalarRadioSignalReceiver *scalarReceiver = const_cast<ScalarRadioSignalReceiver *>(check_and_cast<const ScalarRadioSignalReceiver *>(receiver));
        scalarTransmitter->setCarrierFrequency(carrierFrequency);
        scalarReceiver->setCarrierFrequency(carrierFrequency);
        EV << "Changing radio channel from " << channelNumber << " to " << newChannelNumber << ".\n";
        channelNumber = newChannelNumber;
        emit(radioChannelChangedSignal, newChannelNumber);
        emit(listeningChangedSignal, 0);
    }
}
