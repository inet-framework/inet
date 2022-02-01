//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MACRELAYUNIT_H
#define __INET_MACRELAYUNIT_H

#include "inet/linklayer/base/MacRelayUnitBase.h"

namespace inet {

class INET_API MacRelayUnit : public MacRelayUnitBase
{
  protected:
    virtual void handleLowerPacket(Packet *packet) override;
};

} // namespace inet

#endif

