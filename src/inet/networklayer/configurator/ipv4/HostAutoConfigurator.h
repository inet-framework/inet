/*
 * Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __INET_HOSTAUTOCONFIGURATOR_H
#define __INET_HOSTAUTOCONFIGURATOR_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

/**
 * HostAutoConfigurator automatically assigns IP addresses and sets up the
 * routing table for the host it is part of.
 *
 * For more info please see the NED file.
 *
 * @author Christoph Sommer
 */
class INET_API HostAutoConfigurator : public OperationalBase
{
  protected:
    IInterfaceTable *interfaceTable = nullptr;

  public:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void handleMessageWhenUp(cMessage *msg) override;

  protected:
    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override { setupNetworkLayer(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { }
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_NETWORK_CONFIGURATION; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }

    virtual void setupNetworkLayer();
};

} // namespace inet

#endif // ifndef __INET_HOSTAUTOCONFIGURATOR_H

