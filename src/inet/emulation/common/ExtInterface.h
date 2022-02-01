//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EXTINTERFACE_H
#define __INET_EXTINTERFACE_H

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

class INET_API ExtInterface : public NetworkInterface
{
  protected:
    virtual void configureInterface();
    virtual void copyNetworkInterfaceConfigurationFromExt();
    virtual void copyNetworkInterfaceConfigurationToExt();
    virtual void copyNetworkAddressFromExt();
    virtual void copyNetworkAddressToExt();

  public:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif

