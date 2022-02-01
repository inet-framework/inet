//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/transceiver/base/StreamingReceiverBase.h"

namespace inet {

void StreamingReceiverBase::initialize(int stage)
{
    PacketReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        transmissionChannel = inputGate->findTransmissionChannel();
        if (transmissionChannel != nullptr) {
            transmissionChannel->subscribe(PRE_MODEL_CHANGE, this);
            transmissionChannel->subscribe(POST_MODEL_CHANGE, this);
        }
        subscribe(PRE_MODEL_CHANGE, this);
        subscribe(POST_MODEL_CHANGE, this);
    }
}

void StreamingReceiverBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    // TODO handle if the channel is cut at the receiver
}

} // namespace inet

