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

#ifndef __INET_CIRCLEMOBILITY_H
#define __INET_CIRCLEMOBILITY_H

#include "inet/common/INETDefs.h"
#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

/**
 * @brief Circle movement model. See NED file for more info.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API PaparazziMobility : public MovingMobilityBase
{
  public:
    enum Mode {
        STAYAT,
        WAYPOINT,
        EIGHT,
        SCAN,
        OVAL
    };

    PaparazziMobility();

  protected:
    simtime_t lastChange;

    bool randomSeq;
    std::vector<Mode> sequenceModels;
    int latestSeq = -1;


    PaparazziMobility::Mode getNextState();
    void startNextState();
    void startState(const Mode &s);
    Coord randomPosition() const;
    void circularMouvement(const double &sp, const double &radious, double initAngle);
    void moveLinear();
    void moveOval(const double &sp, const double &radious, const double &linDist, const double &initialAngle);
    void moveEight(const double &sp, const double &radious, const double &linDist, const double &initialAngle);
    void moveScan(const double &sp, const double &radious, const double &linDist,const int &hLines , const double &initialAngle);

    virtual void startOval();
    virtual void startEigth();
    virtual void startScan();
    virtual void startWaypoint();
    virtual void startStayAt();

    Coord initCircle;

    enum Mode mode;

    Coord destination;
    Coord origin;



    double borderX;
    double borderY;
    double borderZ;
    double r;
    int scanLines;

    rad startAngle;
    rad acummulateAngle;
    double speed = 0;
    double linDist = 0;

    /** @brief angular velocity [rad/s], derived from speed and radius. */
    double omega;

    /** @brief Direction from the center of the circle. */
    rad angle;

    enum PartialState {
        Init,
        Lineal,
        Circle
    };

    PartialState partialState;
    simtime_t endPattern;
    Coord targetPosition;
    bool down = false;
    bool rigth = false;

  protected:

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Move the host according to the current simulation time. */
    virtual void move() override;


    simtime_t computeStayAt(const double &sp, const double &radious, const int& endDregree);
    simtime_t computeOvalTime(const double &sp, const double &radious, const double &linDist);
    simtime_t computeScanTime(const double &sp, const double &radious, const double &linDist, const int &);

  public:
    virtual double getMaxSpeed() const override { return speed; }

    virtual Quaternion getCurrentAngularVelocity() override { return Quaternion(EulerAngles(rad(omega), rad(0), rad(0))); }
    virtual Quaternion getCurrentAngularAcceleration() override { return Quaternion(); }
};

} // namespace inet

#endif // ifndef __INET_CIRCLEMOBILITY_H

