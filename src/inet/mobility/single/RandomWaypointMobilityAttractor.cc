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

/*
 * <configuration>
 * //default configuration
 * <common>
 * <attractConf  id = "type_1">
 * <type = "attractor" />
 * <behaviorType = "nearest"/> --"nearest" "incremental", "landscape",  "incremental"  if not attractor is present in the area, what must select, the nearest, increase the area until there are attractors or select a random point in the landscape, default, nearest.
 * <Xdistribution = "uniform()" />
 * <Ydistribution = "uniform()" />
 * <Zdistribution = "uniform()" />
 * <proximity distance="100" repeat = "5" />   -- close attractor
 * <proximity distance="200" repeat = "3" />   -- second close
 * <proximity distance="300" repeat = "2" />   -- third close
 * </attractConf>
 * <attractConf  id = "type_2">
 * <type = "attractor" />
 * <behaviourType = "nearest"/> -- "incremental", "landscape",  "incremental"  if not attractor is present in the area, what must select, the nearest, increase the area until there are attractors or select a random point in the landscape, default if not
 * <Xdistribution = "uniform()" />
 * <Ydistribution = "uniform()" />
 * <Zdistribution = "uniform()" />
 * <proximity distance="200" repeat = "4" />   -- close attractor
 * <proximity distance="300" repeat = "3" />   -- second close
 * <proximity distance="400" repeat = "2" />   -- third close
 * <proximity distance= "MAX" repeat = "1" />  -- any point
 * </attractConf>
 * <attractConf  id = "type_3">
 * <type = "landscape" />
 * <Xdistribution = "uniform($MINX,$MAXX)" />
 * <Ydistribution = "uniform($MINY,$MAXY)" />
 * <Zdistribution = "uniform($MINZ,$MAXZ)" />
 * </attractConf>
 * </common>
 * <attractor x="xxx" y="xxx" z="xxx" AttractorId="type_1">  -- set the position of the attractor
 * <freeAttractor x="xxx" y="xxx" z="xxx" Xdistribution = "uniform()" Ydistribution = "uniform()" Zdistribution = "uniform()" repetition = "3">  -- set the position of the attractor, this is free list, the nodes that doesn't have a type assigned or the type doesn't exist, will use this
 * </configuration>
 *
 */

#include "inet/mobility/single/RandomWaypointMobilityAttractor.h"
#include <algorithm>

namespace inet {

Define_Module(RandomWaypointMobilityAttractor);

cDynamicExpression *RandomWaypointMobilityAttractor::getValue(cXMLElement *statement)
{
    // first, textually replace $MAXX and $MAXY with their actual values
    const char *s = statement->getNodeValue();
    std::string str;
    if (strchr(s, '$')) {
        char strMinX[32], strMinY[32], strMinZ[32];
        char strMaxX[32], strMaxY[32], strMaxZ[32];
        sprintf(strMinX, "%g", constraintAreaMin.x);
        sprintf(strMinY, "%g", constraintAreaMin.y);
        sprintf(strMinZ, "%g", constraintAreaMin.z);
        sprintf(strMaxX, "%g", constraintAreaMax.x);
        sprintf(strMaxY, "%g", constraintAreaMax.y);
        sprintf(strMaxZ, "%g", constraintAreaMax.z);

        str = s;
        std::string::size_type pos;

        while ((pos = str.find("$MINX")) != std::string::npos)
            str.replace(pos, sizeof("$MINX") - 1, strMinX);

        while ((pos = str.find("$MINY")) != std::string::npos)
            str.replace(pos, sizeof("$MINY") - 1, strMinY);

        while ((pos = str.find("$MINZ")) != std::string::npos)
            str.replace(pos, sizeof("$MINZ") - 1, strMinZ);

        while ((pos = str.find("$MAXX")) != std::string::npos)
            str.replace(pos, sizeof("$MAXX") - 1, strMaxX);

        while ((pos = str.find("$MAXY")) != std::string::npos)
            str.replace(pos, sizeof("$MAXY") - 1, strMaxY);

        while ((pos = str.find("$MAXZ")) != std::string::npos)
            str.replace(pos, sizeof("$MAXZ") - 1, strMaxZ);

        s = str.c_str();
    }

    // then use cDynamicExpression to evaluate the string
    try {
        cDynamicExpression *expr = new cDynamicExpression();
        expr->parse(s);
        // check
        double val = expr->doubleValue(this);
        return expr;
    }
    catch (std::exception& e) {
        throw cRuntimeError("Wrong value '%s' around %s: %s", s,
                statement->getSourceLocation(), e.what());
    }
}


void RandomWaypointMobilityAttractor::parseAttractor(cXMLElement *nodes)
{
    cXMLElementList childs = nodes->getChildren();

    std::string id(nodes->getAttribute("id"));
    AttractorConf conf;
    for (auto & child : childs)
    {
        const char *tag = child->getTagName();
        if (!strcmp(tag, "type")) {
            const char * s = child->getNodeValue();
            if (!strcmp(s, "attractor"))
                conf.type = ATTRACTOR;
            else if (!strcmp(s, "landscape"))
                conf.type = LANDSCAPE;
            else
                throw cRuntimeError("Wrong value '%s' ", s);
        }
        else if (!strcmp(tag, "Xdistribution")) {
            conf.distX = getValue(child);
        }
        else if (!strcmp(tag, "Ydistribution")) {
            conf.distY = getValue(child);
        }
        else if (!strcmp(tag, "Zdistribution")) {
            conf.distZ = getValue(child);
        }
        else if (!strcmp(tag, "proximity")) {
            double distance = atof(child->getAttribute("distance"));
            int prob = atof(child->getAttribute("repeat"));
            AttractorData data;
            data.distance = distance;
            data.probability = prob;
            conf.attractorData.push_back(data);
            conf.totalRep += prob;
        }
        else if (!strcmp(tag, "behaviorType")) {
            std::string str(child->getNodeValue());
            if (str != "nearest" && str != "incremental" && str != "landscape")
                throw cRuntimeError("behaviourType '%s' not allowed", str.c_str());
            conf.behavior = str;
        }
    }
    attractorConf[id] = conf;
}


void RandomWaypointMobilityAttractor::parseXml(cXMLElement *nodes)
{
    // Recursively traverse the whole config file, looking for
    // speed attributes
    cXMLElementList childs = nodes->getChildren();
    for (auto & child : childs)
    {
        const char *tag = child->getTagName();
        if (!strcmp(tag, "common")) {
            cXMLElementList childsCommon = child->getChildren();
            for (auto & childCommon : childsCommon) {
                const char *tag = childCommon->getTagName();
                if (!strcmp(tag, "attractConf")) {
                    parseAttractor(childCommon);
                }
            }
        }
        if (!strcmp(tag, "attractor")) {
            try {
                std::string x(child->getAttribute("x"));
                std::string y(child->getAttribute("y"));
                std::string z(child->getAttribute("z"));
                std::string type(child->getAttribute("AttractorId"));
                Coord point;
                if (!x.empty())
                    point.x = std::stod(x);
                if (!y.empty())
                    point.y = std::stod(y);
                if (!z.empty())
                    point.z = std::stod(z);
                // first check if exist
                auto it = attractorConf.find(type);
                if (it == attractorConf.end())
                    throw cRuntimeError("Doesn't exist a configured attractor with id '%s'", type.c_str());
                listsAttractors[type].push_back(point);
            }
            catch (std::exception& e) {
                throw cRuntimeError("Wrong value around %s: %s",
                        child->getSourceLocation(), e.what());
            }
        }
        else if (!strcmp(tag, "freeAttractor")) {
            std::string x(child->getAttribute("x"));
            std::string y(child->getAttribute("y"));
            std::string z(child->getAttribute("z"));
            std::string repetition(child->getAttribute("repetition"));
            std::string Xdistribution(child->getAttribute("Xdistribution"));
            std::string Ydistribution(child->getAttribute("Ydistribution"));
            std::string Zdistribution(child->getAttribute("Zdistribution"));
            PointData point;
            if (!x.empty())
                point.pos.x = std::stod(x);
            if (!y.empty())
                point.pos.y = std::stod(y);
            if (!z.empty())
                point.pos.z = std::stod(z);
            if (!repetition.empty())
                point.repetition = std::stod(repetition);
            if (!Xdistribution.empty()) {
                cDynamicExpression *expr = new cDynamicExpression();
                expr->parse(Xdistribution.c_str());
                point.distX = expr;
            }
            if (!Ydistribution.empty()) {
                cDynamicExpression *expr = new cDynamicExpression();
                expr->parse(Ydistribution.c_str());
                point.distY = expr;
            }
            if (!Zdistribution.empty()) {
                cDynamicExpression *expr = new cDynamicExpression();
                expr->parse(Zdistribution.c_str());
                point.distZ = expr;
            }
            freeAtractors.push_back(point);
        }
    }
}

Coord RandomWaypointMobilityAttractor::getNewCoord()
{
    auto itAttractors = listsAttractors.find(typeAttractor);
    if (itAttractors == listsAttractors.end() || itAttractors->second.empty()) {
        return getRandomPosition();
    }
    auto itconf = attractorConf.find(typeAttractor);
    if (itconf == attractorConf.end())
        throw cRuntimeError("Doesn't exist a configured attractor with id '%s'", typeAttractor.c_str());

    if (itconf->second.attractorData.empty())
        return getRandomPosition();

    // First, compute probabilities.
    double prob = uniform(0, itconf->second.totalRep);
    double repetitions = 0;
    unsigned int pos = 0;
    for (auto elem : itconf->second.attractorData) {
        if (repetitions < prob)
            break;
        repetitions += elem.probability;
        pos++;
    }

    if (pos == itconf->second.attractorData.size())
        pos--;

    std::vector<Coord> attractors;
    double distance = itconf->second.attractorData[pos].distance;
    do {
        for (const auto &elem : itAttractors->second) {
            if (elem.distance(this->getCurrentPosition()) < distance) {
                attractors.push_back(elem);
            }
        }
        if (attractors.empty()) {
            // check behavior
            if (itconf->second.behavior == "landscape") {
                return getRandomPosition();
            }
            if (itconf->second.behavior == "nearest") {
                double distNearest = std::numeric_limits<double>::max();
                Coord pos = Coord::NIL;
                for (const auto &elem : itAttractors->second) {
                    if (elem.distance(this->getCurrentPosition()) < distNearest) {
                        pos = elem;
                        distNearest = elem.distance(this->getCurrentPosition());
                    }
                }
                if (pos == Coord::NIL)
                    throw cRuntimeError("Impossible to find an attractor with id '%s'", typeAttractor.c_str());
                attractors.push_back(pos);
            }
            else {
                // increment the area.
                pos++;
                if (pos >= itconf->second.attractorData.size()) {
                    return getRandomPosition();
                }
                distance = itconf->second.attractorData[pos].distance;
                for (const auto &elem : itAttractors->second) {
                    if (elem.distance(this->getCurrentPosition()) < distance) {
                        attractors.push_back(elem);
                    }
                }
            }
        }
    } while(attractors.empty());

    // choose an attractor in the list
    Coord posAttractor = attractors[intuniform(0, attractors.size()-1)];
    // now select the position in function of the distributions.
    if (itconf->second.distX)
        posAttractor.x += itconf->second.distX->evaluate(this).doubleValue();
    if (itconf->second.distY)
        posAttractor.y += itconf->second.distY->evaluate(this).doubleValue();
    if (itconf->second.distZ)
        posAttractor.z += itconf->second.distZ->evaluate(this).doubleValue();
    // Check that the value is in the landscape,

    return posAttractor;
}

RandomWaypointMobilityAttractor::~RandomWaypointMobilityAttractor()
{
    for (auto elem : attractorConf) {
        if (elem.second.distX)
            delete elem.second.distX;
        if (elem.second.distY)
            delete elem.second.distY;
        if (elem.second.distZ)
            delete elem.second.distZ;
    }
    attractorConf.clear();
    for (auto elem : freeAtractors) {
        if (elem.distX)
            delete elem.distX;
        if (elem.distY)
            delete elem.distY;
        if (elem.distZ)
            delete elem.distZ;
    }
    freeAtractors.clear();
}

RandomWaypointMobilityAttractor::RandomWaypointMobilityAttractor()
{
    nextMoveIsWait = false;
}

void RandomWaypointMobilityAttractor::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        waitTimeParameter = &par("waitTime");
        hasWaitTime = waitTimeParameter->isExpression() || waitTimeParameter->doubleValue() != 0;
        speedParameter = &par("speed");
        stationary = !speedParameter->isExpression() && speedParameter->doubleValue() == 0;
        parseXml(par("xmlConfiguration"));
        // Order the distances ring
        for (auto elem : attractorConf) {
            std::sort(elem.second.attractorData.begin(), elem.second.attractorData.end());
        }
    }
}

void RandomWaypointMobilityAttractor::setTargetPosition()
{
    if (nextMoveIsWait) {
        simtime_t waitTime = waitTimeParameter->doubleValue();
        nextChange = simTime() + waitTime;
        nextMoveIsWait = false;
    }
    else {
        targetPosition = getRandomPosition();
        double speed = speedParameter->doubleValue();
        double distance = lastPosition.distance(targetPosition);
        simtime_t travelTime = distance / speed;
        nextChange = simTime() + travelTime;
        nextMoveIsWait = hasWaitTime;
    }
}

void RandomWaypointMobilityAttractor::move()
{
    LineSegmentsMobilityBase::move();
    raiseErrorIfOutside();
}

double RandomWaypointMobilityAttractor::getMaxSpeed() const
{
    return speedParameter->isExpression() ? NaN : speedParameter->doubleValue();
}

} // namespace inet

