//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDEALCONTROLCHANNEL_H
#define __INET_IDEALCONTROLCHANNEL_H

#include "inet/common/SimpleModule.h"

namespace inet {

namespace cellular {

class GnbMac;
class UeMac;

/**
 * The out-of-band control plane of the dummy model: the MACs find each other here, and then
 * exchange attachments, uplink grants and SNIR reports by direct method calls, without delay
 * and without errors. No control signal goes over the radio medium.
 */
class INET_API IdealControlChannel : public SimpleModule
{
  protected:
    // in registration order, which is the initialization order of the modules
    std::vector<GnbMac *> gnbMacs;
    std::vector<UeMac *> ueMacs;

  protected:
    virtual void handleMessage(cMessage *message) override;

  public:
    // the index of the registered MAC
    int registerGnbMac(GnbMac *gnbMac);
    int registerUeMac(UeMac *ueMac);

    const std::vector<GnbMac *>& getGnbMacs() const { return gnbMacs; }
};

} // namespace cellular

} // namespace inet

#endif

