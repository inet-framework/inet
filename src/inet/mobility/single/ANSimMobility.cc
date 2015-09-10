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

#include "inet/mobility/single/ANSimMobility.h"
#include "inet/common/INETMath.h"

namespace inet {

Define_Module(ANSimMobility);

static cXMLElement *firstChildWithTag(cXMLElement *node, const char *tagname)
{
    cXMLElement *child = node->getFirstChild();
    while (child && strcmp(child->getTagName(), tagname) != 0)
        child = child->getNextSibling();

    if (!child)
        throw cRuntimeError("Element <%s> has no <%s> child at %s",
                node->getTagName(), tagname, node->getSourceLocation());

    return child;
}


void ANSimMobility::computeMaxSpeed()
{
    cXMLElement *rootElem = par("ansimTrace");
    cXMLElement *curElem = rootElem->getElementByPath("mobility/position_change");
    if (!curElem)
        throw cRuntimeError("Element doesn't have <mobility> child or <position_change> grandchild at %s",
                rootElem->getSourceLocation());
    cXMLElement *nextElem = findNextPositionChange(curElem);
    if (!nextElem)
        throw cRuntimeError("Element doesn't have second <position_change> grandchild at %s",
                curElem->getSourceLocation());
    curElem = nextElem;
    cXMLElement *destElem = firstChildWithTag(curElem, "destination");
    const char *xStr = firstChildWithTag(destElem, "xpos")->getNodeValue();
    const char *yStr = firstChildWithTag(destElem, "ypos")->getNodeValue();
    Coord lastPos(atof(xStr), atof(yStr), 0);
    // first position is the initial position so we don't take it into account to compute maxSpeed
    if (curElem)
        curElem = curElem->getNextSibling();
    while (curElem)
    {
        cXMLElement *destElem = firstChildWithTag(curElem, "destination");
        const char *xStr = firstChildWithTag(destElem, "xpos")->getNodeValue();
        const char *yStr = firstChildWithTag(destElem, "ypos")->getNodeValue();
        const char *startTimeStr = firstChildWithTag(curElem, "start_time")->getNodeValue();
        const char *endTimeStr = firstChildWithTag(curElem, "end_time")->getNodeValue();
        double elapsedTime = atof(endTimeStr) - atof(startTimeStr);
        if (elapsedTime == 0)
            throw cRuntimeError("Elapsed time is zero: infinite speed");
        Coord currentPos(atof(xStr), atof(yStr), 0);
        double distance = currentPos.distance(lastPos);
        double currentSpeed = distance / elapsedTime;
        if (currentSpeed > maxSpeed)
            maxSpeed = currentSpeed;
        lastPos = currentPos;
        curElem = findNextPositionChange(curElem->getNextSibling());
    }
}

ANSimMobility::ANSimMobility()
{
    maxSpeed = 0;
    nodeId = -1;
    nextPositionChange = nullptr;
}

void ANSimMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing ANSimMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        nodeId = par("nodeId");
        if (nodeId == -1)
            nodeId = getContainingNode(this)->getIndex();

        // get script: param should point to <simulation> element
        cXMLElement *rootElem = par("ansimTrace");
        if (strcmp(rootElem->getTagName(), "simulation") != 0)
            throw cRuntimeError("<simulation> is expected as root element not <%s> at %s",
                    rootElem->getTagName(), rootElem->getSourceLocation());
        nextPositionChange = rootElem->getElementByPath("mobility/position_change");
        if (!nextPositionChange)
            throw cRuntimeError("Element doesn't have <mobility> child or <position_change> grandchild at %s",
                    rootElem->getSourceLocation());
        computeMaxSpeed();
    }
}

void ANSimMobility::setInitialPosition()
{
    cXMLElement *firstPositionChange = findNextPositionChange(nextPositionChange);
    if (firstPositionChange)
        extractDataFrom(firstPositionChange);
    lastPosition = targetPosition;
}

cXMLElement *ANSimMobility::findNextPositionChange(cXMLElement *positionChange)
{
    // find next <position_change> element with matching <node_id> tag (current one also OK)
    while (positionChange) {
        const char *nodeIdStr = firstChildWithTag(positionChange, "node_id")->getNodeValue();
        if (nodeIdStr && atoi(nodeIdStr) == nodeId)
            break;

        positionChange = positionChange->getNextSibling();
    }
    return positionChange;
}

void ANSimMobility::setTargetPosition()
{
    nextPositionChange = findNextPositionChange(nextPositionChange);
    if (!nextPositionChange) {
        nextChange = -1;
        stationary = true;
        targetPosition = lastPosition;
        return;
    }

    // extract data from it
    extractDataFrom(nextPositionChange);

    // skip this one
    nextPositionChange = nextPositionChange->getNextSibling();
}

void ANSimMobility::extractDataFrom(cXMLElement *node)
{
    // extract data from <position_change> element
    // FIXME start_time has to be taken into account too! as pause from prev element's end_time
    const char *startTimeStr = firstChildWithTag(node, "start_time")->getNodeValue();
    if (!startTimeStr)
        throw cRuntimeError("No content in <start_time> element at %s", node->getSourceLocation());
    const char *endTimeStr = firstChildWithTag(node, "end_time")->getNodeValue();
    if (!endTimeStr)
        throw cRuntimeError("No content in <end_time> element at %s", node->getSourceLocation());
    cXMLElement *destElem = firstChildWithTag(node, "destination");
    const char *xStr = firstChildWithTag(destElem, "xpos")->getNodeValue();
    const char *yStr = firstChildWithTag(destElem, "ypos")->getNodeValue();
    if (!xStr || !yStr)
        throw cRuntimeError("No content in <destination>/<xpos> or <ypos> element at %s", node->getSourceLocation());

    nextChange = atof(endTimeStr);
    targetPosition.x = atof(xStr);
    targetPosition.y = atof(yStr);
}

void ANSimMobility::move()
{
    LineSegmentsMobilityBase::move();
    raiseErrorIfOutside();
}

} // namespace inet

