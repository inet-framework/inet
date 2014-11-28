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

#include "inet/linklayer/ethernet/switch/MACRelayUnit.h"
#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/linklayer/ethernet/EtherMACBase.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"

namespace inet {

Define_Module(MACRelayUnit);

void MACRelayUnit::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // number of ports
        numPorts = gate("ifOut", 0)->size();
        if (gate("ifIn", 0)->size() != numPorts)
            throw cRuntimeError("the sizes of the ifIn[] and ifOut[] gate vectors must be the same");

        numProcessedFrames = numDiscardedFrames = 0;

        addressTable = check_and_cast<IMACAddressTable *>(getModuleByPath(par("macTablePath")));

        WATCH(numProcessedFrames);
        WATCH(numDiscardedFrames);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
    }
}

void MACRelayUnit::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
        return;
    }
    EtherFrame *frame = check_and_cast<EtherFrame *>(msg);
    // Frame received from MAC unit
    handleAndDispatchFrame(frame);
}

void MACRelayUnit::handleAndDispatchFrame(EtherFrame *frame)
{
    int inputport = frame->getArrivalGate()->getIndex();

    numProcessedFrames++;

    // update address table
    addressTable->updateTableWithAddress(inputport, frame->getSrc());

    // handle broadcast frames first
    if (frame->getDest().isBroadcast()) {
        EV << "Broadcasting broadcast frame " << frame << endl;
        broadcastFrame(frame, inputport);
        return;
    }

    // Finds output port of destination address and sends to output port
    // if not found then broadcasts to all other ports instead
    int outputport = addressTable->getPortForAddress(frame->getDest());
    // should not send out the same frame on the same ethernet port
    // (although wireless ports are ok to receive the same message)
    if (inputport == outputport) {
        EV << "Output port is same as input port, " << frame->getFullName()
           << " dest " << frame->getDest() << ", discarding frame\n";
        numDiscardedFrames++;
        delete frame;
        return;
    }

    if (outputport >= 0) {
        EV << "Sending frame " << frame << " with dest address " << frame->getDest() << " to port " << outputport << endl;
        send(frame, "ifOut", outputport);
    }
    else {
        EV << "Dest address " << frame->getDest() << " unknown, broadcasting frame " << frame << endl;
        broadcastFrame(frame, inputport);
    }
}

void MACRelayUnit::broadcastFrame(EtherFrame *frame, int inputport)
{
    for (int i = 0; i < numPorts; ++i)
        if (i != inputport)
            send((EtherFrame *)frame->dup(), "ifOut", i);

    delete frame;
}

void MACRelayUnit::start()
{
    addressTable->clearTable();
    isOperational = true;
}

void MACRelayUnit::stop()
{
    addressTable->clearTable();
    isOperational = false;
}

bool MACRelayUnit::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER) {
            start();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER) {
            stop();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            stop();
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}

void MACRelayUnit::finish()
{
    recordScalar("processed frames", numProcessedFrames);
    recordScalar("discarded frames", numDiscardedFrames);
}

} // namespace inet

