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

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/NodeOperations.h"

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
  public:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void handleMessageWhenUp(cMessage *msg) override;

  protected:
    // lifecycle
    virtual bool handleNodeStart(IDoneCallback *) override { setupNetworkLayer(); return true; }
    virtual bool handleNodeShutdown(IDoneCallback *) override { return true; }
    virtual void handleNodeCrash() override {}
    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_NETWORK_CONFIGURATION; }
    virtual bool isNodeStartStage(int stage) override { return stage == NodeStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isNodeShutdownStage(int stage) override { return stage == NodeShutdownOperation::STAGE_NETWORK_LAYER; }

    virtual void setupNetworkLayer();
};

} // namespace inet

#endif // ifndef __INET_HOSTAUTOCONFIGURATOR_H

