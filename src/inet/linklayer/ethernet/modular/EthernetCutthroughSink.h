//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETCUTTHROUGHSINK_H
#define __INET_ETHERNETCUTTHROUGHSINK_H

#include "inet/protocolelement/common/PacketStreamer.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetCutthroughSink : public PacketStreamer
{
  protected:
    virtual void endStreaming() override;
};

} // namespace inet

#endif

