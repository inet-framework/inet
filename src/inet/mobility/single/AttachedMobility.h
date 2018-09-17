//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_ATTACHEDMOBILITY_H
#define __INET_ATTACHEDMOBILITY_H

#include "inet/mobility/base/MobilityBase.h"

namespace inet {

class INET_API AttachedMobility : public MobilityBase, public cListener
{
  protected:
    IMobility *mobility = nullptr;
    Coord positionOffset = Coord::NIL;
    Quaternion orientationOffset = Quaternion::NIL;
    bool isZeroOffset = false;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleSelfMessage(cMessage *msg) override { throw cRuntimeError("Unknown self message"); }

  public:
    virtual Coord getCurrentPosition() override;
    virtual Coord getCurrentVelocity() override;
    virtual Coord getCurrentAcceleration() override;

    virtual Quaternion getCurrentAngularPosition() override;
    virtual Quaternion getCurrentAngularVelocity() override;
    virtual Quaternion getCurrentAngularAcceleration() override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif // ifndef __INET_ATTACHEDMOBILITY_H

