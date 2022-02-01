//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2008 Alfonso Ariza
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_NS2MOTIONMOBILITY_H
#define __INET_NS2MOTIONMOBILITY_H

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

#endif

