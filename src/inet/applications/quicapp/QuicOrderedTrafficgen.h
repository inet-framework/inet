//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_QUIC_QUICORDEREDTRAFFICGEN_H_
#define __INET_QUIC_QUICORDEREDTRAFFICGEN_H_

#include <omnetpp.h>
#include "QuicTrafficgen.h"

using namespace omnetpp;

namespace inet {

class QuicOrderedTrafficgen : public QuicTrafficgen
{
protected:
    virtual void sendData(TrafficgenData* gmsg) override;

private:
    uint8_t currentByte = 0;
};

} //namespace

#endif
