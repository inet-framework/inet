//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "QuadTreeNeighborCache.h"
using namespace radio;
Define_Module(QuadTreeNeighborCache);

void QuadTreeNeighborCache::addRadio(const IRadio* radio)
{
    radios.push_back(radio);
    if (rootNode == NULL)
        rootNode = createQuadTree();
    addPointToQuadTree(rootNode, radio);
}

void QuadTreeNeighborCache::removeRadio(const IRadio* radio)
{
    Radios::iterator it = find(radios.begin(), radios.end(), radio);
    if (it != radios.end())
        radios.erase(it);
    else
    {
        // TODO: is it an error?
    }
}

void QuadTreeNeighborCache::sendToNeighbors(IRadio* transmitter, const IRadioFrame* frame)
{
    Radios neighbors;
    double radius;

    if (isStaticNetwork)
    {
        // TODO: in this case, the range can be a parameter
        radius = range;
    }
    else
        radius = range + rebuildPeriod * maxSpeed;

    Coord minRectangleBoundary, maxRectangleBoundary, transmitterPos;
    transmitterPos = minRectangleBoundary = maxRectangleBoundary = transmitter->getAntenna()->getMobility()->getCurrentPosition();

    // The boundary coordinates of a rectangle which contains a ball with our radius
    minRectangleBoundary.x -= radius;
    minRectangleBoundary.y -= radius;
    maxRectangleBoundary.x += radius;
    maxRectangleBoundary.y += radius;

    if (minRectangleBoundary.x < constraintAreaMin.x)
        minRectangleBoundary.x = constraintAreaMin.x;
    if (minRectangleBoundary.y < constraintAreaMin.y)
        minRectangleBoundary.y = constraintAreaMin.y;
    if (maxRectangleBoundary.x > constraintAreaMax.x)
        maxRectangleBoundary.x = constraintAreaMax.x;
    if (maxRectangleBoundary.y > constraintAreaMax.y)
        maxRectangleBoundary.y = constraintAreaMax.y;

    if (strictQueries)
        strictRangeQuery(rootNode, transmitter, transmitterPos, radius, minRectangleBoundary, maxRectangleBoundary, neighbors);
    else
        rangeQuery(rootNode, transmitter, minRectangleBoundary, maxRectangleBoundary, neighbors);

    for (unsigned int i = 0; i < neighbors.size(); i++)
        radioMedium->sendToRadio(transmitter, neighbors[i], frame);
}

void QuadTreeNeighborCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        radioMedium = check_and_cast<RadioMedium *>(getParentModule());
        rebuildQuadTreeTimer = new cMessage("rebuildQuadTreeTimer");
        constraintAreaMax.x = par("constraintAreaMaxX");
        constraintAreaMax.y = par("constraintAreaMaxY");
        constraintAreaMin.x = par("constraintAreaMinX");
        constraintAreaMin.y = par("constraintAreaMinY");

        range = par("range");
        rebuildPeriod = par("refillPeriod");
        maxSpeed = par("maxSpeed");
        maxNumOfPointsPerQuadrant = par("maxNumOfPointsPerQuadrant");
        strictQueries = par("strictQueries");
        isStaticNetwork = par("isStaticNetwork");
    }
    else if (stage == INITSTAGE_LINK_LAYER_2)
    {
        rootNode = createQuadTree();
        fillQuadTreeWithRadios(rootNode);

        if (!isStaticNetwork)
            scheduleAt(simTime() + rebuildPeriod, rebuildQuadTreeTimer);
    }
}

void QuadTreeNeighborCache::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("This module only handles self messages");

    EV_DETAIL << "Rebuilding the Quadtree" << endl;
    deleteTree(rootNode);
    rootNode = createQuadTree();
    fillQuadTreeWithRadios(rootNode);

    /*for (unsigned int i = 0; i < radios.size() - 2; i++)
        removeQuadTreePoint(rootNode, radios[i]);

    for (unsigned int j = 0; j < radios.size(); j++)
    {
        QuadTreeNode* node = searchRadioQuadrant(rootNode, radios[j]);
        if (node == NULL)
            ASSERT(false);
        std::cout << "Radio: " << j << " " << node->boundaryMin << " " << node->boundaryMax << endl;
        for (unsigned int i = 0; i < node->points.size(); i++)
        {
            std::cout << node->points[i]->getId() << endl;
        }
        std::cout << endl;
    }*/

    scheduleAt(simTime() + rebuildPeriod, msg);
}

bool QuadTreeNeighborCache::addPointToQuadTree(QuadTreeNode *rootNode, const IRadio *radio)
{
    ASSERT(rootNode != NULL);
    Coord radioPos = radio->getAntenna()->getMobility()->getCurrentPosition();

    if (!isInRectangleRange(radioPos, rootNode->boundaryMin, rootNode->boundaryMax))
        return false;

    if (rootNode->points.size() < maxNumOfPointsPerQuadrant && rootNode->quadrants[0] == NULL)
    {
        lastPosition[radio] = radioPos;
        rootNode->points.push_back(radio);
        return true;
    }
    // If the rootNode is a leaf with a point set
    // with no free capacity, then we must split its
    // points into new quadrants

    if (!hasChild(rootNode))
        splitPoints(rootNode);

    for (unsigned int i = 0; i < 4; i++)
        if (addPointToQuadTree(rootNode->quadrants[i], radio))
            return true;

    ASSERT(false);
    return false;
}

unsigned int QuadTreeNeighborCache::whichQuadrant(QuadTreeNode *node, Coord radioPos) const
{
    for (unsigned int i = 0; i < 4; i++)
        if (isInRectangleRange(radioPos, node->quadrants[i]->boundaryMin, node->quadrants[i]->boundaryMax))
            return i;
    ASSERT(false);
    return 100;
}

void radio::QuadTreeNeighborCache::setBoundary(QuadTreeNode *node)
{
    // We just divide a rectangle into four smaller congruent rectangle

    node->quadrants[0]->boundaryMax.x = (node->boundaryMax.x - node->boundaryMin.x) / 2 + node->boundaryMin.x;
    node->quadrants[0]->boundaryMin.y = (node->boundaryMax.y - node->boundaryMin.y) / 2 + node->boundaryMin.y;
    node->quadrants[0]->boundaryMin.x = node->boundaryMin.x;
    node->quadrants[0]->boundaryMax.y = node->boundaryMax.y;

    node->quadrants[1]->boundaryMin.x = (node->boundaryMax.x - node->boundaryMin.x) / 2 + node->boundaryMin.x;
    node->quadrants[1]->boundaryMin.y = (node->boundaryMax.y - node->boundaryMin.y) / 2 + node->boundaryMin.y;
    node->quadrants[1]->boundaryMax.x = node->boundaryMax.x;
    node->quadrants[1]->boundaryMax.y = node->boundaryMax.y;

    node->quadrants[2]->boundaryMax.y = (node->boundaryMax.y - node->boundaryMin.y) / 2 + node->boundaryMin.y;
    node->quadrants[2]->boundaryMax.x = (node->boundaryMax.x - node->boundaryMin.x) / 2 + node->boundaryMin.x;
    node->quadrants[2]->boundaryMin.x = node->boundaryMin.x;
    node->quadrants[2]->boundaryMin.y = node->boundaryMin.y;

    node->quadrants[3]->boundaryMin.x = (node->boundaryMax.x - node->boundaryMin.x) / 2 + node->boundaryMin.x;
    node->quadrants[3]->boundaryMax.y = (node->boundaryMax.y - node->boundaryMin.y) / 2 + node->boundaryMin.y;
    node->quadrants[3]->boundaryMax.x = node->boundaryMax.x;
    node->quadrants[3]->boundaryMin.y = node->boundaryMin.y;
}

void QuadTreeNeighborCache::splitPoints(QuadTreeNode *node)
{
    // We make four new quadrants
    for (unsigned int i = 0; i < 4; i++)
    {
        node->quadrants[i] = new QuadTreeNode;
        // The new quadrants are leaves
        setToLeaf(node->quadrants[i]);
        // Set parent
        node->quadrants[i]->parent = node;
    }
    setBoundary(node);

    Radios *points = &node->points;

    // The node is not a leaf anymore
    // so we have to split its point

    for (unsigned int i = 0; i < points->size(); i++)
    {
        Coord radioPos = (*points)[i]->getAntenna()->getMobility()->getCurrentPosition();
        unsigned int quadrantNum = whichQuadrant(node, radioPos);

        // We recursively call addPointToQuadTree() for each points
        addPointToQuadTree(node->quadrants[quadrantNum], (*points)[i]);
    }

    // Now we can free the node's point vector
    points->clear();
}

void QuadTreeNeighborCache::setToLeaf(QuadTreeNode* node)
{
    for (unsigned int i = 0; i < 4; i++)
        node->quadrants[i] = NULL;
}

QuadTreeNeighborCache::QuadTreeNode* QuadTreeNeighborCache::createQuadTree()
{
    QuadTreeNode *rootNode = new QuadTreeNode;
    setToLeaf(rootNode);
    rootNode->boundaryMax = constraintAreaMax;
    rootNode->boundaryMin = constraintAreaMin;
    // root has no parent
    rootNode->parent = NULL;
    return rootNode;
}

void QuadTreeNeighborCache::fillQuadTreeWithRadios(QuadTreeNode *rootNode)
{
    for (unsigned int i = 0; i < radios.size(); i++)
        if (!addPointToQuadTree(rootNode, radios[i]))
            throw cRuntimeError("Unsuccessful QuadTree building");
}

void QuadTreeNeighborCache::rangeQuery(QuadTreeNode *rootNode, const IRadio *transmitter, Coord minRectangleBoundary, Coord maxRectangleBoundary, Radios& neighbors)
{
    // If our rectangle intersects with a quadrant then we insert its points to the
    // neighbors vector
    // Note that, a node have points only if it is a leaf node
    if (!hasChild(rootNode) &&
            doesIntersectWithQuadrant(rootNode, minRectangleBoundary, maxRectangleBoundary))
    {
        for (unsigned int i = 0; i < rootNode->points.size(); i++)
            if (transmitter->getId() != rootNode->points[i]->getId())
                neighbors.push_back(rootNode->points[i]);
    }
    else if (hasChild(rootNode))
    {
        for (unsigned int i = 0; i < 4; i++)
            rangeQuery(rootNode->quadrants[i], transmitter, minRectangleBoundary, maxRectangleBoundary, neighbors);
    }
}

void QuadTreeNeighborCache::strictRangeQuery(QuadTreeNode *rootNode, const IRadio *transmitter, Coord transmitterPos, double range, Coord minRectangleBoundary, Coord maxRectangleBoundary, Radios& neighbors)
{
    if (!hasChild(rootNode)
            && doesIntersectWithQuadrant(rootNode, minRectangleBoundary, maxRectangleBoundary))
    {
        for (unsigned int i = 0; i < rootNode->points.size(); i++)
        {
            Coord otherRadioPos = rootNode->points[i]->getAntenna()->getMobility()->getCurrentPosition();
            if (transmitter->getId() != rootNode->points[i]->getId() &&
                    transmitterPos.sqrdist(otherRadioPos) <= range * range)
                neighbors.push_back(rootNode->points[i]);
        }
    }
    else if (hasChild(rootNode))
    {
        for (unsigned int i = 0; i < 4; i++)
            rangeQuery(rootNode->quadrants[i], transmitter, minRectangleBoundary, maxRectangleBoundary, neighbors);
    }
}

void radio::QuadTreeNeighborCache::deleteTree(QuadTreeNode *rootNode)
{
    if (rootNode == NULL)
        return;
    for (unsigned int i = 0; i < 4; i++)
        deleteTree(rootNode->quadrants[i]);
    delete rootNode;
}

bool QuadTreeNeighborCache::isInRectangleRange(Coord pointCoord, Coord rectangleBoundaryMin, Coord rectangleBoundaryMax) const
{
    if (pointCoord.x <= rectangleBoundaryMax.x && pointCoord.x >= rectangleBoundaryMin.x &&
        pointCoord.y <= rectangleBoundaryMax.y && pointCoord.y >= rectangleBoundaryMin.y)
        return true;
    return false;
}

bool QuadTreeNeighborCache::doesIntersectWithQuadrant(QuadTreeNode *quadrant, Coord rectangleBoundaryMin, Coord rectangleBoundaryMax) const
{
    return !( (rectangleBoundaryMin.x > quadrant->boundaryMax.x) || (rectangleBoundaryMax.x < quadrant->boundaryMin.x) ||
            (rectangleBoundaryMin.y > quadrant->boundaryMax.y) || (rectangleBoundaryMax.y < quadrant->boundaryMin.y) );
}

bool QuadTreeNeighborCache::removeQuadTreePoint(QuadTreeNode* rootNode, const IRadio* radio)
{
    // We search for the quadrant that contains our radio
    // Note that, we need the last position of radio, that is, the position when we
    // inserted it into the QuadTree
    // This helps to searchRadioQuadrant(), since we don't have to traverse
    // the whole QuadTree and check each node's point vector one by one.
    QuadTreeNode *quadrant = searchRadioQuadrant(rootNode, lastPosition[radio]);

    if (quadrant == NULL)
        throw cRuntimeError("Quadrant not found for radio ID: %d", radio->getId());

    Radios::iterator it = find(quadrant->points.begin(), quadrant->points.end(), radio);

    if (it != quadrant->points.end())
        quadrant->points.erase(it);
    else
        return false;

    tryToJoinChildQuadrants(quadrant->parent);

    return true;
}

QuadTreeNeighborCache::QuadTreeNode* QuadTreeNeighborCache::searchRadioQuadrant(QuadTreeNode *rootNode, Coord lastPos)
{
    // If radiosPos is in the quadrants's rectangle range and that quadrant has no child,
    // then we found the quadrant which _may_ contain our radio.
    // Note that, this can not guarantee that the radio is in the quadrant's
    // point vector, so you should check it yourself!

    if (!hasChild(rootNode) &&
            isInRectangleRange(lastPos, rootNode->boundaryMin, rootNode->boundaryMax))
        return rootNode;

    else if (hasChild(rootNode))
    {
        for (int i = 0; i < 4; i++)
            if (isInRectangleRange(lastPos, rootNode->quadrants[i]->boundaryMin, rootNode->quadrants[i]->boundaryMax))
                return searchRadioQuadrant(rootNode->quadrants[i], lastPos);
        return NULL;
    }
    else
        return NULL;
}

bool QuadTreeNeighborCache::hasChild(QuadTreeNode* node) const
{
    return node->quadrants[0] != NULL;
}

void radio::QuadTreeNeighborCache::tryToJoinChildQuadrants(QuadTreeNode* node)
{
    if (node == NULL)
        return;

    unsigned int quadrantSum = 0;

    for (unsigned int i = 0; i < 4; i++)
    {
        // We surely can't join quadrants if one quadrant has another
        // subquadrants

        if (hasChild(node->quadrants[i]))
            return;

        quadrantSum += node->quadrants[i]->points.size();
    }

    // If the child quadrants together contain no more
    // than maxNumOfPointsPerQuadrant points then we can
    // join these quadrants

    if (quadrantSum <= maxNumOfPointsPerQuadrant)
    {
        // Copy the points to the parent node
        for (unsigned int i = 0; i < 4; i++)
        {
            QuadTreeNode *quadrant = node->quadrants[i];
            for (unsigned int j = 0; j < quadrant->points.size(); j++)
                node->points.push_back(quadrant->points[j]);

            // Delete the child quadrants
            delete quadrant;
        }

        // Then set to leaf
        setToLeaf(node);

        // Finally, we call it for the parent node
        tryToJoinChildQuadrants(node->parent);
    }
}

void QuadTreeNeighborCache::moveQuadTreePoint(const IRadio* radio)
{
    Coord radioPos = radio->getAntenna()->getMobility()->getCurrentPosition();
    QuadTreeNode *quadrant = searchRadioQuadrant(rootNode, radioPos);

    if (quadrant == NULL)
        throw cRuntimeError("Quadrant not found for radio ID: %d", radio->getId());

    Radios::iterator it = find(quadrant->points.begin(), quadrant->points.end(), radio);

    // If we search for a quadrant with the radio's current position and the we find
    // it in the quadrant's point vector, then the move occurred inside this quadrant,
    // thus we have nothing to do with this case

    // Otherwise, we remove the point and insert it again
    if (it == quadrant->points.end())
    {
        removeQuadTreePoint(rootNode, radio);
        addPointToQuadTree(rootNode, radio);
    }
}

QuadTreeNeighborCache::~QuadTreeNeighborCache()
{
    deleteTree(rootNode);
    cancelAndDelete(rebuildQuadTreeTimer);
}
