//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/energyconsumer/StateBasedEpEnergyConsumer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace physicallayer {

using namespace inet::power;

Define_Module(StateBasedEpEnergyConsumer);

void StateBasedEpEnergyConsumer::initialize(int stage)
{
    cSimpleModule::initialize(stage);
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
        powerConsumption = W(0);
        energySource.reference(this, "energySourceModule", true);
        WATCH(powerConsumption);
    }
    else if (stage == INITSTAGE_POWER)
        energySource->addEnergyConsumer(this);
}

void StateBasedEpEnergyConsumer::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == IRadio::radioModeChangedSignal ||
        signal == IRadio::receptionStateChangedSignal ||
        signal == IRadio::transmissionStateChangedSignal ||
        signal == IRadio::receivedSignalPartChangedSignal ||
        signal == IRadio::transmittedSignalPartChangedSignal)
    {
        powerConsumption = computePowerConsumption();
        emit(powerConsumptionChangedSignal, powerConsumption.get());
    }
    else
        throw cRuntimeError("Unknown signal");
}

W StateBasedEpEnergyConsumer::computePowerConsumption() const
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
        switch (receptionState) {
            case IRadio::RECEPTION_STATE_IDLE:
                powerConsumption += receiverIdlePowerConsumption;
                break;
            case IRadio::RECEPTION_STATE_BUSY:
                powerConsumption += receiverBusyPowerConsumption;
                break;
            case IRadio::RECEPTION_STATE_RECEIVING: {
                auto part = radio->getReceivedSignalPart();
                switch (part) {
                    case IRadioSignal::SIGNAL_PART_NONE:
                        break;
                    case IRadioSignal::SIGNAL_PART_WHOLE:
                        powerConsumption += receiverReceivingPowerConsumption;
                        break;
                    case IRadioSignal::SIGNAL_PART_PREAMBLE:
                        powerConsumption += receiverReceivingPreamblePowerConsumption;
                        break;
                    case IRadioSignal::SIGNAL_PART_HEADER:
                        powerConsumption += receiverReceivingHeaderPowerConsumption;
                        break;
                    case IRadioSignal::SIGNAL_PART_DATA:
                        powerConsumption += receiverReceivingDataPowerConsumption;
                        break;
                    default:
                        throw cRuntimeError("Unknown received signal part");
                }
                break;
            }
            case IRadio::RECEPTION_STATE_UNDEFINED:
                break;
            default:
                throw cRuntimeError("Unknown radio reception state");
        }
    }
    if (radioMode == IRadio::RADIO_MODE_TRANSMITTER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        switch (transmissionState) {
            case IRadio::TRANSMISSION_STATE_IDLE:
                powerConsumption += transmitterIdlePowerConsumption;
                break;
            case IRadio::TRANSMISSION_STATE_TRANSMITTING: {
                auto part = radio->getTransmittedSignalPart();
                switch (part) {
                    case IRadioSignal::SIGNAL_PART_NONE:
                        break;
                    case IRadioSignal::SIGNAL_PART_WHOLE:
                        powerConsumption += transmitterTransmittingPowerConsumption;
                        break;
                    case IRadioSignal::SIGNAL_PART_PREAMBLE:
                        powerConsumption += transmitterTransmittingPreamblePowerConsumption;
                        break;
                    case IRadioSignal::SIGNAL_PART_HEADER:
                        powerConsumption += transmitterTransmittingHeaderPowerConsumption;
                        break;
                    case IRadioSignal::SIGNAL_PART_DATA:
                        powerConsumption += transmitterTransmittingDataPowerConsumption;
                        break;
                    default:
                        throw cRuntimeError("Unknown transmitted signal part");
                }
                break;
            }
            case IRadio::TRANSMISSION_STATE_UNDEFINED:
                break;
            default:
                throw cRuntimeError("Unknown radio transmission state");
        }
    }
    return powerConsumption;
}

} // namespace physicallayer

} // namespace inet

