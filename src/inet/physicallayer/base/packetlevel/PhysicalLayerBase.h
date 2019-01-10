//
// Copyright (C) 2013 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PHYSICALLAYERBASE_H
#define __INET_PHYSICALLAYERBASE_H

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/physicallayer/contract/packetlevel/IPhysicalLayer.h"

namespace inet {

namespace physicallayer {

class INET_API PhysicalLayerBase : public LayeredProtocolBase, public IPhysicalLayer
{
  protected:
    int upperLayerInGateId = -1;
    int upperLayerOutGateId = -1;
    int radioInGateId = -1;

  protected:
    virtual void initialize(int stage) override;

    virtual void handleSignal(Signal *signal);
    virtual void handleLowerMessage(cMessage *message) override;

    virtual void sendUp(cMessage *message);

    virtual bool isUpperMessage(cMessage *message) override;
    virtual bool isLowerMessage(cMessage *message) override;

    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_PHYSICAL_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_PHYSICAL_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_PHYSICAL_LAYER; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_PHYSICALLAYERBASE_H

