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

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/energyconsumer/StateBasedCcEnergyConsumer.h"

namespace inet {

namespace physicallayer {

using namespace inet::power;

Define_Module(StateBasedCcEnergyConsumer);

void StateBasedCcEnergyConsumer::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minSupplyVoltage = V(par("minSupplyVoltage"));
        maxSupplyVoltage = V(par("maxSupplyVoltage"));
        offCurrentConsumption = A(par("offCurrentConsumption"));
        sleepCurrentConsumption = A(par("sleepCurrentConsumption"));
        switchingCurrentConsumption = A(par("switchingCurrentConsumption"));
        receiverIdleCurrentConsumption = A(par("receiverIdleCurrentConsumption"));
        receiverBusyCurrentConsumption = A(par("receiverBusyCurrentConsumption"));
        receiverReceivingCurrentConsumption = A(par("receiverReceivingCurrentConsumption"));
        receiverReceivingPreambleCurrentConsumption = A(par("receiverReceivingPreambleCurrentConsumption"));
        receiverReceivingHeaderCurrentConsumption = A(par("receiverReceivingHeaderCurrentConsumption"));
        receiverReceivingDataCurrentConsumption = A(par("receiverReceivingDataCurrentConsumption"));
        transmitterIdleCurrentConsumption = A(par("transmitterIdleCurrentConsumption"));
        transmitterTransmittingCurrentConsumption = A(par("transmitterTransmittingCurrentConsumption"));
        transmitterTransmittingPreambleCurrentConsumption = A(par("transmitterTransmittingPreambleCurrentConsumption"));
        transmitterTransmittingHeaderCurrentConsumption = A(par("transmitterTransmittingHeaderCurrentConsumption"));
        transmitterTransmittingDataCurrentConsumption = A(par("transmitterTransmittingDataCurrentConsumption"));
        cModule *radioModule = getParentModule();
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(IRadio::receivedSignalPartChangedSignal, this);
        radioModule->subscribe(IRadio::transmittedSignalPartChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
        const char *energySourceModule = par("energySourceModule");
        energySource = check_and_cast<ICcEnergySource *>(getModuleByPath(energySourceModule));
        check_and_cast<cModule *>(energySource)->subscribe(ICcEnergySource::currentConsumptionChangedSignal, this);
        currentConsumption = A(0);
        WATCH(currentConsumption);
    }
    else if (stage == INITSTAGE_POWER)
        energySource->addEnergyConsumer(this);
}

void StateBasedCcEnergyConsumer::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details)
{
    if (signal == IRadio::radioModeChangedSignal ||
        signal == IRadio::receptionStateChangedSignal ||
        signal == IRadio::transmissionStateChangedSignal ||
        signal == IRadio::receivedSignalPartChangedSignal ||
        signal == IRadio::transmittedSignalPartChangedSignal)
    {
        currentConsumption = computeCurrentConsumption();
        emit(currentConsumptionChangedSignal, currentConsumption.get());
    }
    else
        throw cRuntimeError("Unknown signal");
}

void StateBasedCcEnergyConsumer::receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details)
{
    if (signal == ICcEnergySource::currentConsumptionChangedSignal) {
        if (energySource->getOutputVoltage() < minSupplyVoltage)
            radio->setRadioMode(IRadio::RADIO_MODE_OFF);
    }
    else
        throw cRuntimeError("Unknown signal");
}

A StateBasedCcEnergyConsumer::computeCurrentConsumption() const
{
    IRadio::RadioMode radioMode = radio->getRadioMode();
    if (radioMode == IRadio::RADIO_MODE_OFF)
        return offCurrentConsumption;
    else if (radioMode == IRadio::RADIO_MODE_SLEEP)
        return sleepCurrentConsumption;
    else if (radioMode == IRadio::RADIO_MODE_SWITCHING)
        return switchingCurrentConsumption;
    A currentConsumption = A(0);
    IRadio::ReceptionState receptionState = radio->getReceptionState();
    IRadio::TransmissionState transmissionState = radio->getTransmissionState();
    if (radioMode == IRadio::RADIO_MODE_RECEIVER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (receptionState == IRadio::RECEPTION_STATE_IDLE)
            currentConsumption += receiverIdleCurrentConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_BUSY)
            currentConsumption += receiverBusyCurrentConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_RECEIVING) {
            auto part = radio->getReceivedSignalPart();
            if (part == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (part == IRadioSignal::SIGNAL_PART_WHOLE)
                currentConsumption += receiverReceivingCurrentConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_PREAMBLE)
                currentConsumption += receiverReceivingPreambleCurrentConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_HEADER)
                currentConsumption += receiverReceivingHeaderCurrentConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_DATA)
                currentConsumption += receiverReceivingDataCurrentConsumption;
            else
                throw cRuntimeError("Unknown received signal part");
        }
        else if (receptionState != IRadio::RECEPTION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio reception state");
    }
    if (radioMode == IRadio::RADIO_MODE_TRANSMITTER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (transmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            currentConsumption += transmitterIdleCurrentConsumption;
        else if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
            // TODO: add transmission power?
            auto part = radio->getTransmittedSignalPart();
            if (part == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (part == IRadioSignal::SIGNAL_PART_WHOLE)
                currentConsumption += transmitterTransmittingCurrentConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_PREAMBLE)
                currentConsumption += transmitterTransmittingPreambleCurrentConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_HEADER)
                currentConsumption += transmitterTransmittingHeaderCurrentConsumption;
            else if (part == IRadioSignal::SIGNAL_PART_DATA)
                currentConsumption += transmitterTransmittingDataCurrentConsumption;
            else
                throw cRuntimeError("Unknown transmitted signal part");
        }
        else if (transmissionState != IRadio::TRANSMISSION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio transmission state");
    }
    return currentConsumption;
}

} // namespace physicallayer

} // namespace inet

