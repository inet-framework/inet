//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPCONGESTIONCONTROL_H
#define __INET_TCPCONGESTIONCONTROL_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace tcp {

class INET_API TcpCongestionControl : public cObject
{
  public:
    virtual void receivedDataAck(uint32_t firstSeqAcked) = 0;

    virtual void receivedDuplicateAck() = 0;
};

} // namespace tcp
} // namespace inet

#endif

