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

#ifndef INET_COMMON_MISC_PERIODICLOSSCHANNEL_H_
#define INET_COMMON_MISC_PERIODICLOSSCHANNEL_H_

#include "inet/common/INETDefs.h"

namespace inet {

class PeriodicLossChannel: public cDatarateChannel {
public:
    PeriodicLossChannel(const char *name = nullptr);
    virtual ~PeriodicLossChannel();

protected:
    /**
     * Calls cDatarateChannell::processPacket and unsets packet's bit error
     * flag. If per is set to a value larger than 0.0, it counts the packets and
     * delivers exactly 1/per packets, followed by one drop.
     */
    virtual void processPacket(cPacket *pkt, const SendOptions& options, simtime_t t, Result& inoutResult) override;

private:
    /*
     * Counts packets to drop packets periodically
     */
    int packetCounter = 0;
};

} /* namespace inet */

#endif /* INET_COMMON_MISC_PERIODICLOSSCHANNEL_H_ */
