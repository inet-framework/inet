//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SINK_H
#define __INET_SINK_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API Sink : public cSimpleModule
{
  protected:
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif

