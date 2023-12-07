//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ITCPRECOVERY_H
#define __INET_ITCPRECOVERY_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace tcp {

class INET_API ITcpRecovery : public cObject
{
  public:
    virtual void receivedDataAck(uint32_t numBytesAcked) = 0;

    virtual void receivedDuplicateAck() = 0;
};

} // namespace tcp
} // namespace inet

#endif

