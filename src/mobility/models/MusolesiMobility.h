//
// Mobilty based on the code released by Mirco Musolesi:
// http://portal.acm.org/citation.cfm?id=1132983.1132990
// "A community based mobility model for ad hoc network research"
// Ported to Omnet++ by Leonardo Maccari for the PAF-FPE project, see pervacy.eu
// leonardo.maccari@unitn.it
//
// Mobility Patterns Generator for ns-2 simulator
// Based on a Community Based Mobility Model
//
// Copyright (C) Mirco Musolesi University College London
// m.musolesi@cs.ucl.ac.uk
// Mirco Musolesi
// Department of Computer Science - University College London
// Gower Street London WC1E 6BT United Kingdom
// Email: m.musolesi@cs.ucl.ac.uk
// Phone: +44 20 7679 0391 Fax: +44 20 7387 1397
// Web: http://www.cs.ucl.ac.uk/staff/m.musolesi
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef MUSOLESI_MOBILITY_H
#define MUSOLESI_MOBILITY_H
#include "LineSegmentsMobilityBase.h"

struct hostsItem
{
        double currentX;
        double currentY;
        int sqaureIdX;
        int squareIdY;
        int homeSquareIdX;
        int homeSquareIdY;
        double speed;
};

struct squareItem
{
        Coord pos;
        int numberOfHosts;
};

class INET_API MusolesiMobility : public LineSegmentsMobilityBase
{
    protected:
        double minHostSpeed;
        double maxHostSpeed;
        int numberOfRows;
        int numberOfColumns;

        double rewiringProb;
        int numberOfCommunities;

        int targetChoice;

        // length of the squares
        double sideLengthX;
        double sideLengthY;

        // interaction threshold
        double threshold;
        // number of mobile hosts
        int numHosts;

        double HCMM;
        int rewiringPeriod;
        int reshufflePeriod;
        int initialRewiringPeriod;
        int initialReshufflePeriod;
        double recordStartTime;
        std::map<int, std::pair<double, double> > nodesInMyBlock;
        static std::map<int, int> intervalDistribution;
        static std::map<int, int> interContactDistribution;
        bool RWP;
        double drift;
        double expmean;
        bool reshufflePositionsOnly;
        int myCommunity;
        bool recordStatistics;

        static std::vector<hostsItem> hosts;
        static std::vector<std::vector<squareItem> > squares;
        static std::vector<int> numberOfMembers;
        static std::vector<std::vector<double> > interaction;
        static std::vector<std::vector<int> > communities;

        static simsignal_t blocksHistogram;
        static simsignal_t walkedMeters;
        static simsignal_t blockChanges;

        std::vector<std::vector<double> > squareAttractivity;
        int nodeId;

        cMessage * moveMessage;

    public:
        MusolesiMobility() {}
        ~MusolesiMobility();

    protected:
        /** @brief Initializes mobility model parameters.*/
        virtual void initialize(int);
        virtual void finish();
        virtual void initializePosition();
        /* @brief Configure the initial group matrix and positions */
        void setInitialPosition();
        /* @brief Refresh the host's positions and the group matrix only if the reshufflePositionsOnly is false */
        void setPosition(bool reshufflePositionsOnly);
        /* @brief Define and allocate static members */
        void defineStaticMembers();
        /* @brief Redefined from LineSegmentsMobility */
        virtual void setTargetPosition();
        /* @brief Redefined from LineSegmentsMobility */
        virtual void fixIfHostGetsOutside();
        virtual void handleSelfMsg(cMessage* msg);
        virtual void handleMessage(cMessage* msg);
        void move();
        /* @brief Get a random point inside a square */
        Coord getRandomPoint(Coord pos);

        void rewire();
        void refreshCommunities();
        bool areInTheSameCommunity(int node1, int node2);
        bool isInCommunity(int node, std::vector<int>& group, int numberOfMembers);
};



#endif

