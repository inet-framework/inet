//
// Copyright (C) 2005 Andras Varga
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

#ifndef __INET_TURTLEMOBILITY_H
#define __INET_TURTLEMOBILITY_H

#include <stack>

#include "inet/common/INETDefs.h"
#include "inet/mobility/base/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief LOGO-style movement model, with the script coming from XML.
 * See NED file for more info.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API TurtleMobility : public LineSegmentsMobilityBase
{
  protected:
    // config
    cXMLElement *turtleScript;

    // state
    cXMLElement *nextStatement;
    double speed;
    rad heading;
    rad elevation;
    BorderPolicy borderPolicy;
    std::stack<long> loopVars;    // for <repeat>
    double maxSpeed;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase. Invokes resumeScript().*/
    virtual void setTargetPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void move() override;

    /** @brief Process next statements from script */
    virtual void resumeScript();

    /** @brief Execute the given statement*/
    virtual void executeStatement(cXMLElement *nextStatement);

    /** @brief Parse attrs in the script -- accepts things like "uniform(10,50) as well */
    virtual double getValue(const char *s);

    /** @brief Advance nextStatement pointer */
    virtual void gotoNextStatement();

    // XXX: In turtleScript xml config files, speed attributes may contain expressions (like uniform(10,30)),
    // in this case, we can't compute the maxSpeed
    virtual void computeMaxSpeed(cXMLElement *nodes);

  public:
    TurtleMobility();
    virtual double getMaxSpeed() const override { return maxSpeed; }
};

} // namespace inet

#endif // ifndef __INET_TURTLEMOBILITY_H

