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

#include "RadioPowerConsumer.h"
#include "ModuleAccess.h"

RadioPowerConsumer::RadioPowerConsumer()
{
    powerConsumerId = 0;
    powerSource = NULL;
    sleepModePowerConsumption = 0;
    receiverModeFreeChannelPowerConsumption = 0;
    receiverModeBusyChannelPowerConsumption = 0;
    receiverModeReceivingPowerConsumption = 0;
    transmitterModeIdlePowerConsumption = 0;
    transmitterModeTransmittingPowerConsumption = 0;
}

void RadioPowerConsumer::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV << "Initializing RadioPowerConsumer, stage = " << stage << endl;
    if (stage == INITSTAGE_LOCAL)
    {
        sleepModePowerConsumption = par("sleepModePowerConsumption");
        receiverModeFreeChannelPowerConsumption = par("receiverModeFreeChannelPowerConsumption");
        receiverModeBusyChannelPowerConsumption = par("receiverModeBusyChannelPowerConsumption");
        receiverModeReceivingPowerConsumption = par("receiverModeReceivingPowerConsumption");
        transmitterModeIdlePowerConsumption = par("transmitterModeIdlePowerConsumption");
        transmitterModeTransmittingPowerConsumption = par("transmitterModeTransmittingPowerConsumption");
        cModule *radioModule = getParentModule()->getSubmodule("radio");
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::radioChannelStateChangedSignal, this);
        radio = check_and_cast<IRadio*>(radioModule);
        cModule *node = findContainingNode(this);
        powerSource = dynamic_cast<IPowerSource *>(node->getSubmodule("powerSource"));
        if (powerSource)
            powerConsumerId = powerSource->addPowerConsumer(this);

    }
}

void RadioPowerConsumer::receiveSignal(cComponent *source, simsignal_t signalID, long value)
{
    if (signalID == IRadio::radioModeChangedSignal || signalID == IRadio::radioChannelStateChangedSignal)
    {
        if (powerSource)
            powerSource->setPowerConsumption(powerConsumerId, getPowerConsumption());
    }
}

double RadioPowerConsumer::getPowerConsumption()
{
    IRadio::RadioMode radioMode = radio->getRadioMode();
    IRadio::RadioChannelState radioChannelState = radio->getRadioChannelState();
    if (radioMode == IRadio::RADIO_MODE_OFF)
        return 0;
    else if (radioMode == IRadio::RADIO_MODE_SLEEP)
        return sleepModePowerConsumption;
    else if (radioMode == IRadio::RADIO_MODE_RECEIVER) {
        if (radioChannelState == IRadio::RADIO_CHANNEL_STATE_FREE)
            return receiverModeFreeChannelPowerConsumption;
        else if (radioChannelState == IRadio::RADIO_CHANNEL_STATE_BUSY)
            return receiverModeBusyChannelPowerConsumption;
        else if (radioChannelState == IRadio::RADIO_CHANNEL_STATE_RECEIVING)
            return receiverModeReceivingPowerConsumption;
        else
            throw cRuntimeError("Unknown radio channel state");
    }
    else if (radioMode == IRadio::RADIO_MODE_TRANSMITTER) {
        if (radioChannelState == IRadio::RADIO_CHANNEL_STATE_FREE)
            return transmitterModeIdlePowerConsumption;
        else if (radioChannelState == IRadio::RADIO_CHANNEL_STATE_TRANSMITTING)
            return transmitterModeTransmittingPowerConsumption;
        else
            throw cRuntimeError("Unknown radio channel state");
    }
    else if (radioMode == IRadio::RADIO_MODE_SWITCHING)
        return 0;
    else
        throw cRuntimeError("Unknown radio mode");
}
