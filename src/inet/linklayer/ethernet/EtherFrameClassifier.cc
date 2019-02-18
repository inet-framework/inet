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

#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherFrameClassifier.h"

namespace inet {

Define_Module(EtherFrameClassifier);

int EtherFrameClassifier::classifyPacket(Packet *packet)
{
    //FIXME need another way to detect pause frame
    auto header = packet->peekAtFront<EthernetMacHeader>(b(-1), Chunk::PF_ALLOW_NULLPTR|Chunk::PF_ALLOW_INCOMPLETE);
    if (header != nullptr && header->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL)
        return 0;
    else
        return 1;
}

} // namespace inet

