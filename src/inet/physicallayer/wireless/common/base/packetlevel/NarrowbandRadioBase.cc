//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandRadioBase.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceiverBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandTransmitterBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {

namespace physicallayer {

Define_Module(NarrowbandRadioBase);

NarrowbandRadioBase::NarrowbandRadioBase() :
    Radio()
{
}

void NarrowbandRadioBase::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE) {
        ConfigureRadioCommand *configureCommand = check_and_cast<ConfigureRadioCommand *>(message->getControlInfo());
        const IModulation *newModulation = configureCommand->getModulation();
        if (newModulation != nullptr)
            setModulation(newModulation);
        Hz newCenterFrequency = configureCommand->getCenterFrequency();
        if (!std::isnan(newCenterFrequency.get()))
            setCenterFrequency(newCenterFrequency);
        Hz newBandwidth = configureCommand->getBandwidth();
        if (!std::isnan(newBandwidth.get()))
            setBandwidth(newBandwidth);
    }
    Radio::handleUpperCommand(message);
}

void NarrowbandRadioBase::setModulation(const IModulation *newModulation)
{
    NarrowbandTransmitterBase *narrowbandTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
    narrowbandTransmitter->setModulation(newModulation);
    NarrowbandReceiverBase *narrowbandReceiver = const_cast<NarrowbandReceiverBase *>(check_and_cast<const NarrowbandReceiverBase *>(receiver));
    narrowbandReceiver->setModulation(newModulation);
}

void NarrowbandRadioBase::setCenterFrequency(Hz newCenterFrequency)
{
    NarrowbandTransmitterBase *narrowbandTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
    narrowbandTransmitter->setCenterFrequency(newCenterFrequency);
    NarrowbandReceiverBase *narrowbandReceiver = const_cast<NarrowbandReceiverBase *>(check_and_cast<const NarrowbandReceiverBase *>(receiver));
    narrowbandReceiver->setCenterFrequency(newCenterFrequency);
}

void NarrowbandRadioBase::setBandwidth(Hz newBandwidth)
{
    NarrowbandTransmitterBase *narrowbandTransmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(transmitter));
    narrowbandTransmitter->setBandwidth(newBandwidth);
    NarrowbandReceiverBase *narrowbandReceiver = const_cast<NarrowbandReceiverBase *>(check_and_cast<const NarrowbandReceiverBase *>(receiver));
    narrowbandReceiver->setBandwidth(newBandwidth);
    receptionTimer = nullptr;
}

} // namespace physicallayer

} // namespace inet

