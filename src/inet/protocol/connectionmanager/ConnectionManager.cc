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
#include "inet/protocol/connectionmanager/ConnectionManager.h"

namespace inet {

Define_Module(ConnectionManager);

ConnectionManager::~ConnectionManager()
{
    delete txSignal;
}

void ConnectionManager::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("physIn");
        physOutGate = gate("physOut");
        txTransmissionChannel = physOutGate->findTransmissionChannel();
        rxTransmissionChannel = physInGate->findIncomingTransmissionChannel();

        subscribe(PRE_MODEL_CHANGE, this);
        subscribe(POST_MODEL_CHANGE, this);

        if (rxTransmissionChannel && txTransmissionChannel) {
            rxTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
            txTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
        }
    }
}

void ConnectionManager::handleMessage(cMessage *msg)
{
//    cProgress *progress = check_and_cast<cProgress *>(msg);
//    auto now = simTime();
//    if (connected && !disabled) {
//        if (progress->arrivedOn("in")) {
//            switch(progress->getKind()) {
//                case cProgress::PACKET_START:
//                    ASSERT(txSignal == nullptr);
//                    txSignal = check_and_cast<physicallayer::Signal *>(progress->getPacket()->dup());
//                    txStartTime = now;
//                    EV << "Received PACKET_START " << progress << " from upper, send to phy\n";
//                    send(progress, physOutGate);
//                    break;
//                case cProgress::PACKET_PROGRESS:
//                    ASSERT(txSignal != nullptr);
//                    ASSERT(now == txStartTime + progress->getTimePosition());
//                    delete txSignal;
//                    txSignal = check_and_cast<physicallayer::Signal *>(progress->getPacket()->dup());
//                    EV << "Received PACKET_PROGRESS " << progress << " from upper, send to phy\n";
//                    send(progress, physOutGate);
//                    break;
//                case cProgress::PACKET_END:
//                    ASSERT(txSignal != nullptr);
//                    ASSERT(now == txStartTime + progress->getTimePosition());
//                    delete txSignal;
//                    txSignal = nullptr;
//                    txStartTime = -1;
//                    EV << "Received PACKET_END " << progress << " from upper, send to phy\n";
//                    send(progress, physOutGate);
//                    break;
//                default:
//                    throw cRuntimeError("Unknown progress kind %d", progress->getKind());
//            }
//        }
//        else if (progress->arrivedOn("physIn")) {
//            EV << "Received " << progress << " from phy, send up\n";
//            send(progress, "out");
//        }
//        else
//            throw cRuntimeError("unknown gate");
//    }
//    else {
//        EV << "Received " << progress << ", dropped\n";
//        delete msg;
//    }
}

void ConnectionManager::propagatePreChannelOff()
{
    if (txSignal != nullptr) {
        txTransmissionChannel->forceTransmissionFinishTime(simTime());
        delete txSignal;
        txSignal = nullptr;
    }
}

void ConnectionManager::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    if (signalID == PRE_MODEL_CHANGE) {
        if (auto gcobj = dynamic_cast<cPrePathCutNotification *>(obj)) {
            if (// TODO: connected &&
                    ((physInGate == gcobj->pathEndGate) || (physOutGate == gcobj->pathStartGate))) {
                propagatePreChannelOff();
            }
        }
        else if (auto gcobj = dynamic_cast<cPreParameterChangeNotification *>(obj)) {
            if (// TODO: connected
                    (gcobj->par->getOwner() == rxTransmissionChannel || gcobj->par->getOwner() == txTransmissionChannel)
                    && gcobj->par->getType() == cPar::BOOL
                    && strcmp(gcobj->par->getName(), "disabled") == 0
                    /* && gcobj->newValue == true */ //TODO the new value of parameter currently unavailable
                    ) {
                propagatePreChannelOff();
            }
        }
    }
}

} // namespace inet

