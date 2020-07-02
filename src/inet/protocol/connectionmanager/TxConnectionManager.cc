//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/protocol/connectionmanager/TxConnectionManager.h"

namespace inet {

Define_Module(TxConnectionManager);

TxConnectionManager::~TxConnectionManager()
{
    delete txSignal;
}

void TxConnectionManager::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        physOutGate = gate("physOut");
        connected = physOutGate->getPathEndGate()->isConnected();
        disabled = true;
        interfaceEntry = getContainingNicModule(this);
        txTransmissionChannel = physOutGate->findTransmissionChannel();

        subscribe(PRE_MODEL_CHANGE, this);
        subscribe(POST_MODEL_CHANGE, this);

        if (txTransmissionChannel) {
            disabled = txTransmissionChannel->isDisabled();
            txTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);

            // TODO copied from ChannelDatarateReader
            if (txTransmissionChannel->hasPar("datarate")) {
                bitrate = txTransmissionChannel->par("datarate");
                propagateDatarate();
            }
        }
        WATCH(bitrate);
        WATCH(connected);
        WATCH(disabled);
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        propagateStatus();
    }
}

void TxConnectionManager::handleMessage(cMessage *msg)
{
    cProgress *progress = check_and_cast<cProgress *>(msg);
    if (connected && !disabled) {
        switch(progress->getKind()) {
            case cProgress::PACKET_START:
                ASSERT(txSignal == nullptr);
                txSignal = check_and_cast<physicallayer::Signal *>(progress->getPacket()->dup());
                txStartTime = simTime();
                send(progress, physOutGate);
                break;
            case cProgress::PACKET_PROGRESS:
                ASSERT(txSignal != nullptr);
                ASSERT(simTime() == txStartTime + progress->getTimePosition());
                delete txSignal;
                txSignal = check_and_cast<physicallayer::Signal *>(progress->getPacket()->dup());
                send(progress, physOutGate);
                break;
            case cProgress::PACKET_END:
                ASSERT(txSignal != nullptr);
                ASSERT(simTime() == txStartTime + progress->getTimePosition());
                delete txSignal;
                txSignal = nullptr;
                txStartTime = -1;
                send(progress, physOutGate);
                break;
            default:
                throw cRuntimeError("Unknown progress kind %d", progress->getKind());
        }
    }
    else {
        delete msg;
    }
}

void TxConnectionManager::propagatePreChannelOff()
{
    if (txSignal != nullptr) {
#if 0   // correct code
        auto packet = txSignal->decapsulate();

        //TODO truncate Packet

        auto duration = simTime() - txStartTime;
        txSignal->encapsulate(packet);
        txSignal->setDuration(duration);
        sendPacketEnd(txSignal, physOutGate, duration);
#else   // KLUDGE for fingerprint
        txTransmissionChannel->forceTransmissionFinishTime(simTime());
        delete txSignal;
#endif
        txSignal = nullptr;
    }
}

void TxConnectionManager::propagateStatus()
{
    interfaceEntry->setCarrier(connected && ! disabled);
}

void TxConnectionManager::propagateDatarate()
{
    //TODO which is the good solution from these?
    interfaceEntry->par("bitrate").setDoubleValue(bitrate);
    interfaceEntry->setDatarate(bitrate);
}

void TxConnectionManager::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    if (signalID == PRE_MODEL_CHANGE) {
        if (auto gcobj = dynamic_cast<cPrePathCutNotification *>(obj)) {
            if (connected && (physOutGate == gcobj->pathStartGate)) {
                propagatePreChannelOff();
            }
        }
        else if (auto gcobj = dynamic_cast<cPreParameterChangeNotification *>(obj)) {
            if (connected
                    && (gcobj->par->getOwner() == txTransmissionChannel)
                    && gcobj->par->getType() == cPar::BOOL
                    && strcmp(gcobj->par->getName(), "disabled") == 0
                    /* && gcobj->newValue == true */ //TODO the new value of parameter currently unavailable
                    ) {
                propagatePreChannelOff();
            }
        }
    }
    else if (signalID == POST_MODEL_CHANGE) {
        if (auto gcobj = dynamic_cast<cPostPathCreateNotification *>(obj)) {
            if (physOutGate == gcobj->pathStartGate) {
                connected = physOutGate->getPathEndGate()->isConnected();
                txTransmissionChannel = physOutGate->findTransmissionChannel();
                disabled = txTransmissionChannel ? txTransmissionChannel->isDisabled() : true;
                if (txTransmissionChannel && !txTransmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
                    txTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
                propagateStatus();
            }
        }
        else if (auto gcobj = dynamic_cast<cPostPathCutNotification *>(obj)) {
            if (connected && (physOutGate == gcobj->pathStartGate)) {
                connected = false;
                propagateStatus();
            }
        }
        else if (auto gcobj = dynamic_cast<cPostParameterChangeNotification *>(obj)) {
            if ((gcobj->par->getOwner() == txTransmissionChannel)) {
                if (disabled != txTransmissionChannel->isDisabled()) {
                    disabled = txTransmissionChannel->isDisabled();
                    propagateStatus();
                }
                if (txTransmissionChannel->hasPar("datarate")) {
                    double newbitrate = txTransmissionChannel->par("datarate");
                    if (bitrate != newbitrate) {
                        bitrate = newbitrate;
                        propagateDatarate();
                    }
                }
            }
        }
    }
}

} // namespace inet

