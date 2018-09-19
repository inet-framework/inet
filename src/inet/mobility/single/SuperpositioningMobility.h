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

#ifndef __INET_SUPERPOSITIONINGMOBILITY_H
#define __INET_SUPERPOSITIONINGMOBILITY_H

#include "inet/mobility/base/MobilityBase.h"

namespace inet {

class INET_API SuperpositioningMobility : public MobilityBase, public cListener
{
  protected:
    enum class PositionComposition {
        PC_UNDEFINED = -1,
        PC_ZERO,
        PC_SUM,
        PC_AVERAGE,
    };

    enum class OrientationComposition {
        OC_UNDEFINED = -1,
        OC_ZERO,
        OC_SUM,
        OC_AVERAGE,
        OC_FACE_FORWARD
    };

  protected:
    PositionComposition positionComposition = PositionComposition::PC_UNDEFINED;
    OrientationComposition orientationComposition = OrientationComposition::OC_UNDEFINED;
    std::vector<IMobility *> elements;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleSelfMessage(cMessage *msg) override { throw cRuntimeError("Unknown self message"); }
    virtual void setInitialPosition() override;

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

#endif // ifndef __INET_SUPERPOSITIONINGMOBILITY_H

