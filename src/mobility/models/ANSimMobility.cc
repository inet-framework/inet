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


#include "ANSimMobility.h"
#include "FWMath.h"


Define_Module(ANSimMobility);


static cXMLElement *firstChildWithTag(cXMLElement *node, const char *tagname)
{
    cXMLElement *child = node->getFirstChild();
    while (child && strcmp(child->getTagName(), tagname)!=0)
        child = child->getNextSibling();

    if (!child)
        throw cRuntimeError("Element <%s> has no <%s> child at %s",
                node->getTagName(), tagname, node->getSourceLocation());

    return child;
}

ANSimMobility::ANSimMobility()
{
    nodeId = -1;
    nextPositionChange = NULL;
}

void ANSimMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);
    EV << "initializing ANSimMobility stage " << stage << endl;
    if (stage == 0)
    {
        nodeId = par("nodeId");
        if (nodeId == -1)
            nodeId = getParentModule()->getIndex();

        // get script: param should point to <simulation> element
        cXMLElement *rootElem = par("ansimTrace");
        if (strcmp(rootElem->getTagName(), "simulation")!=0)
            throw cRuntimeError("<simulation> is expected as root element not <%s> at %s",
                  rootElem->getTagName(), rootElem->getSourceLocation());
        nextPositionChange = rootElem->getElementByPath("mobility/position_change");
        if (!nextPositionChange)
            throw cRuntimeError("Element doesn't have <mobility> child or <position_change> grandchild at %s",
                  rootElem->getSourceLocation());
    }
}

void ANSimMobility::initializePosition()
{
    cXMLElement *firstPositionChange = findNextPositionChange(nextPositionChange);
    if (firstPositionChange)
        extractDataFrom(firstPositionChange);
    lastPosition = targetPosition;
}

cXMLElement *ANSimMobility::findNextPositionChange(cXMLElement *positionChange)
{
    // find next <position_change> element with matching <node_id> tag (current one also OK)
    while(positionChange){
        const char *nodeIdStr = firstChildWithTag(positionChange, "node_id")->getNodeValue();
        if(nodeIdStr && atoi(nodeIdStr) == nodeId)
            break;

        positionChange = positionChange->getNextSibling();
    }
    return positionChange;
}

void ANSimMobility::setTargetPosition()
{
    nextPositionChange = findNextPositionChange(nextPositionChange);
    if (!nextPositionChange)
    {
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
    const char *endTimeStr = firstChildWithTag(node, "end_time")->getNodeValue();
    cXMLElement *destElem = firstChildWithTag(node, "destination");
    const char *xStr = firstChildWithTag(destElem, "xpos")->getNodeValue();
    const char *yStr = firstChildWithTag(destElem, "ypos")->getNodeValue();

    if (!endTimeStr || !xStr || !yStr)
        throw cRuntimeError("No content in <end_time>, <destination>/<xpos> or <ypos> element at %s", node->getSourceLocation());

    nextChange = atof(endTimeStr);
    targetPosition.x = atof(xStr);
    targetPosition.y = atof(yStr);
}

void ANSimMobility::move()
{
    LineSegmentsMobilityBase::move();
    raiseErrorIfOutside();
}
