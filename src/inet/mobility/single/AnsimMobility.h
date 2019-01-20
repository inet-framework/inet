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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_ANSIMMOBILITY_H
#define __INET_ANSIMMOBILITY_H

#include "inet/common/INETDefs.h"
#include "inet/mobility/base/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief Uses the \<position_change> elements of the ANSim tool's trace file.
 * See NED file for more info.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API AnsimMobility : public LineSegmentsMobilityBase
{
  protected:
    // config
    int nodeId;    ///< we'll have to compare this to the \<node_id> elements
    // state
    cXMLElement *nextPositionChange;    ///< points to the next \<position_change> element
    double maxSpeed; // the possible maximum speed at any future time
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int stage) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase. */
    virtual void setTargetPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase. */
    virtual void move() override;

    /** @brief Finds the next \<position_change> element. */
    virtual cXMLElement *findNextPositionChange(cXMLElement *positionChange);

    /** @brief Utility: extract data from given \<position_update> element. */
    virtual void extractDataFrom(cXMLElement *node);
    virtual void computeMaxSpeed();
  public:
    virtual double getMaxSpeed() const override { return maxSpeed; }
    AnsimMobility();
};

} // namespace inet

#endif // ifndef __INET_ANSIMMOBILITY_H

