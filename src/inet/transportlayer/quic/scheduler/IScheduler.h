//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_SCHEDULER_ISCHEDULER_H_
#define INET_APPLICATIONS_QUIC_SCHEDULER_ISCHEDULER_H_

#include "../stream/Stream.h"

namespace inet {
namespace quic {

class Stream;

class IScheduler {
public:
    IScheduler(std::map<uint64_t, Stream *> *streamMap) {
        this->streamMap = streamMap;
    }
    virtual ~IScheduler() { };

    virtual Stream *selectStream(uint64_t maxFrameSize) = 0;

protected:
    std::map<uint64_t, Stream *> *streamMap = nullptr;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_SCHEDULER_ISCHEDULER_H_ */
