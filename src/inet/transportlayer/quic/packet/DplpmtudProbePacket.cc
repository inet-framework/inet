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

#include "DplpmtudProbePacket.h"

namespace inet {
namespace quic {

DplpmtudProbePacket::DplpmtudProbePacket(std::string name, Dplpmtud *dplpmtud) : QuicPacket(name) {
    this->dplpmtud = dplpmtud;
}

DplpmtudProbePacket::~DplpmtudProbePacket() { }

void DplpmtudProbePacket::onPacketLost()
{
    dplpmtud->onProbePacketLost(this->getSize());
}

void DplpmtudProbePacket::onPacketAcked()
{
    dplpmtud->onProbePacketAcked(this->getSize());
}

bool DplpmtudProbePacket::isDplpmtudProbePacket()
{
    return true;
}

} /* namespace quic */
} /* namespace inet */
