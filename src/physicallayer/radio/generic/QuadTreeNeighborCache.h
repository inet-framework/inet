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
#include "QuadTree.h"

namespace physicallayer {

class QuadTreeNeighborCache : public cSimpleModule, public RadioMedium::INeighborCache
{
    public:
        typedef std::vector<const IRadio *> Radios;
        typedef QuadTree::QuadTreeVisitor QuadTreeVisitor;

    protected:
        class QuadTreeNeighborCacheVisitor : public QuadTree::QuadTreeVisitor
        {
            protected:
                RadioMedium *radioMedium;
                IRadio *transmitter;
                const IRadioFrame *frame;
            public:
                void visitor(const cObject *radio);
                QuadTreeNeighborCacheVisitor(RadioMedium *radioMedium, IRadio *transmitter, const IRadioFrame *frame) :
                    radioMedium(radioMedium), transmitter(transmitter), frame(frame) {}
        };

    protected:
        QuadTree *quadTree;
        Radios radios;
        cMessage *rebuildQuadTreeTimer;
        RadioMedium *radioMedium;
        Coord constraintAreaMax, constraintAreaMin;
        unsigned int maxNumOfPointsPerQuadrant;
        double range;
        double rebuildPeriod;
        double maxSpeed;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
        void fillQuadTreeWithRadios();

    public:
        void addRadio(const IRadio *radio);
        void removeRadio(const IRadio *radio);
        void sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame);
        QuadTreeNeighborCache();
        ~QuadTreeNeighborCache();
};

} /* namespace physicallayer */

#endif /* QUADTREENEIGHBORCACHE_H_ */
