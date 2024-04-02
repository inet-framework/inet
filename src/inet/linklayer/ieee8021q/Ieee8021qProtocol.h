//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021QPROTOCOL_H
#define __INET_IEEE8021QPROTOCOL_H

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

class INET_API Ieee8021qProtocol : public cModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif

