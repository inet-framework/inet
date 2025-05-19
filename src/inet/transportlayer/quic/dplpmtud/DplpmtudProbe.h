//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
