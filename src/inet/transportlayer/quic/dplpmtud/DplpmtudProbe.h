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

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDPROBE_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDPROBE_H_

#include <string>
#include <sstream>
#include <omnetpp/simtime.h>

namespace inet {
namespace quic {

using namespace omnetpp;

class DplpmtudProbe {
public:
    DplpmtudProbe(int probeSize, SimTime timeSent, int probeCount);
    virtual ~DplpmtudProbe();

    int getSize() {
        return probeSize;
    }
    SimTime getTimeSent() {
        return timeSent;
    }
    int getProbeCount() {
        return probeCount;
    }
    bool isLost() {
        return probeLost;
    }
    void lost() {
        probeLost = true;
    }
    std::string str() {
        std::stringstream s;
        s << probeSize << "(" << probeCount << ", ";
        if (probeLost) {
            s << "lost";
        } else {
            s << "in-flight";
        }
        s << ")";
        return s.str();
    }

private:
    int probeSize;
    SimTime timeSent;
    int probeCount;
    bool probeLost = false;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDPROBE_H_ */
