//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDPROBES_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDPROBES_H_

#include "DplpmtudProbe.h"
#include <map>

namespace inet {
namespace quic {

class DplpmtudProbes : public std::map<int, DplpmtudProbe *> {
public:
    DplpmtudProbes();
    virtual ~DplpmtudProbes();

    void add(DplpmtudProbe *probe);
    void removeAll();
    void removeAllEqualOrSmaller(int size);
    void removeAllLarger(int size);
    void removeAllEqualOrLarger(int size);
    DplpmtudProbe *getBySize(int size);
    DplpmtudProbe *getSmallest();
    DplpmtudProbe *getSmallestLost();
    bool containsProbesNotLost();
    std::string str();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDPROBES_H_ */
