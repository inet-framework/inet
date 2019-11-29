//
// Copyright (C) 2005 Georg Lutz, Institut fuer Telematik, University of Karlsruhe
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

#ifndef __INET_RANDOMWPMOBILITYATRACTTOR_H
#define __INET_RANDOMWPMOBILITYATRACTTOR_H

#include "inet/common/INETDefs.h"
#include "inet/mobility/base/LineSegmentsMobilityBase.h"
#include <vector>

namespace inet {

/**
 * Random Waypoint mobility model. See NED file for more info.
 *
 * @author Georg Lutz (georglutz AT gmx DOT de), Institut fuer Telematik,
 *  Universitaet Karlsruhe, http://www.tm.uka.de, 2004-2005
 * @author Andras Varga (generalized, ported to LineSegmentsMobilityBase)
 * @author Alfonso Ariza, Universidad de Málaga 2019
 */

class INET_API RandomWaypointMobilityAttractor : public LineSegmentsMobilityBase
{
    enum TypeAttractor {ATTRACTOR, LANDSCAPE};
  protected:

    bool nextMoveIsWait;
    cPar *speedParameter = nullptr;
    cPar *waitTimeParameter = nullptr;
    bool hasWaitTime;
    cXMLElement *attractorScript;

    struct AttractorData {
        double distance = NaN;
        double probability = NaN;
        AttractorData operator = (const AttractorData &a){
            distance = a.distance;
            probability = a.probability;
            return *this;
        }
        bool operator == (const AttractorData &a){
            return (distance == a.distance);
        }
        bool operator < (const AttractorData &a){
            return (distance < a.distance);
        }
        bool operator <= (const AttractorData &a){
            return (distance <= a.distance);
        }
        bool operator > (const AttractorData &a){
            return (distance > a.distance);
        }
        bool operator >= (const AttractorData &a){
            return (distance >= a.distance);
        }
    };

    struct AttractorConf {
        TypeAttractor type;
        std::string behavior = std::string("nearest");
        cDynamicExpression *distX = nullptr;
        cDynamicExpression *distY = nullptr;
        cDynamicExpression *distZ = nullptr;
        std::vector<AttractorData> attractorData;
        double totalRep = 0;
    };

    struct PointData {
          Coord pos;
          double repetition = 0;
          cDynamicExpression *distX = nullptr;
          cDynamicExpression *distY = nullptr;
          cDynamicExpression *distZ = nullptr;
     };

    std::vector<AttractorConf> attractorsDefaultConf;

    typedef std::vector<Coord> ListAttactor;
    std::map<std::string, ListAttactor> listsAttractors;
    std::map<std::string, AttractorConf> attractorConf;

    std::vector<PointData> freeAtractors;

    ListAttactor relativeList; // can change

    void parseXml(cXMLElement *nodes);
    void parseAttractor(cXMLElement *nodes);
    cDynamicExpression *getValue(cXMLElement *statement);
    Coord getNewCoord();

    std::string attractorId;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void setTargetPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void move() override;

  public:
    RandomWaypointMobilityAttractor();
    ~RandomWaypointMobilityAttractor();
    virtual double getMaxSpeed() const override;
};

} // namespace inet

#endif // ifndef __INET_RANDOMWPMOBILITY_H

