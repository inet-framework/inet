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

#ifndef QUADTREENEIGHBORCACHE_H_
#define QUADTREENEIGHBORCACHE_H_

#include "RadioMedium.h"

namespace radio {

// It is a Quadtree implementation for efficient (in a static network)
// orthogonal range queries.

class QuadTreeNeighborCache : public RadioMedium::INeighborCache, public cSimpleModule
{
    public:
        typedef std::vector<const IRadio *> Radios;
        struct QuadTreeNode
        {
            Coord boundaryMin;
            Coord boundaryMax;
            Radios points;
            QuadTreeNode *quadrants[4];
            QuadTreeNode *parent;
        };
    protected:
        std::map<const IRadio *, Coord> lastPosition; // the radio position when we inserted to the QuadTree
        unsigned int maxNumOfPointsPerQuadrant;
        QuadTreeNode *rootNode;
        cMessage *rebuildQuadTreeTimer;
        RadioMedium *radioMedium;
        Coord constraintAreaMax, constraintAreaMin;
        Radios radios;
        double range;
        double rebuildPeriod;
        double maxSpeed;
        bool strictQueries;
        bool isStaticNetwork;
    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
        bool addPointToQuadTree(QuadTreeNode *rootNode, const IRadio *radio);
        bool removeQuadTreePoint(QuadTreeNode *rootNode, const IRadio *radio);
        QuadTreeNode *searchRadioQuadrant(QuadTreeNode *rootNode, Coord lastPos);
        unsigned int whichQuadrant(QuadTreeNode *node, Coord radioPos) const;
        bool hasChild(QuadTreeNode *node) const;
        void setBoundary(QuadTreeNode *node);
        void splitPoints(QuadTreeNode *node);
        void setToLeaf(QuadTreeNode *node);
        QuadTreeNode *createQuadTree();
        void fillQuadTreeWithRadios(QuadTreeNode *rootNode);
        void rangeQuery(QuadTreeNode *rootNode, const IRadio *transmitter, Coord minRectangleBoundary, Coord maxRectangleBoundary, Radios& neighbors);
        void strictRangeQuery(QuadTreeNode *rootNode, const IRadio *transmitter, Coord transmitterPos, double range, Coord minRectangleBoundary, Coord maxRectangleBoundary, Radios& neighbors);
        void deleteTree(QuadTreeNode *rootNode);
        bool isInRectangleRange(Coord pointCoord, Coord rectangleBoundaryMin, Coord rectangleBoundaryMax) const;
        bool doesIntersectWithQuadrant(QuadTreeNode *quadrant, Coord rectangleBoundaryMin, Coord rectangleBoundaryMax) const;
        void tryToJoinChildQuadrants(QuadTreeNode *node);
        void moveQuadTreePoint(const IRadio *radio);
    public:
        void addRadio(const IRadio *radio);
        void removeRadio(const IRadio *radio);
        void sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame);
        QuadTreeNeighborCache() : rootNode(NULL), rebuildQuadTreeTimer(NULL), radioMedium(NULL) {};
        ~QuadTreeNeighborCache();
};

} /* namespace radio */

#endif /* QUADTREENEIGHBORCACHE_H_ */
