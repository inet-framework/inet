//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/protocol/ethernet/LlcSocketCommandProcessor.h"

namespace inet {

Define_Module(LlcSocketCommandProcessor);

void LlcSocketCommandProcessor::initialize()
{
    socketTable = getModuleFromPar<LlcSocketTable>(par("socketTableModule"), this);
}

void LlcSocketCommandProcessor::handleMessage(cMessage *msg)
{
    if (auto rq = dynamic_cast<Request *>(msg)) {
        auto ctrl = msg->getControlInfo();
        if (auto cmd = dynamic_cast<Ieee8022LlcSocketCommand *>(ctrl)) {
            handleLlcSocketCommand(rq, cmd);
            return;
        }
    }
    send(msg, "out");
}

void LlcSocketCommandProcessor::handleLlcSocketCommand(Request *request, Ieee8022LlcSocketCommand *ctrl)
{
    if (auto command = dynamic_cast<Ieee8022LlcSocketOpenCommand *>(ctrl)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();

        socketTable->createSocket(socketId, command->getLocalSap(), -1);
        delete request;
    }
    else if (dynamic_cast<Ieee8022LlcSocketCloseCommand *>(ctrl) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->deleteSocket(socketId);
        delete request;
        auto indication = new Indication("closed", IEEE8022_LLC_I_SOCKET_CLOSED);
        auto ctrl = new Ieee8022LlcSocketClosedIndication();
        indication->setControlInfo(ctrl);
        indication->addTag<SocketInd>()->setSocketId(socketId);
        send(indication, "cmdOut");

    }
    else if (dynamic_cast<Ieee8022LlcSocketDestroyCommand *>(ctrl) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        socketTable->deleteSocket(socketId);
        delete request;
    }
}

} // namespace inet

