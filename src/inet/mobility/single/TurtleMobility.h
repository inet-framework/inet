//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TURTLEMOBILITY_H
#define __INET_TURTLEMOBILITY_H

#include <stack>

#include "inet/mobility/base/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief LOGO-style movement model, with the script coming from XML.
 * See NED file for more info.
 *
 * @ingroup mobility
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
    std::stack<long> loopVars; // for <repeat>
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

    // TODO In turtleScript xml config files, speed attributes may contain expressions (like uniform(10,30)),
    // in this case, we can't compute the maxSpeed
    virtual void computeMaxSpeed(cXMLElement *nodes);

  public:
    TurtleMobility();
    virtual double getMaxSpeed() const override { return maxSpeed; }
};

} // namespace inet

#endif

