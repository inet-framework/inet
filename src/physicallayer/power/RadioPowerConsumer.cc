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

namespace inet {

namespace physicallayer {

RadioPowerConsumer::RadioPowerConsumer()
{
    powerConsumerId = 0;
    powerSource = NULL;
    offPowerConsumption = 0;
    sleepPowerConsumption = 0;
    switchingPowerConsumption = 0;
    receiverIdlePowerConsumption = 0;
    receiverBusyPowerConsumption = 0;
    receiverSynchronizingPowerConsumption = 0;
    receiverReceivingPowerConsumption = 0;
    transmitterIdlePowerConsumption = 0;
    transmitterTransmittingPowerConsumption = 0;
}

void RadioPowerConsumer::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV << "Initializing RadioPowerConsumer, stage = " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        offPowerConsumption = par("offPowerConsumption");
        sleepPowerConsumption = par("sleepPowerConsumption");
        switchingPowerConsumption = par("switchingPowerConsumption");
        receiverIdlePowerConsumption = par("receiverIdlePowerConsumption");
        receiverBusyPowerConsumption = par("receiverBusyPowerConsumption");
        receiverSynchronizingPowerConsumption = par("receiverSynchronizingPowerConsumption");
        receiverReceivingPowerConsumption = par("receiverReceivingPowerConsumption");
        transmitterIdlePowerConsumption = par("transmitterIdlePowerConsumption");
        transmitterTransmittingPowerConsumption = par("transmitterTransmittingPowerConsumption");
        cModule *radioModule = getParentModule()->getSubmodule("radio");
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
        cModule *node = findContainingNode(this);
        powerSource = dynamic_cast<IPowerSource *>(node->getSubmodule("powerSource"));
        if (powerSource)
            powerConsumerId = powerSource->addPowerConsumer(this);
    }
}

void RadioPowerConsumer::receiveSignal(cComponent *source, simsignal_t signalID, long value)
{
    if (signalID == IRadio::radioModeChangedSignal || signalID == IRadio::receptionStateChangedSignal || signalID == IRadio::transmissionStateChangedSignal) {
        if (powerSource)
            powerSource->setPowerConsumption(powerConsumerId, getPowerConsumption());
    }
}

double RadioPowerConsumer::getPowerConsumption()
{
    IRadio::RadioMode radioMode = radio->getRadioMode();
    if (radioMode == IRadio::RADIO_MODE_OFF)
        return offPowerConsumption;
    else if (radioMode == IRadio::RADIO_MODE_SLEEP)
        return sleepPowerConsumption;
    else if (radioMode == IRadio::RADIO_MODE_SWITCHING)
        return switchingPowerConsumption;
    double powerConsumption = 0;
    IRadio::ReceptionState receptionState = radio->getReceptionState();
    IRadio::TransmissionState transmissionState = radio->getTransmissionState();
    if (radioMode == IRadio::RADIO_MODE_RECEIVER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (receptionState == IRadio::RECEPTION_STATE_IDLE)
            powerConsumption += receiverIdlePowerConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_BUSY)
            powerConsumption += receiverBusyPowerConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_SYNCHRONIZING)
            powerConsumption += receiverSynchronizingPowerConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_RECEIVING)
            powerConsumption += receiverReceivingPowerConsumption;
        else
            throw cRuntimeError("Unknown radio reception state");
    }
    if (radioMode == IRadio::RADIO_MODE_TRANSMITTER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (transmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            powerConsumption += transmitterIdlePowerConsumption;
        else if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING)
            powerConsumption += transmitterTransmittingPowerConsumption;
        else
            throw cRuntimeError("Unknown radio transmission state");
    }
    return powerConsumption;
}

} // namespace physicallayer

} // namespace inet

