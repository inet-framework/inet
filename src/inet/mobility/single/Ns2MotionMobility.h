//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2008 Alfonso Ariza
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

#ifndef __INET_NS2MOTIONMOBILITY_H
#define __INET_NS2MOTIONMOBILITY_H

#include "inet/common/INETDefs.h"

#include "inet/mobility/base/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief Uses the ns2 motion native file format. See NED file for more info.
 *
 * @ingroup mobility
 * @author Alfonso Ariza
 */

class Ns2MotionMobility;

/**
 * Represents a ns2 motion file's contents.
 */
class INET_API Ns2MotionFile
{
  public:
    typedef std::vector<double> Line;
    double initial[3];

  protected:
    friend class Ns2MotionMobility;
    typedef std::vector<Line> LineList;
    LineList lines;
};

class INET_API Ns2MotionMobility : public LineSegmentsMobilityBase
{
  protected:
    // state
    unsigned int vecpos;
    Ns2MotionFile *ns2File;
    int nodeId;
    double scrollX;
    double scrollY;
    double maxSpeed;

  protected:
    void parseFile(const char *filename);

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void setTargetPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void move() override;

    virtual void computeMaxSpeed();
  public:
    Ns2MotionMobility();
    virtual ~Ns2MotionMobility();
    virtual double getMaxSpeed() const override { return maxSpeed; }
};

} // namespace inet

#endif // ifndef __INET_NS2MOTIONMOBILITY_H

