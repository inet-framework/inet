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

#ifndef __INET_QUADTREENEIGHBORCACHE_H
#define __INET_QUADTREENEIGHBORCACHE_H

#include "inet/physicallayer/common/RadioMedium.h"
#include "inet/common/geometry/container/QuadTree.h"

namespace inet {

namespace physicallayer {

class QuadTreeNeighborCache : public cSimpleModule, public INeighborCache
{
  public:
    typedef std::vector<const IRadio *> Radios;

  protected:
    class QuadTreeNeighborCacheVisitor : public IVisitor
    {
      protected:
        RadioMedium *radioMedium;
        IRadio *transmitter;
        const IRadioFrame *frame;

      public:
        void visit(const cObject *radio) const;
        QuadTreeNeighborCacheVisitor(RadioMedium *radioMedium, IRadio *transmitter, const IRadioFrame *frame) :
            radioMedium(radioMedium), transmitter(transmitter), frame(frame) {}
    };

  protected:
    QuadTree *quadTree;
    Radios radios;
    RadioMedium *radioMedium;
    cMessage *rebuildQuadTreeTimer;
    Coord constraintAreaMax, constraintAreaMin;
    unsigned int maxNumOfPointsPerQuadrant;
    double refillPeriod;
    double maxSpeed;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    void fillQuadTreeWithRadios();
    void rebuildQuadTree();

  public:
    QuadTreeNeighborCache();
    ~QuadTreeNeighborCache();

    virtual void printToStream(std::ostream& stream) const;
    virtual void addRadio(const IRadio *radio);
    virtual void removeRadio(const IRadio *radio);
    virtual void sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame, double range) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_QUADTREENEIGHBORCACHE_H

