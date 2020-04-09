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

#include "inet/protocol/connectionmanager/TxConnectionManager.h"

namespace inet {

void TxConnectionManager::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        subscribe(PRE_MODEL_CHANGE, this);
        subscribe(POST_MODEL_CHANGE, this);
        physOutGate = gate("physOut");
        connected = physOutGate->getPathEndGate()->isConnected();
        txTransmissionChannel = physOutGate->findTransmissionChannel();
        disabled = txTransmissionChannel ? txTransmissionChannel->isDisabled() : true;
        if (txTransmissionChannel && !txTransmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
            txTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        propagateStatus();
    }
}

void TxConnectionManager::handleMessage(cMessage *msg)
{
}

void TxConnectionManager::propagatePreChannelOff()
{
    throw cRuntimeError("Add implementation");
}

void TxConnectionManager::propagateStatus()
{
    throw cRuntimeError("Add implementation");
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
        else if (auto gcobj = dynamic_cast<cPostParameterChangeNotification *>(obj)) {
            if ((gcobj->par->getOwner() == txTransmissionChannel)) {
                disabled = txTransmissionChannel->isDisabled();
                propagateStatus();
            }
        }
    }
}

} // namespace inet

