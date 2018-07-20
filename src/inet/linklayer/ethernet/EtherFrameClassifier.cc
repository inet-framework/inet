//
// Copyright (C) 2011 Zoltan Bojthe
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/linklayer/ethernet/EtherFrameClassifier.h"

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"

namespace inet {

Define_Module(EtherFrameClassifier);

void EtherFrameClassifier::handleMessage(cMessage *msg)
{
    //FIXME msg is always Packet*, need another way to detect pause frame
    if (Packet *pk = dynamic_cast<Packet *>(msg)) {
        auto hdr = pk->peekAtFront<EthernetMacHeader>(b(-1), Chunk::PF_ALLOW_NULLPTR|Chunk::PF_ALLOW_INCOMPLETE);
        if (hdr != nullptr) {
            if (hdr->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) {
                send(msg, "pauseOut");
                return;
            }
        }
    }

    send(msg, "defaultOut");
}

} // namespace inet

