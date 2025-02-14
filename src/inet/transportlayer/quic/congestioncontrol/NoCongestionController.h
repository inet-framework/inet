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

#ifndef INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_NOCONGESTIONCONTROLLER_H_
#define INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_NOCONGESTIONCONTROLLER_H_

#include "ICongestionController.h"

namespace inet {
namespace quic {

class NoCongestionController: public ICongestionController {
public:
    NoCongestionController();
    virtual ~NoCongestionController();

    virtual void onPacketAcked(QuicPacket *ackedPacket) override;
    virtual void onPacketSent(QuicPacket *sentPacket) override;
    virtual void onPacketsLost(std::vector<QuicPacket *> *lostPackets, bool inPersistentCongestion) override;
    virtual uint32_t getRemainingCongestionWindow() override;
    virtual void setAppLimited(bool appLimited) override;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_NOCONGESTIONCONTROLLER_H_ */
