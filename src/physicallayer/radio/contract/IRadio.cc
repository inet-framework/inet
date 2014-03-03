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

#include "IRadio.h"

simsignal_t OldIRadio::radioModeChangedSignal = cComponent::registerSignal("radioModeChanged");
simsignal_t OldIRadio::receptionStateChangedSignal = cComponent::registerSignal("receptionStateChanged");
simsignal_t OldIRadio::transmissionStateChangedSignal = cComponent::registerSignal("transmissionStateChanged");
simsignal_t OldIRadio::radioChannelChangedSignal = cComponent::registerSignal("radioChannelChanged");

cEnum *OldIRadio::radioModeEnum = NULL;
cEnum *OldIRadio::receptionStateEnum = NULL;
cEnum *OldIRadio::transmissionStateEnum = NULL;

Register_Enum(RadioMode,
              (OldIRadio::RADIO_MODE_OFF,
               OldIRadio::RADIO_MODE_SLEEP,
               OldIRadio::RADIO_MODE_RECEIVER,
               OldIRadio::RADIO_MODE_TRANSMITTER,
               OldIRadio::RADIO_MODE_TRANSCEIVER,
               OldIRadio::RADIO_MODE_SWITCHING));

Register_Enum(ReceptionState,
              (OldIRadio::RECEPTION_STATE_UNDEFINED,
               OldIRadio::RECEPTION_STATE_IDLE,
               OldIRadio::RECEPTION_STATE_BUSY,
               OldIRadio::RECEPTION_STATE_SYNCHRONIZING,
               OldIRadio::RECEPTION_STATE_RECEIVING));

Register_Enum(TransmissionState,
              (OldIRadio::TRANSMISSION_STATE_UNDEFINED,
               OldIRadio::TRANSMISSION_STATE_IDLE,
               OldIRadio::TRANSMISSION_STATE_TRANSMITTING));

const char *OldIRadio::getRadioModeName(RadioMode radioMode)
{
    if (!radioModeEnum)
        radioModeEnum = cEnum::get("RadioMode");
    return radioModeEnum->getStringFor(radioMode) + 11;
}

const char *OldIRadio::getRadioReceptionStateName(ReceptionState receptionState)
{
    if (!receptionStateEnum)
        receptionStateEnum = cEnum::get("ReceptionState");
    return receptionStateEnum->getStringFor(receptionState) + 16;
}

const char *OldIRadio::getRadioTransmissionStateName(TransmissionState transmissionState)
{
    if (!transmissionStateEnum)
        transmissionStateEnum = cEnum::get("TransmissionState");
    return transmissionStateEnum->getStringFor(transmissionState) + 19;
}
