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

#ifndef INET_TRANSPORTLAYER_QUIC_PACKET_DPLPMTUDPROBEPACKET_H_
#define INET_TRANSPORTLAYER_QUIC_PACKET_DPLPMTUDPROBEPACKET_H_

#include "QuicPacket.h"
#include "../dplpmtud/Dplpmtud.h"

namespace inet {
namespace quic {

class Dplpmtud;

class DplpmtudProbePacket: public QuicPacket {
public:
    DplpmtudProbePacket(std::string name, Dplpmtud *dplpmtud);
    virtual ~DplpmtudProbePacket();

    virtual void onPacketLost() override;
    virtual void onPacketAcked() override;
    virtual bool isDplpmtudProbePacket() override;

private:
    Dplpmtud *dplpmtud;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_PACKET_DPLPMTUDPROBEPACKET_H_ */
