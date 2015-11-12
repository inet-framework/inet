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

#include "inet/physicallayer/energyconsumer/StateBasedEnergyConsumer.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

namespace physicallayer {

Define_Module(StateBasedEnergyConsumer);

StateBasedEnergyConsumer::StateBasedEnergyConsumer() :
    offPowerConsumption(W(NaN)),
    sleepPowerConsumption(W(NaN)),
    switchingPowerConsumption(W(NaN)),
    receiverIdlePowerConsumption(W(NaN)),
    receiverBusyPowerConsumption(W(NaN)),
    receiverReceivingPowerConsumption(W(NaN)),
    receiverReceivingPreamblePowerConsumption(W(NaN)),
    receiverReceivingHeaderPowerConsumption(W(NaN)),
    receiverReceivingDataPowerConsumption(W(NaN)),
    transmitterIdlePowerConsumption(W(NaN)),
    transmitterTransmittingPowerConsumption(W(NaN)),
    transmitterTransmittingPreamblePowerConsumption(W(NaN)),
    transmitterTransmittingHeaderPowerConsumption(W(NaN)),
    transmitterTransmittingDataPowerConsumption(W(NaN)),
    radio(nullptr),
    energySource(nullptr),
    energyConsumerId(-1)
{
}

void StateBasedEnergyConsumer::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV << "Initializing StateBasedEnergyConsumer, stage = " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        offPowerConsumption = W(par("offPowerConsumption"));
        sleepPowerConsumption = W(par("sleepPowerConsumption"));
        switchingPowerConsumption = W(par("switchingPowerConsumption"));
        receiverIdlePowerConsumption = W(par("receiverIdlePowerConsumption"));
        receiverBusyPowerConsumption = W(par("receiverBusyPowerConsumption"));
        receiverReceivingPowerConsumption = W(par("receiverReceivingPowerConsumption"));
        receiverReceivingPreamblePowerConsumption = W(par("receiverReceivingPreamblePowerConsumption"));
        receiverReceivingHeaderPowerConsumption = W(par("receiverReceivingHeaderPowerConsumption"));
        receiverReceivingDataPowerConsumption = W(par("receiverReceivingDataPowerConsumption"));
        transmitterIdlePowerConsumption = W(par("transmitterIdlePowerConsumption"));
        transmitterTransmittingPowerConsumption = W(par("transmitterTransmittingPowerConsumption"));
        transmitterTransmittingPreamblePowerConsumption = W(par("transmitterTransmittingPreamblePowerConsumption"));
        transmitterTransmittingHeaderPowerConsumption = W(par("transmitterTransmittingHeaderPowerConsumption"));
        transmitterTransmittingDataPowerConsumption = W(par("transmitterTransmittingDataPowerConsumption"));
        cModule *radioModule = getParentModule();
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(IRadio::receivedSignalPartChangedSignal, this);
        radioModule->subscribe(IRadio::transmittedSignalPartChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
        const char *energySourceModule = par("energySourceModule");
        energySource = dynamic_cast<IEnergySource *>(getModuleByPath(energySourceModule));
        if (!energySource)
            throw cRuntimeError("Cannot find power source");
        energyConsumerId = energySource->addEnergyConsumer(this);
    }
}

void StateBasedEnergyConsumer::receiveSignal(cComponent *source, simsignal_t signalID, long value DETAILS_ARG)
{
    energySource->setPowerConsumption(energyConsumerId, getPowerConsumption());
}

W StateBasedEnergyConsumer::getPowerConsumption() const
{
    IRadio::RadioMode radioMode = radio->getRadioMode();
    if (radioMode == IRadio::RADIO_MODE_OFF)
        return offPowerConsumption;
    else if (radioMode == IRadio::RADIO_MODE_SLEEP)
        return sleepPowerConsumption;
    else if (radioMode == IRadio::RADIO_MODE_SWITCHING)
        return switchingPowerConsumption;
    W powerConsumption = W(0);
    IRadio::ReceptionState receptionState = radio->getReceptionState();
    IRadio::TransmissionState transmissionState = radio->getTransmissionState();
    if (radioMode == IRadio::RADIO_MODE_RECEIVER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (receptionState == IRadio::RECEPTION_STATE_IDLE)
            powerConsumption += receiverIdlePowerConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_BUSY)
            powerConsumption += receiverBusyPowerConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_RECEIVING) {
            auto part = radio->getReceivedSignalPart();
            if (part == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (part == IRadioSignal::SIGNAL_PART_WHOLE)
                powerConsumption += receiverReceivingPowerConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_PREAMBLE)
                powerConsumption += receiverReceivingPreamblePowerConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_HEADER)
                powerConsumption += receiverReceivingHeaderPowerConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_DATA)
                powerConsumption += receiverReceivingDataPowerConsumption;
            else
                throw cRuntimeError("Unknown received signal part");
        }
        else if (receptionState != IRadio::RECEPTION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio reception state");
    }
    if (radioMode == IRadio::RADIO_MODE_TRANSMITTER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (transmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            powerConsumption += transmitterIdlePowerConsumption;
        else if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
            // TODO: add transmission power?
            auto part = radio->getTransmittedSignalPart();
            if (part == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (part == IRadioSignal::SIGNAL_PART_WHOLE)
                powerConsumption += transmitterTransmittingPowerConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_PREAMBLE)
                powerConsumption += transmitterTransmittingPreamblePowerConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_HEADER)
                powerConsumption += transmitterTransmittingHeaderPowerConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_DATA)
                powerConsumption += transmitterTransmittingDataPowerConsumption;
            else
                throw cRuntimeError("Unknown transmitted signal part");
        }
        else if (transmissionState != IRadio::TRANSMISSION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio transmission state");
    }
    return powerConsumption;
}

} // namespace physicallayer

} // namespace inet

