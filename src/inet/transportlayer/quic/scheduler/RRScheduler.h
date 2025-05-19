//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_SCHEDULER_RRSCHEDULER_H_
#define INET_APPLICATIONS_QUIC_SCHEDULER_RRSCHEDULER_H_

#include "IScheduler.h"

namespace inet {
namespace quic {

class RRScheduler: public IScheduler {
public:
    using IScheduler::IScheduler;
    virtual ~RRScheduler() {}

    virtual Stream *selectStream(uint64_t maxFrameSize);
    virtual Stream *getNextStream(Stream *stream);

private:
    Stream *lastScheduledStream = nullptr;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_SCHEDULER_RRSCHEDULER_H_ */
