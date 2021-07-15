//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "inet/linklayer/common/RandomQosClassifier.h"

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"

namespace inet {

Define_Module(RandomQosClassifier);

void RandomQosClassifier::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
    }
}

void RandomQosClassifier::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(intrand(8));
    send(msg, "out");
}

std::vector<cGate *> RandomQosClassifier::getRegistrationForwardingGates(cGate *gate)
{
    if (gate == outputGate)     // TODO this case was missed from original code
        return std::vector<cGate *>({inputGate});
    else if (gate == inputGate)
        return std::vector<cGate *>({outputGate});
    else
        throw cRuntimeError("Unknown gate");
}

} // namespace inet

