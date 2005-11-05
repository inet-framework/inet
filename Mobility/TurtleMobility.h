//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef TURTLE_MOBILITY_H
#define TURTLE_MOBILITY_H

#include <stack>
#include "LineSegmentsMobilityBase.h"


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
    double angle;
    BorderPolicy borderPolicy;
    std::stack<long> loopVars; // for <repeat>

  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

    /** @brief Overridden from LineSegmentsMobilityBase. Invokes resumeScript().*/
    virtual void setTargetPosition();

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void fixIfHostGetsOutside();

  protected:
    /** @brief Process next statements from script */
    void resumeScript();

    /** @brief Execute the given statement*/
    void executeStatement(cXMLElement *nextStatement);

    /** @brief Parse attrs in the script -- accepts things like "uniform(10,50) as well */
    double getValue(const char *s);

    /** @brief Advance nextStatement pointer */
    void gotoNextStatement();
};

#endif

