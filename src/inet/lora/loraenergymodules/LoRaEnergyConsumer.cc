//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "../loraenergymodules/LoRaEnergyConsumer.h"

#include "../loraphy/LoRaTransmitter.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
namespace inet {

namespace lora {

using namespace inet::power;

Define_Module(LoRaEnergyConsumer);

void LoRaEnergyConsumer::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        if (!readConfigurationFile())
            throw cRuntimeError("LoRaEnergyConsumer: error in reading the input configuration file");
        standbySupplyCurrent = 0;
        sleepSupplyCurrent = 0;
        sleepPowerConsumption = mW(supplyVoltage*sleepSupplyCurrent);

        receiverIdlePowerConsumption = mW(supplyVoltage*idleSupplyCurrent);
        transmitterIdlePowerConsumption = mW(supplyVoltage*idleSupplyCurrent);
        receiverReceivingPowerConsumption = mW(supplyVoltage*receiverReceivingSupplyCurrent);
        receiverBusyPowerConsumption = mW(supplyVoltage*receiverBusySupplyCurrent);

        offPowerConsumption = W(par("offPowerConsumption"));
        switchingPowerConsumption = W(par("switchingPowerConsumption"));

        //Setting to zero, values are passed to energystorage based on tx power of current packet in getPowerConsumption()
        transmitterTransmittingPowerConsumption = W(0);
        transmitterTransmittingPreamblePowerConsumption = W(0);
        transmitterTransmittingHeaderPowerConsumption = W(0);
        transmitterTransmittingDataPowerConsumption = W(0);

        cModule *radioModule = getParentModule();
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(IRadio::receivedSignalPartChangedSignal, this);
        radioModule->subscribe(IRadio::transmittedSignalPartChangedSignal, this);
        //radioModule->subscribe(EpEnergyStorageBase::residualEnergyCapacityChangedSignal, this);
        //radioModule->subscribe(IdealEpEnergyStorage::residualEnergyCapacityChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
        const char *energySourceModule = par("energySourceModule");
        energySource = dynamic_cast<IdealEpEnergyStorage *>(getParentModule()->getSubmodule(energySourceModule));
        if (!energySource)
            throw cRuntimeError("Cannot find power source");
        //energyConsumerId = energySource->addEnergyConsumer(this);
        energySource->addEnergyConsumer(this);
        totalEnergyConsumed = 0;
        energyBalance = J(0);
    }
}

void LoRaEnergyConsumer::finish()
{
    recordScalar("totalEnergyConsumed", double(totalEnergyConsumed));
}

bool LoRaEnergyConsumer::readConfigurationFile()
{
    cXMLElement *xmlConfig = par("configFile").xmlValue();
    if (xmlConfig == nullptr)
        return false;
    cXMLElementList tagList;
    cXMLElement *tempTag;
    const char *str;
    std::string sstr;    // for easier comparison

    tagList = xmlConfig->getElementsByTagName("receiverReceivingSupplyCurrent");
    if(tagList.empty()) {
        throw cRuntimeError("receiverReceivingSupplyCurrent not defined in the configuration file!");
    }
    tempTag = tagList.front();
    str = tempTag->getAttribute("value");
    receiverReceivingSupplyCurrent = strtod(str, nullptr);

    tagList = xmlConfig->getElementsByTagName("receiverBusySupplyCurrent");
    if(tagList.empty()) {
        throw cRuntimeError("receiverBusySupplyCurrent not defined in the configuration file!");
    }
    tempTag = tagList.front();
    str = tempTag->getAttribute("value");
    receiverBusySupplyCurrent = strtod(str, nullptr);

    tagList = xmlConfig->getElementsByTagName("idleSupplyCurrent");
    if(tagList.empty()) {
        throw cRuntimeError("idleSupplyCurrent not defined in the configuration file!");
    }
    tempTag = tagList.front();
    str = tempTag->getAttribute("value");
    idleSupplyCurrent = strtod(str, nullptr);

    tagList = xmlConfig->getElementsByTagName("supplyVoltage");
    if(tagList.empty()) {
        throw cRuntimeError("supplyVoltage not defined in the configuration file!");
    }
    tempTag = tagList.front();
    str = tempTag->getAttribute("value");
    supplyVoltage = strtod(str, nullptr);

    tagList = xmlConfig->getElementsByTagName("txSupplyCurrents");
    if(tagList.empty()) {
        throw cRuntimeError("txSupplyCurrents not defined in the configuration file!");
    }
    tempTag = tagList.front();
    cXMLElementList txSupplyCurrentList = tempTag->getElementsByTagName("txSupplyCurrent");
    if (txSupplyCurrentList.empty())
        throw cRuntimeError("No txSupplyCurrent have been defined in txSupplyCurrents!");

    for (cXMLElementList::const_iterator aComb = txSupplyCurrentList.begin(); aComb != txSupplyCurrentList.end(); aComb++) {
        const char *txPower, *supplyCurrent;
        if ((*aComb)->getAttribute("txPower") != nullptr)
            txPower = (*aComb)->getAttribute("txPower");
        else
            txPower = "";
        if ((*aComb)->getAttribute("supplyCurrent") != nullptr)
            supplyCurrent = (*aComb)->getAttribute("supplyCurrent");
        else
            supplyCurrent = "";
        transmitterTransmittingSupplyCurrent[strtod(txPower, nullptr)] = strtod(supplyCurrent, nullptr);
    }
    return true;
}

void LoRaEnergyConsumer::receiveSignal(cComponent *source, simsignal_t signal, long value, cObject *details)
{
    if (signal == IRadio::radioModeChangedSignal ||
        signal == IRadio::receptionStateChangedSignal ||
        signal == IRadio::transmissionStateChangedSignal ||
        signal == IRadio::receivedSignalPartChangedSignal ||
        signal == IRadio::transmittedSignalPartChangedSignal)
    {
        powerConsumption = getPowerConsumption();
        emit(powerConsumptionChangedSignal, powerConsumption.get());

        simtime_t currentSimulationTime = simTime();
        //if (currentSimulationTime != lastEnergyBalanceUpdate) {
            energyBalance += s((currentSimulationTime - lastEnergyBalanceUpdate).dbl()) * (lastPowerConsumption);
            totalEnergyConsumed = (energyBalance.get());
            lastEnergyBalanceUpdate = currentSimulationTime;
            lastPowerConsumption = powerConsumption;
        //}
    }
    else
        throw cRuntimeError("Unknown signal");
}

W LoRaEnergyConsumer::getPowerConsumption() const
{
    IRadio::RadioMode radioMode = radio->getRadioMode();

    if (radioMode == IRadio::RADIO_MODE_OFF)
        return offPowerConsumption;
    if (radioMode == IRadio::RADIO_MODE_SLEEP || radioMode == IRadio::RADIO_MODE_SWITCHING)
        return W(0);

    W powerConsumption = W(0);
    IRadio::ReceptionState receptionState = radio->getReceptionState();
    IRadio::TransmissionState transmissionState = radio->getTransmissionState();

    if (radioMode == IRadio::RADIO_MODE_RECEIVER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (receptionState == IRadio::RECEPTION_STATE_IDLE)
            powerConsumption += receiverIdlePowerConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_RECEIVING) {
            auto part = radio->getReceivedSignalPart();
            if (part == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (part == IRadioSignal::SIGNAL_PART_WHOLE || part == IRadioSignal::SIGNAL_PART_PREAMBLE || part == IRadioSignal::SIGNAL_PART_HEADER || part == IRadioSignal::SIGNAL_PART_DATA )
                powerConsumption += receiverReceivingPowerConsumption;
            else
                throw cRuntimeError("Unknown received signal part");
        }
        else if (receptionState == IRadio::RECEPTION_STATE_BUSY)
            powerConsumption += receiverBusyPowerConsumption;
        else if (receptionState != IRadio::RECEPTION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio reception state");
        //EV << "*** EnergyConsumer: Power consumption Receiver mode, radioMode = " << radioMode << ", receptionState = " << receptionState << ", power = " << powerConsumption << std::endl;
    }

    if (radioMode == IRadio::RADIO_MODE_TRANSMITTER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (transmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            powerConsumption += transmitterIdlePowerConsumption;
        else if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
            LoRaRadio *radio = check_and_cast<LoRaRadio *>(getParentModule());
            auto part = radio->getTransmittedSignalPart();
            if (part == IRadioSignal::SIGNAL_PART_NONE)
                ;
            else if (part == IRadioSignal::SIGNAL_PART_WHOLE || part == IRadioSignal::SIGNAL_PART_PREAMBLE || part == IRadioSignal::SIGNAL_PART_HEADER || part == IRadioSignal::SIGNAL_PART_DATA)
            {
                auto current = transmitterTransmittingSupplyCurrent.find(radio->getCurrentTxPower());
                //EV << "*** EnergyConsumer: Transmission power = " << radio->getCurrentTxPower() << ", supply current = " << current->second << " power consumption = " << supplyVoltage*current->second << std::endl;
                powerConsumption += mW(supplyVoltage*current->second);
            }
            else
                throw cRuntimeError("Unknown transmitted signal part");
        }
        else if (transmissionState != IRadio::TRANSMISSION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio transmission state");
    }
    return powerConsumption;
}
}
}
