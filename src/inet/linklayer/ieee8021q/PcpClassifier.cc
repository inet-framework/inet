//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#include "inet/linklayer/ieee8021q/PcpClassifier.h"

#include "inet/linklayer/common/PcpTag_m.h"

namespace inet {

Define_Module(PcpClassifier);

void PcpClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mode = par("mode");
        pcpToGateIndex = check_and_cast<cValueArray *>(par("pcpToGateIndex").objectValue());
        defaultGateIndex = par("defaultGateIndex");
    }
}

int PcpClassifier::classifyPacket(Packet *packet)
{
    int pcp = -1;
    switch (*mode) {
        case 'r': {
            auto pcpReq = packet->findTag<PcpReq>();
            if (pcpReq != nullptr)
                pcp = pcpReq->getPcp();
            break;
        }
        case 'i': {
            auto pcpInd = packet->findTag<PcpInd>();
            if (pcpInd != nullptr)
                pcp = pcpInd->getPcp();
            break;
        }
        case 'b': {
            auto pcpReq = packet->findTag<PcpReq>();
            if (pcpReq != nullptr)
                pcp = pcpReq->getPcp();
            else {
                auto pcpInd = packet->findTag<PcpInd>();
                if (pcpInd != nullptr)
                    pcp = pcpInd->getPcp();
            }
            break;
        }
    }
    if (pcp != -1)
        return pcpToGateIndex->get(pcp).intValue();
    else
        return defaultGateIndex;
}

} // namespace inet

