//
// Copyright (C) 2015 OpenSim Ltd.
// Copyright (C) 2019 Alfonso Ariza, universidad de Malaga.
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

#ifndef __INET_MANHATTANGRID_H
#define __INET_MANHATTANGRID_H

#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/LineSegmentsMobilityBase.h"

namespace inet {

class INET_API ManhattanGrid : public LineSegmentsMobilityBase
{
  protected:
    // configuration
    bool nextMoveIsWait;
    cPar *speedParameter = nullptr;
    cPar *waitTimeParameter = nullptr;
    bool hasWaitTime;
    Coord lastCroosPoint = Coord::NIL;
    typedef std::vector<Coord> CrossPoints;

    std::map<double,CrossPoints> intersectionX;
    std::map<double,CrossPoints> intersectionY;

  protected:
    virtual void initialize(int stage) override;
    virtual void setInitialPosition() override;
    virtual void move() override;
    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void setTargetPosition() override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

  public:
    virtual double getMaxSpeed() const override;
};

} // namespace inet

#endif // ifndef __INET_VEHICLEMOBILITY_H

