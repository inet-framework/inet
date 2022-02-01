//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RESIDENCETIMEMEASURER_H
#define __INET_RESIDENCETIMEMEASURER_H

#include "inet/common/ProtocolTag_m.h"

namespace inet {

class INET_API ResidenceTimeMeasurer : public cSimpleModule, public cListener
{
  public:
    static simsignal_t packetStayedSignal;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

