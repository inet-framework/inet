//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/protocolelement/transceiver/base/StreamingTransmitterBase.h"

namespace inet {

void StreamingTransmitterBase::initialize(int stage)
{
    PacketTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        transmissionChannel = outputGate->findTransmissionChannel();
        if (transmissionChannel != nullptr) {
            transmissionChannel->subscribe(PRE_MODEL_CHANGE, this);
            transmissionChannel->subscribe(POST_MODEL_CHANGE, this);
        }
        subscribe(PRE_MODEL_CHANGE, this);
        subscribe(POST_MODEL_CHANGE, this);
    }
}

bool StreamingTransmitterBase::canPushSomePacket(cGate *gate) const
{
    return transmissionChannel != nullptr && !transmissionChannel->isDisabled() && PacketTransmitterBase::canPushSomePacket(gate);
}

void StreamingTransmitterBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
#ifdef WITH_CLOCK
    ClockUserModuleMixin::receiveSignal(source, signal, object, details);
#else
    Enter_Method("receiveSignal");
#endif
    if (signal == PRE_MODEL_CHANGE) {
        if (auto notification = dynamic_cast<cPrePathCutNotification *>(object)) {
            if (outputGate == notification->pathStartGate && isTransmitting())
                abortTx();
        }
        else if (auto notification = dynamic_cast<cPreParameterChangeNotification *>(object)) {
            if (notification->par->getOwner() == transmissionChannel &&
                notification->par->getType() == cPar::BOOL && strcmp(notification->par->getName(), "disabled") == 0 &&
                !transmissionChannel->isDisabled() &&
                isTransmitting()) // TODO: the new value of parameter currently unavailable: notification->newValue == true
            {
                abortTx();
            }
        }
    }
    else if (signal == POST_MODEL_CHANGE) {
        if (auto notification = dynamic_cast<cPostPathCreateNotification *>(object)) {
            if (outputGate == notification->pathStartGate) {
                transmissionChannel = outputGate->findTransmissionChannel();
                if (transmissionChannel != nullptr && !transmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
                    transmissionChannel->subscribe(POST_MODEL_CHANGE, this);
                if (producer != nullptr)
                    producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
            }
        }
        else if (auto notification = dynamic_cast<cPostPathCutNotification *>(object)) {
            if (outputGate == notification->pathStartGate) {
                transmissionChannel = nullptr;
                if (producer != nullptr)
                    producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
            }
        }
        else if (auto notification = dynamic_cast<cPostParameterChangeNotification *>(object)) {
            if (producer != nullptr && notification->par->getOwner() == transmissionChannel && notification->par->getType() == cPar::BOOL && strcmp(notification->par->getName(), "disabled") == 0)
                producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
        }
    }
}

} // namespace inet

