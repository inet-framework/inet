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

#include "inet/common/INETMath.h"
#include "inet/mobility/single/PaparazziMobility.h"

namespace inet {

Define_Module(PaparazziMobility);

EXECUTE_ON_STARTUP(
        cEnum * e = cEnum::find("inet::PaparazziMobilityModels");
        if (!e)
            enums.getInstance()->add(e = new cEnum("inet::PaparazziMobilityModels"));
        e->insert(PaparazziMobility::WAYPOINT, "waypoint");
        e->insert(PaparazziMobility::STAYAT, "stayat");
        e->insert(PaparazziMobility::SCAN, "scan");
        e->insert(PaparazziMobility::OVAL, "oval");
        e->insert(PaparazziMobility::EIGHT, "eigth");
        );

PaparazziMobility::PaparazziMobility()
{
    r = -1;
    startAngle = deg(0);
    speed = 0;
    omega = 0;
    angle = deg(0);
}

PaparazziMobility::Mode PaparazziMobility::getNextState() {
    if (randomSeq) {
        int index = intuniform(0, sequenceModels.size() -1);
        return sequenceModels[index];
    }
    else {
        if (latestSeq == -1) {
            latestSeq = 0;
        }
        else
            latestSeq++;
        if (latestSeq >= sequenceModels.size())
            latestSeq = 0;
        return sequenceModels[latestSeq];
    }
}

void PaparazziMobility::startNextState() {
    Mode mode = getNextState();
    startState(mode);
}


void PaparazziMobility::startState(const PaparazziMobility::Mode &s) {
    if (STAYAT == s)
        startStayAt();
    else if (WAYPOINT == s)
        startWaypoint();
    else if (EIGHT == s)
        startEigth();
    else if (SCAN == s)
        startScan();
    else if (OVAL == s)
        startOval();
    else
        throw cRuntimeError("Mode invalid");

}

void PaparazziMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);

    EV_TRACE << "initializing PaparazziMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        borderX = par("borderX");
        borderY = par("borderY");
        borderZ = par("borderZ");
        r = par("r");
        ASSERT(r > 0);
        startAngle = deg(par("startAngle"));
        speed = par("speed");
        omega = speed / r;
        stationary = (omega == 0);

        // if the node is out of the limits, search a position inside the limits
        const char *sequence = par("sequence");
        cStringTokenizer tokenizer(sequence);
        const char *token;

        while ((token = tokenizer.nextToken()) != nullptr) {
            int state = cEnum::get("inet::PaparazziMobilityModels")->lookup(token);
            if (state == -1)
                throw cRuntimeError("Invalid statate: '%s'", token);
            sequenceModels.push_back(Mode(state));
        }
        latestSeq = -1;
        randomSeq = par("randomSequence");

        if (sequenceModels.empty())
            throw cRuntimeError("No valid sequence '%s'", sequence);
    }
    else if (stage == INITSTAGE_SINGLE_MOBILITY)
        startNextState();
}

void PaparazziMobility::setInitialPosition()
{
    MovingMobilityBase::setInitialPosition();
    if (lastPosition.x-constraintAreaMin.x < borderX)
        lastPosition.x = borderX;
    if (constraintAreaMax.x-lastPosition.x < borderX )
        lastPosition.x = constraintAreaMax.x - borderX;
    if (lastPosition.y-constraintAreaMin.y < borderY)
        lastPosition.y = borderY;
    if (constraintAreaMax.y-lastPosition.y < borderY )
        lastPosition.y = constraintAreaMax.y - borderY;
}



Coord PaparazziMobility::randomPosition(void) const{
    Coord posi(0,0,0);
    posi.x = uniform(constraintAreaMin.x+borderX, constraintAreaMax.x-borderX);
    posi.y = uniform(constraintAreaMin.y+borderY, constraintAreaMax.y-borderY);
    if (constraintAreaMax.z > 0)
        posi.z = uniform(constraintAreaMin.z + borderZ, constraintAreaMax.z - borderZ);
    return posi;
}

void PaparazziMobility::circularMouvement(const double &sp, const double &radious, double initAngle){
    simtime_t t = simTime() - lastChange;
    double o = sp / radious;
    angle = acummulateAngle + rad(o * t.dbl());
    double cosAngle = cos(rad(angle).get());
    double sinAngle = sin(rad(angle).get());
    lastPosition.x = initCircle.x + radious * cosAngle;
    lastPosition.y = initCircle.y - radious * sinAngle;
    lastVelocity.x = -sinAngle * speed;
    lastVelocity.y = cosAngle * speed;
    lastVelocity.z = 0;
}



void PaparazziMobility::moveLinear()
{
    simtime_t now = simTime();
    if (now > lastUpdate) {
        ASSERT(nextChange == -1 || now < nextChange);
        lastPosition += lastVelocity * (now - lastUpdate).dbl();
    }
}


void PaparazziMobility::moveOval(const double &sp, const double &radious, const double &linDist, const double &initialAngle)
{
    simtime_t now = simTime();
    if (partialState == Init) {
        partialState = Lineal;
        targetPosition = lastPosition;
        // check distance to the borders
        lastVelocity.y = 0;
        lastVelocity.z = 0;
        if (targetPosition.x - constraintAreaMin.x  < constraintAreaMax.x - targetPosition.x) {
            targetPosition.x += linDist;
            lastVelocity.x = sp;
            rigth = true;
        }
        else {
            targetPosition.x -= linDist;
            lastVelocity.x = -sp;
            rigth = false;
        }
        initCircle = lastPosition;
        if (targetPosition.y - constraintAreaMin.y < constraintAreaMax.y - targetPosition.y) {
            down = true;
            initCircle.y += radious;
            acummulateAngle = rad(PI/2);
        }
        else {
            down = false;
            initCircle.y -= radious;
            acummulateAngle = rad(3*PI/2);
        }
        // compute the center position of both circles.
        center1 = initCircle;
        center2 = initCircle;
        if (rigth)
            center2.x += linDist;
        else
            center2.x -= linDist;

        // angle proyection;
        nextChange = simTime() + linDist / sp;
    }

    if (nextChange == simTime()) {
        lastChange = simTime();
        if (partialState == Lineal) {
            partialState = Circle;

            initCircle = lastPosition;
            double omega = std::abs(sp/radious);
            nextChange = simTime() + std::abs(PI / omega);
            if (nextChange > endPattern)
                nextChange = endPattern;

            if (down) {
                acummulateAngle = rad(PI/2);
                initCircle.y += radious;
            }
            else {
                acummulateAngle = rad(3*PI/2);
                initCircle.y -= radious;
            }

            // correct the center position of the circle
            if (initCircle.distance(center1) < initCircle.distance(center2))
                initCircle = center1;
            else
                initCircle = center2;
            rigth = !rigth;
        }
        else if (partialState == Circle) {
            down = !down;
            partialState = Lineal;
            targetPosition = lastPosition;
            if (rigth) {
                targetPosition.x += linDist;
                // correct position
                lastVelocity.x = speed;
            }
            else {
                targetPosition.x -= linDist;
                lastVelocity.x = -speed;
            }
// correct errors in the position
            if (std::abs(targetPosition.x - center1.x) < std::abs(targetPosition.x - center2.x))
                targetPosition.x  = center1.x;
            else
                targetPosition.x  = center2.x;

            if (std::abs(targetPosition.y - (center1.y+radious)) < std::abs(targetPosition.y - (center1.y - radious)))
                targetPosition.y  = center1.y + radious;
            else
                targetPosition.y  = center1.y - radious;

            lastUpdate = simTime();
            nextChange = simTime() + std::abs(targetPosition.distance(lastPosition) / sp);
        }
    }
    if (now > lastUpdate) {
        if (partialState == Circle) {
            simtime_t t = now - lastChange;
            double omega = std::abs(sp / radious);
            if ((down && !rigth) || (!down && rigth)) {
                omega *= -1;
            }
            rad ang = acummulateAngle + rad(omega * t.dbl());

            double cosAngle = cos(rad(ang).get());
            double sinAngle = sin(rad(ang).get());
            if (t > 0) {
                lastPosition.x = initCircle.x + radious * cosAngle;
                lastPosition.y = initCircle.y - radious * sinAngle;
            }
            lastVelocity.x = -sinAngle * sp;
            lastVelocity.y = cosAngle * sp;
            lastVelocity.z = 0;
        }
        else if (partialState == Lineal) {
            lastPosition += lastVelocity * (now - lastUpdate).dbl();
        }
    }
}

void PaparazziMobility::moveEight(const double &sp, const double &radious, const double &linDist, const double &initialAngle) {

    double val = (linDist*linDist) - (4*radious*radious);
    if (val < 0)
        throw cRuntimeError("Error moveEight");

    double disX = std::sqrt(val);
    double angle = std::asin(2*radious/linDist);

    simtime_t now = simTime();
    if (partialState == Init) {
        partialState = Lineal;
        targetPosition = lastPosition;
        // check distance to the borders
        lastVelocity.y = 0;
        lastVelocity.z = 0;
        originLimit = origin;
        if (targetPosition.y - constraintAreaMin.y  < constraintAreaMax.y - targetPosition.y) {
            down = true;
            targetPosition.y += 2*radious;
            lastVelocity.y = sp * std::abs(std::sin(angle));
        }
        else {
            down = false;
            targetPosition.y -= 2*radious;
            lastVelocity.y = - sp * std::abs(std::sin(angle));
        }

        if (targetPosition.x - constraintAreaMin.x  < constraintAreaMax.x - targetPosition.x) {
            targetPosition.x += disX;
            lastVelocity.x = sp * std::abs(std::cos(angle));
            rigth = true;
        }
        else {
            targetPosition.x -= disX;
            lastVelocity.x = -sp * std::abs(std::cos(angle));
        }
        initCircle = lastPosition;
        if (down) {
            acummulateAngle = rad(PI);
            initCircle.y += radious;
        }
        else {
            initCircle.y -= radious;
            acummulateAngle = rad(3*PI/2);
        }

        center1 = initCircle;
        center2 = initCircle;
        if (rigth)
            center2.x += disX;
        else
            center2.x -= disX;

        double distance = lastPosition.distance(targetPosition);
        nextChange = simTime() + distance / sp;
    }

    if (nextChange == simTime()) {
        lastChange = simTime();
        if (partialState == Lineal) {
            partialState = Circle;
            initCircle = lastPosition;
            double omega = sp / radious;
            rigth = !rigth;
            if (!down) {
                acummulateAngle = rad(PI/2);
                initCircle.y += radious;
            }
            else {
                acummulateAngle = rad(3*PI/2);
                initCircle.y -= radious;
            }

            // correct the center position of the circle
            if (initCircle.distance(center1) < initCircle.distance(center2))
                initCircle = center1;
            else
                initCircle = center2;

            nextChange = simTime() + std::abs(PI / omega);
            if (nextChange > endPattern)
                nextChange = endPattern;
        }
        else if (partialState == Circle) {
            //down = !down;
            partialState = Lineal;

            // compute next target position
            targetPosition = lastPosition;
            if (rigth)
                targetPosition.x += disX;
            else
                targetPosition.x -= disX;
            if (down)
                targetPosition.y += 2*radious;
            else
                targetPosition.y -= 2*radious;

            // correct position
            if (std::abs(targetPosition.x - center1.x) < std::abs(targetPosition.x - center2.x))
                targetPosition.x  = center1.x;
            else
                targetPosition.x  = center2.x;

            if (std::abs(targetPosition.y - (center1.y+radious)) < std::abs(targetPosition.y - (center1.y - radious)))
                targetPosition.y  = center1.y + radious;
            else
                targetPosition.y  = center1.y - radious;

            // compute speed
            if (targetPosition.x - lastPosition.x == 0)
                angle = PI/2;
            else
                angle = std::atan(std::abs(targetPosition.y - lastPosition.y)/std::abs(targetPosition.x - lastPosition.x));

            lastVelocity.x = sp*std::abs(std::cos(angle));
            lastVelocity.y = sp*std::abs(std::sin(angle));
            if (!rigth)
                lastVelocity.x *=-1;
            if (!down)
                lastVelocity.y *= -1;
            nextChange = simTime() + std::abs(targetPosition.distance(lastPosition) / sp);
            // angle projection;
            lastUpdate = simTime();

            if (nextChange > endPattern)
                nextChange = endPattern;
        }
    }
    if (now > lastUpdate) {
        if (partialState == Circle) {
            simtime_t t = now - lastChange;
            double omega = std::abs(sp / radious);
            if ((!down && !rigth) || (down && rigth))
                omega *=-1;

            rad ang = acummulateAngle + rad(omega * t.dbl());

            double cosAngle = cos(rad(ang).get());
            double sinAngle = sin(rad(ang).get());

            if (t > 0) {
                lastPosition.x = initCircle.x + radious * cosAngle;
                lastPosition.y = initCircle.y - radious * sinAngle;
            }
            lastVelocity.x = -sinAngle * sp;
            lastVelocity.y = cosAngle * sp;
            lastVelocity.z = 0;
        }
        else if (partialState == Lineal) {
            lastPosition += lastVelocity * (now - lastUpdate).dbl();
        }
    }
}

void PaparazziMobility::moveScan(const double &sp, const double &radious, const double &linDist,const int &hLines , const double &initialAngle) {

    simtime_t now = simTime();
    if (partialState == Init) {
        partialState = Lineal;
        targetPosition = lastPosition;
        lastVelocity.y = 0;
        lastVelocity.z = 0;
        // check distance to the borders
        if (targetPosition.x - constraintAreaMin.x  < constraintAreaMax.x - targetPosition.x) {
            targetPosition.x += linDist;
            lastVelocity.x = sp;
            rigth = true;
        }
        else {
            targetPosition.x -= linDist;
            lastVelocity.x = -sp;
            rigth = false;
        }
        if (targetPosition.y - constraintAreaMin.y < constraintAreaMax.y - targetPosition.y)
            down = true;
        else
            down = false;

        if (down) {
            acummulateAngle = rad(PI/2);
            initCircle.y += radious;
        }
        else {
            acummulateAngle = rad(3*PI/2);
            initCircle.y -= radious;
        }
        // angle projection;
        nextChange = simTime() + linDist / sp;
    }

    if (nextChange == simTime()) {
        lastChange = simTime();
        if (partialState == Lineal) {
            partialState = Circle;

            initCircle = lastPosition;
            double omega = std::abs(sp / radious);
            nextChange = simTime() + (PI / omega);
            rigth = !rigth;
            if (down) {
                acummulateAngle = rad(PI/2);
                initCircle.y += radious;
            }
            else {
                acummulateAngle = rad(3*PI/2);
                initCircle.y -= radious;
            }
            if (nextChange > endPattern)
                nextChange = endPattern;
        }
        else if (partialState == Circle) {
            partialState = Lineal;
            targetPosition = lastPosition;
            if (rigth) {
                lastVelocity.x = sp;
                targetPosition.x += linDist;
            }
            else {
                targetPosition.x -= linDist;
                lastVelocity.x = -sp;
            }
            nextChange = simTime() + linDist / sp;
            if (nextChange > endPattern)
                nextChange = endPattern;
        }
    }

    if (now > lastUpdate) {
        if (partialState == Circle) {
            simtime_t t = now - lastChange;
            double omega = std::abs(sp / radious);
            if ((down && !rigth ) || (!down && rigth))
                omega *= -1;

            rad ang = acummulateAngle + rad(omega * t.dbl());
            double cosAngle = cos(rad(ang).get());
            double sinAngle = sin(rad(ang).get());
            if (t > 0) {
                lastPosition.x = initCircle.x + radious * cosAngle;
                lastPosition.y = initCircle.y - radious * sinAngle;
            }
            lastVelocity.x = -sinAngle * sp;
            lastVelocity.y = cosAngle * sp;
            lastVelocity.z = 0;
        }
        else if (partialState == Lineal) {
            lastPosition += lastVelocity * (now - lastUpdate).dbl();
        }
    }
}

simtime_t PaparazziMobility::computeStayAt(const double &sp, const double &radious, const int& endDregree){
    double omega = sp / radious;
    double endAngle = 2*PI*endDregree/360;
    simtime_t end = endAngle/ omega;
    return end;
}


simtime_t PaparazziMobility::computeOvalTime(const double &sp, const double &radious, const double &linDist) {
    double o = sp / radious;
    simtime_t cirCleTime = (2 * PI)/o;
    simtime_t lineTime = 2*linDist/sp;
    return cirCleTime + lineTime;
}

simtime_t PaparazziMobility::computeScanTime(const double &sp, const double &radious, const double &linDist, const int &hori) {
    double o = sp / radious;
    simtime_t halfCirCleTime = PI/o;
    simtime_t lineTime = linDist/sp;
    if (hori > 1)
        return (hori-1 * halfCirCleTime) + (hori*lineTime);
    else
        return lineTime;

}

void PaparazziMobility::startStayAt() {
    mode = STAYAT;
    speed = par("speed");
    r = par("r");
    acummulateAngle = rad(PI/2);
    endPattern = simTime() + computeStayAt(speed, r, 360);
    nextChange = endPattern;
    origin = lastPosition;
    partialState = Init;
    lastChange = simTime();
    initCircle = lastPosition;
    initCircle.y +=r;
    circularMouvement(speed, r, 0);
}

void PaparazziMobility::startOval() {
    mode = OVAL;
    partialState = Init;
    acummulateAngle = rad(PI/2);
    linDist = par("linearDist"); // configuration parameter or random?
    speed = par("speed");
    r = par("r");
    endPattern = simTime() + computeOvalTime(speed, r, linDist);
    origin = lastPosition;
    lastChange = simTime();
    moveOval(speed, r, linDist, 0);
}

void PaparazziMobility::startEigth() {
    mode=EIGHT;
    partialState = Init;
    acummulateAngle = rad(PI/2);
    linDist = par("linearDist"); // configuration parameter or random?
    speed = par("speed");
    r = par("r");
    endPattern = simTime() + computeOvalTime(speed, r, linDist);
    origin = lastPosition;
    lastChange = simTime();
    moveEight (speed, r, linDist, 0);
}

void PaparazziMobility::startScan() {
    mode=SCAN;
    partialState = Init;
    acummulateAngle = rad(PI/2);
    linDist = par("linearDist"); // configuration parameter or random?
    speed = par("speed");
    r = par("r");
    scanLines = par("ScanLines");
    endPattern = simTime() + computeScanTime(speed, r, linDist, scanLines);
    origin = lastPosition;
    lastChange = simTime();
    moveScan(speed, r, linDist, scanLines, 0);
}


void PaparazziMobility::startWaypoint() {
    mode = WAYPOINT;
    partialState = Init;
    speed = par("speed");
    targetPosition = randomPosition();
    double distance = lastPosition.distance(targetPosition);
    simtime_t travelTime = distance / speed;
    lastVelocity = (targetPosition - lastPosition) / travelTime.dbl();
    nextChange = simTime() + travelTime;
    endPattern = nextChange;
}



void PaparazziMobility::move()
{
    if (fisrtTime) {
        fisrtTime = false;
        return;
    }
    simtime_t now = simTime();
    if (mode == WAYPOINT){
        if (now == nextChange) {
            startNextState();
        }
        else
            moveLinear();
    }
    else if (mode==STAYAT){
        if(now == nextChange){
            startNextState();
        }
        else {
            circularMouvement(speed, r, 0);
        }
    }
    else if (mode == OVAL){
        if (endPattern <= now) {
            if (par("correctPositionErrors"))
                lastPosition = origin; // remove navigation errors
            startNextState();
        }
        else{
            moveOval(speed, r, linDist, 0);
        }
    }
    else if (mode == EIGHT){
        if (endPattern <= now) {
            if (par("correctPositionErrors"))
                lastPosition = origin; // remove navigation errors
            startNextState();
        }
        else
            moveEight (speed, r, linDist, 0);
    }
    else if (mode == SCAN){
        if (endPattern <= now)
            startNextState();
        else
            moveScan(speed, r, linDist, scanLines , 0);
    }

    else {
        throw cRuntimeError("PaparazziMobility mode patter invalid");
    }

    // do something if we reach the wall
    Coord dummyCoord;
    handleIfOutside(REFLECT, dummyCoord, dummyCoord);
}

} // namespace inet

