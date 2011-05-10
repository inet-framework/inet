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

#ifndef ANSIM_MOBILITY_H
#define ANSIM_MOBILITY_H

#include <omnetpp.h>
#include "LineSegmentsMobilityBase.h"


/**
 * @brief Uses the \<position_change> elements of the ANSim tool's trace file.
 * See NED file for more info.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API ANSimMobility : public LineSegmentsMobilityBase
{
  protected:
    // config
    int nodeId; ///< we'll have to compare this to the \<node_id> elements

    // state
    cXMLElement *nextPosChange; ///< points to the next \<position_change> element

  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

  protected:
    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void setTargetPosition();

    /** @brief Utility: extract data from given \<position_update> element*/
    virtual void extractDataFrom(cXMLElement *node);

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void fixIfHostGetsOutside();
};

#endif

