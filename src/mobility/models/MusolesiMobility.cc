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

#include <sstream>
#include "MusolesiMobility.h"

#define DETERMINISTIC 0
#define PSEUDODETERMINISTIC 1
#define PROPORTIONAL 2

Define_Module(MusolesiMobility);

// This members must be the same for each node.

std::vector<hostsItem> MusolesiMobility::hosts;
std::vector<std::vector<squareItem> > MusolesiMobility::squares;
std::vector<int> MusolesiMobility::numberOfMembers;
std::vector<std::vector<double> > MusolesiMobility::interaction;
std::vector<std::vector<int> > MusolesiMobility::communities;
std::map<int, int> MusolesiMobility::intervalDistribution;
std::map<int, int> MusolesiMobility::interContactDistribution;

simsignal_t MusolesiMobility::blocksHistogram = SIMSIGNAL_NULL;
simsignal_t MusolesiMobility::walkedMeters = SIMSIGNAL_NULL;
simsignal_t MusolesiMobility::blockChanges = SIMSIGNAL_NULL;

void MusolesiMobility::initialize(int stage)
{
    EV<< "initializing MusolesiMobility stage " << stage << endl;
    stationary = false; // this stuff is needed by LineSegmentsMobilityBase

    LineSegmentsMobilityBase::initialize(stage);
    if (stage == 0)
    {
        minHostSpeed= par("minHostSpeed");
        updateInterval = par("updateInterval");
        maxHostSpeed = par("maxHostSpeed");
        threshold = par("connectionThreshold");
        numberOfRows = par("numberOfRows");
        numberOfColumns = par("numberOfColumns");
        rewiringProb = par("rewiringProb");
        numberOfCommunities = par("numberOfCommunities");
        numHosts = par("numHosts");
        HCMM = par("HCMM");
        recordStartTime = par("recordStartTime");
        recordStatistics = par("recordStatistics");
        drift = par("drift");
        expmean = par("expmean");
        myCommunity = -1;

        constraintAreaMin.y = par("constraintAreaMinY");
        constraintAreaMax.x = par("constraintAreaMaxX");
        constraintAreaMin.x = par("constraintAreaMinX");
        constraintAreaMax.y = par("constraintAreaMaxY");

        constraintAreaMax += constraintAreaMin;

        nodeId = getParentModule()->getIndex();
        RWP = par("RWP");

        initialRewiringPeriod = rewiringPeriod = par("rewiringPeriod");
        initialReshufflePeriod = reshufflePeriod = par("reshufflePeriod");
        reshufflePositionsOnly = par("reshufflePositionsOnly");

        if (constraintAreaMin.y != 0 || constraintAreaMin.x != 0)
            error("Musolesi implementation does not currently support top \
                        coordinates != from (0,0), please set constraintAreaY =\
                        constraintAreaX = 0");

        if (numberOfCommunities > numHosts)
            error("You have set a number of hosts lower than the number of groups: %d < %d",numHosts,numberOfCommunities);

        if (par("targetChoice").stdstringValue().compare("proportional") == 0)
            targetChoice = PROPORTIONAL;
        else if (par("targetChoice").stdstringValue().compare("pseudodeterministic") == 0)
            targetChoice = PSEUDODETERMINISTIC;
        else if (par("targetChoice").stdstringValue().compare("deterministic") == 0)
            targetChoice = DETERMINISTIC;
        else
            error("targetChoice parameter must be one in deterministic, pseudodeterministic, proportional");
        if (targetChoice == DETERMINISTIC)
            EV << "targetChoice == DETERMINISTIC makes the node converge to fixed positions in the original algorithm. \
                After a few time there is almost no mobility. It must be pared with Boldrini variation and rewiring (see parameter hcmm)";
        if (HCMM > 0)
        {
            EV << "Using Boldrini/Conti/Passarella fix to original mobility" << std::endl;
            if (HCMM > 1)
                error("HCMM parameter is a probability, must be below 1");
        }

        // Note origin is top-left
        // Calculates the square's side length
        sideLengthX = constraintAreaMax.x / (1.0*numberOfColumns);
        sideLengthY = constraintAreaMax.y / (1.0*numberOfRows);

        defineStaticMembers(); // just allocate static memory
        if (recordStatistics)
            for (int i = 0; i< numHosts; i++)
                nodesInMyBlock[i].first = nodesInMyBlock[i].second = 0;

        moveMessage = new cMessage("MusolesiHeartbeat");
        scheduleAt(simTime() + 1, moveMessage);// do statistics, reshuffle, rewire...
        blocksHistogram = registerSignal("blocksHistogram");
        walkedMeters = registerSignal("walkedMeters");
        blockChanges = registerSignal("blockChanges");
    }
}

void MusolesiMobility::initializePosition()
{
    // maps are static so just one node is required to calculate them
    if (nodeId == 0)
        setInitialPosition();

    lastPosition.x = hosts[nodeId].currentX;
    lastPosition.y = hosts[nodeId].currentY;
}

void MusolesiMobility::defineStaticMembers()
{
    // memory allocation
    squareAttractivity.resize(numberOfRows);

    for (int i = 0; i < numberOfRows; i++)
        squareAttractivity[i].resize(numberOfColumns);

    if (nodeId == 0)
    {
        // memory allocation for static data members
        hosts.resize(numHosts);
        numberOfMembers.resize(numHosts);
        squares.resize(numberOfRows);
        for (int i = 0; i < numberOfRows; i++)
            squares[i].resize(numberOfColumns);

        interaction.resize(numHosts);
        communities.resize(numHosts);
        for (int i = 0; i < numHosts; i++)
        {
            interaction[i].resize(numHosts);
            communities[i].resize(numHosts);
        }
        // setup the speeds and areas
        for (int i = 0; i < numHosts; i++)
            hosts[i].speed = uniform(minHostSpeed, maxHostSpeed);
        for (int i = 0; i < numberOfRows; i++)
        {
            for (int j = 0; j < numberOfColumns; j++)
            {
                squares[i][j].pos.x = 1.0 * j * sideLengthX;
                squares[i][j].pos.y = 1.0 * i * sideLengthY;
                squares[i][j].numberOfHosts = 0;
            }
        }
    }
}

void MusolesiMobility::setTargetPosition()
{
    int selectedGoalSquareX = 0;
    int selectedGoalSquareY = 0;
    int previousGoalSquareX = hosts[nodeId].sqaureIdX;
    int previousGoalSquareY = hosts[nodeId].squareIdY;

    if (RWP)
    {
        selectedGoalSquareX = uniform(0, numberOfRows);
        selectedGoalSquareY = uniform(0, numberOfColumns);
    }
    else if ((HCMM > uniform(0, 1)) && (previousGoalSquareY != hosts[nodeId].homeSquareIdX)
            && (previousGoalSquareX != hosts[nodeId].homeSquareIdY))
    {
        // Boldrini variation, set a probability for each node to go back home
        // when a target has been reached. In Boldrini it could be anytime when
        // the node is out of home sampled at the rate
        selectedGoalSquareX = hosts[nodeId].homeSquareIdX;
        selectedGoalSquareY = hosts[nodeId].homeSquareIdY;
    }
    else
    {
        // update attraction matrix
        for (int c = 0; c < numberOfRows; c++)
        {
            for (int r = 0; r < numberOfColumns; r++)
                squareAttractivity[c][r] = 0.0;
        }
        for (int n = 0; n < numHosts; n++)
            if (n != nodeId)
                squareAttractivity[hosts[n].sqaureIdX][hosts[n].squareIdY] += interaction[nodeId][n];

        // The deterministic targetChoice only works with high HCMM
        if (targetChoice == DETERMINISTIC)
        {
            double CAMax = 0;
            for (int c = 0; c < numberOfRows; c++)
            {
                for (int r = 0; r < numberOfColumns; r++)
                {
                    if (squares[c][r].numberOfHosts != 0)
                    {
                        squareAttractivity[c][r] = squareAttractivity[c][r] / (1.0 * squares[c][r].numberOfHosts);
                    }
                    else
                        squareAttractivity[c][r] = 0;
                }
            }
            // Select the most attractive square
            for (int c = 0; c < numberOfRows; c++)
            {
                for (int r = 0; r < numberOfColumns; r++)
                {
                    if (squareAttractivity[c][r] > CAMax)
                    {
                        selectedGoalSquareX = c;
                        selectedGoalSquareY = r;

                        CAMax = squareAttractivity[c][r];

                    }
                }
            }
        }

        // end deterministic;
        // Pseudodeterministic choice. Order the squares with attraction != 0
        // for their attraction, then extract a random value with exponential
        // distribution (expmean defined before, recall average = 1/expmean)
        // that has a strong bias on the most attractive ones, but adds some
        // turbulence to the choice, (not that much as the uniform)
        else if (targetChoice == PSEUDODETERMINISTIC)
        {
            std::map<double, std::pair<int, int> > orderedSquareSet;
            std::map<double, std::pair<int, int> >::iterator oSeti;

            for (int c = 0; c < numberOfRows; c++)
                for (int r = 0; r < numberOfColumns; r++)
                    if (squareAttractivity[c][r] != 0)
                        orderedSquareSet[squareAttractivity[c][r]] = std::pair<int, int>(c, r);

            unsigned int rnd = exponential(1.0 * expmean);
            if (rnd >= orderedSquareSet.size())
            {
                rnd = orderedSquareSet.size() - 1;
            }
            oSeti = orderedSquareSet.begin();
            std::advance(oSeti,rnd);
            selectedGoalSquareX = (oSeti->second.first);
            selectedGoalSquareY = (oSeti->second.second);
        }
        else if (targetChoice == PROPORTIONAL)
        {
            // Probabilistic target selection. Each node selects a target in a
            // square. The choice is random with probability distribution
            // proportional to the attractivity of the squares.

            // Algorithm of the selection of the new square
            // Denominator for the normalization of the values
            double denNorm = 0.00;
            // this is the added probability to choose an empty square
            // calculate normalized attractivity denominator, plus drift for squares
            // that have 0 attractivity.
            for (int c = 0; c < numberOfRows; c++)
            {
                for (int r = 0; r < numberOfColumns; r++)
                {
                    denNorm = denNorm + squareAttractivity[c][r] + drift;
                }
            }
            // calculate normalized attractivity for each square,
            // assign a value of a distribution from 0 to 1 proportional to the
            // normalized attractivity to each square, get a random square with
            // probability proportional to the normalized attractivity. This
            // has been rewritten compared to original code. It was made of
            // too much for(), now it is quicker, just one cycle.
            // You can imagine squares as an array, extract a random value
            // (total squareAttractivity is normalized by deNorm to 1), get the element of
            // the array for which cumulative probability is higher than random
            //
            // Note, each square with some node has some attractivity, in the
            // previous case the best square is chosen, in this case, even a square
            // with little attractivity may be chosen
            float infiniteDice = (float) uniform(0.0, 1.0);
            double totalInterest = 0;
            bool goOut = 0;
            for (int c = 0; c < numberOfRows; c++)
            {
                for (int r = 0; r < numberOfColumns; r++)
                {
                    totalInterest += (squareAttractivity[c][r] + drift) / denNorm;
                    if (infiniteDice < totalInterest)
                    {
                        selectedGoalSquareX = c;
                        selectedGoalSquareY = r;
                        goOut = 1;
                        break;
                    }

                }
                if (goOut)
                    break;
            }
        }
    }

    // Re-definition of the number of hosts in each square
    squares[previousGoalSquareX][previousGoalSquareY].numberOfHosts -= 1;
    squares[selectedGoalSquareX][selectedGoalSquareY].numberOfHosts += 1;

    if (recordStatistics && (previousGoalSquareX != selectedGoalSquareX || previousGoalSquareY != selectedGoalSquareY))
        emit(blockChanges, 1);

    // refresh of the information
    hosts[nodeId].sqaureIdX = selectedGoalSquareX;
    hosts[nodeId].squareIdY = selectedGoalSquareY;

    Coord randomPoint = getRandomPoint(squares[selectedGoalSquareX][selectedGoalSquareY].pos);

    targetPosition.x = randomPoint.x;
    targetPosition.y = randomPoint.y;

    hosts[nodeId].speed = uniform(minHostSpeed, maxHostSpeed);
    double distance = lastPosition.distance(targetPosition);
    emit(walkedMeters, distance);
    simtime_t travelTime = distance / hosts[nodeId].speed;
    if (nextChange > 0)
        nextChange += travelTime;
    else
        nextChange = travelTime;
}

void MusolesiMobility::fixIfHostGetsOutside()
{
    if (lastPosition.x < 0 || lastPosition.y < 0 || lastPosition.x > constraintAreaMax.x
            || lastPosition.y > constraintAreaMax.y)
        EV << "The last step of the mobility was pointing out of the playground, adjusted to margin" << std::endl;
        if (lastPosition.x < 0 )
            lastPosition.x = 0;
        if (lastPosition.y < 0 )
            lastPosition.y = 0;
        if (lastPosition.x > constraintAreaMax.x)
            lastPosition.x = constraintAreaMax.x;
        if (lastPosition.y > constraintAreaMax.y)
            lastPosition.y = constraintAreaMax.y;
    }

void MusolesiMobility::handleMessage(cMessage * msg)
{
    if (msg == moveMessage)
    {
        handleSelfMsg(msg);
        char buf[40];
        sprintf(buf, "community:%d, tgt:%d,%d", myCommunity, hosts[nodeId].sqaureIdX, hosts[nodeId].squareIdY);
        getParentModule()->getDisplayString().setTagArg("t", 0, buf);

        for (int i = 0; i < numberOfCommunities; i++)
        {
            for (int j = 0; j < numberOfMembers[i]; j++)
                if (communities[i][j] - 1 == nodeId)
                    myCommunity = i + 1;
        }
        int communitySpan = 256 / numberOfCommunities;

        sprintf(buf, "#%.2x%.2x%.2x", communitySpan * myCommunity % 256,
                (communitySpan * (numberOfCommunities / 3) + communitySpan * myCommunity) % 256,
                (communitySpan * (numberOfCommunities * 2 / 3) + communitySpan * myCommunity) % 256);
        getParentModule()->getDisplayString().setTagArg("t", 2, buf);
        getParentModule()->getDisplayString().setTagArg("i", 1, buf);
    }
    else
        MobilityBase::handleMessage(msg);
}

void MusolesiMobility::handleSelfMsg(cMessage * msg)
{
    if (simTime() > rewiringPeriod && initialRewiringPeriod > 0 && nodeId == 0)
    {
        setPosition(false);
        rewire();
        rewiringPeriod += initialRewiringPeriod;
    }
    if (simTime() > reshufflePeriod && initialReshufflePeriod > 0 && nodeId == 0)
    {
        if (reshufflePositionsOnly)
            setPosition(true);
        else
            rewire();

        reshufflePeriod += initialReshufflePeriod;
    }
    int myCurrSquare = -1;
    if (recordStatistics)
    {
        if (simTime() > recordStartTime)
        {
            for (int i = 0; i < numHosts; i++)
            {
                if (i == nodeId)
                    continue;
                if (hosts[nodeId].sqaureIdX == hosts[i].sqaureIdX && hosts[nodeId].squareIdY == hosts[i].squareIdY)
                {
                    if (nodesInMyBlock[i].first == 0)
                    {
                        // was him in my block?, no
                        nodesInMyBlock[i].first = simTime().dbl(); // add in the map
                        interContactDistribution[(int) (10 * (simTime().dbl() - nodesInMyBlock[i].second))]++;
                        nodesInMyBlock[i].second = simTime().dbl(); // add in the map
                    }
                }
                // somebody is not in my block, but used to be
                else if (nodesInMyBlock[i].first != 0)
                {
                    // the map maps decades of seconds to adjacency intervals
                    intervalDistribution[(int) (10 * (simTime().dbl() - nodesInMyBlock[i].first))]++;
                    nodesInMyBlock[i].second = simTime().dbl();
                    nodesInMyBlock[i].first = 0;
                }
            }
        }
        myCurrSquare = ((int) (lastPosition.x / sideLengthX)) % numberOfColumns
                + (((int) (lastPosition.y / sideLengthY)) % numberOfRows) * numberOfColumns;
        if (recordStatistics && ((int) simTime().dbl()) % 10 == 0) // log every 10 sec
            emit(blocksHistogram, myCurrSquare);
    }
    scheduleAt(simTime() + 1, msg);
}

// @Brief Set the initial position of the nodes, or shuffle the communities (if shuffle==1)
void MusolesiMobility::setInitialPosition()
{
    // NOTE: the playground for omnet has zero coordinate on top left.
    // consequently, the squares are always addressed as a matrix indexed
    // using top-left element as 0,0
    double gridSizex = (constraintAreaMax.x - constraintAreaMin.x) / numberOfColumns;

    cDisplayString& parentDispStr = getParentModule()->getParentModule()->getDisplayString();

    std::stringstream buf;
    buf << "bgg=" << int(gridSizex);
    parentDispStr.parse(buf.str().c_str());

    refreshCommunities();

    for (int i = 0; i < numberOfCommunities; i++)
    {
        EV << "The members of communities "<< i + 1 << " are: ";
        for (int j = 0; j < numberOfMembers[i]; j++)
            EV << communities[i][j]-1 << " ";
        EV << std::endl;
    }
    for (int i = 0; i < numberOfCommunities; i++)
    {
        int squareIdX = uniform(0, numberOfRows);
        int squareIdY = uniform(0, numberOfColumns);
        for (int j = 0; j < numberOfMembers[i]; j++)
        {
            int hostId = communities[i][j];
            hosts[hostId - 1].homeSquareIdX = squareIdX;
            hosts[hostId - 1].homeSquareIdY = squareIdY;
            // increment the number of the hosts in that square
            squares[squareIdX][squareIdY].numberOfHosts += 1;
            hosts[hostId - 1].sqaureIdX = squareIdX;
            hosts[hostId - 1].squareIdY = squareIdY;
            EV<< "Setting home of " << hostId << " in position " << hosts[hostId-1].homeSquareIdX << " " << hosts[hostId-1].homeSquareIdY << std::endl;
            EV<< "Setting pos  of " << hostId << " in position " << hosts[hostId-1].sqaureIdX << " " << hosts[hostId-1].squareIdY << std::endl;
        }
    }
    // definition of the initial position of the hosts
    for (int k = 0; k < numHosts; k++)
    {
        Coord randomPoint = getRandomPoint(squares[hosts[k].sqaureIdX][hosts[k].squareIdY].pos);
        hosts[k].currentX = randomPoint.x;
        hosts[k].currentY = randomPoint.y;
    }
}

void MusolesiMobility::setPosition(bool reshufflePositionOnly)
{
    // communities based on the initial number of caves in the Caveman model
    // i.e., w=0
    if(!reshufflePositionOnly)
    {
        refreshCommunities();
        for (int i = 0; i < numberOfCommunities; i++)
        {
            EV << "The members of communtities " << i+1 << " are: ";
            for (int j = 0;j < numberOfMembers[i]; j++)
                EV << communities[i][j]-1 << " ";
            EV << std::endl;
        }
    }
    for (int i = 0; i < numberOfCommunities; i++)
    {
        int squareIdX = uniform(0, numberOfRows);
        int squareIdY = uniform(0, numberOfColumns);
        for (int j = 0; j < numberOfMembers[i]; j++)
        {
            int hostId = communities[i][j];
            hosts[hostId - 1].homeSquareIdX = squareIdX;
            hosts[hostId - 1].homeSquareIdY = squareIdY;
        }
    }
}

void MusolesiMobility::move()
{
    LineSegmentsMobilityBase::move();
    double angle = 0;

    // see http://dev.omnetpp.org/bugs/view.php?id=527
    // raiseErrorIfOutside();
    reflectIfOutside(lastPosition, lastSpeed, angle);
}

Coord MusolesiMobility::getRandomPoint(Coord pos)
{
    Coord p;
    p.x = uniform(pos.x, pos.x + sideLengthX);
    p.y = uniform(pos.y, pos.y + sideLengthY);
    p.z = uniform(0, 1);

    return p;
}

void MusolesiMobility::finish()
{
    if (recordStatistics)
    {
        for (int i = 0; i < numHosts; i++)
            if (i != nodeId && nodesInMyBlock[i].first != 0)
                intervalDistribution[(int) (10 * (simTime().dbl() - nodesInMyBlock[i].first))]++;
    }

    if (nodeId == 0)
    {
        int totalFreq = 0;
        double partialFreq = 0;
        if (recordStatistics)
        {
            EV << "#CONTACTTIME" << std::endl;
            // print statistics
            for (std::map<int, int>::iterator ii = intervalDistribution.begin(); ii != intervalDistribution.end(); ii++)
            {
                totalFreq += ii->second;
            }
            for (std::map<int, int>::iterator ii = intervalDistribution.begin(); ii != intervalDistribution.end(); ii++)
            {
                partialFreq += ii->second;
                EV << ii->first << " " << ii->second << " " << 1 - partialFreq / totalFreq << std::endl;
            }
            totalFreq = 0;
            partialFreq = 0;
            EV << "#INTERCTIME" << std::endl;

            for (std::map<int, int>::iterator ii = interContactDistribution.begin(); ii != interContactDistribution.end(); ii++)
            {
                totalFreq += ii->second;
            }
            for (std::map<int, int>::iterator ii = interContactDistribution.begin(); ii != interContactDistribution.end(); ii++)
            {
                partialFreq += ii->second;
                EV << ii->first << " " << ii->second << " " << 1 - partialFreq / totalFreq << std::endl;
            }
            EV << "#ENDSTATISTICS" << std::endl;

        }
    }
    cancelAndDelete(moveMessage);
}

MusolesiMobility::~MusolesiMobility()
{
    for(int i = 0; i < numberOfRows; i++)
    {
        squareAttractivity[i].clear();
        squares[i].clear();
    }
    squareAttractivity.clear();
    squares.clear();
    for (int i = 0; i < numHosts; i++)
    {
        interaction[i].clear();
        communities[i].clear();
    }
    interaction.clear();
    communities.clear();
    numberOfMembers.clear();
    hosts.clear();
    intervalDistribution.clear();
    interContactDistribution.clear();
}

// in this rewiring I do not unlink the nodes from their current community, I just
// add links to other nodes in other communities. This way the network keeps being
// clustered but with more inter-cluster connections, else, the network becomes
// unclustered.

void MusolesiMobility::rewire()
{
    double scalefactor = 0;
    if (threshold < 0.5)  // little number that depends on the threshold
        scalefactor = threshold;
    else
        scalefactor = (1 - threshold);

    for (int i = 0; i < numHosts; i++)
        for (int j = 0; j < numHosts; j++)
        {
            if (areInTheSameCommunity(i + 1, j + 1) == true)
            {
                // chose a couple of nodes in the community
                if (uniform(0, 1) < rewiringProb)
                {
                    // do rewiring
                    bool found = false;
                    for (int z = 0; z < numHosts; z++)
                        if ((areInTheSameCommunity(i + 1, z + 1) == false)
                                && (interaction[i][z] < threshold) && (found == false) && i != z)
                        {
                            interaction[i][z] = interaction[z][i] = 1.0 - uniform(0, 1) * scalefactor; // give i,z a good score
                            found = true;
                        }
                }
            }
        }
}

// This function initializes the matrix creating a matrix composed of n disjoint community
// with nodes uniformly distributed, then performs re-wiring for each link with
// probability probRewiring. threshold is the value under which the relationship
// is not considered important

void MusolesiMobility::refreshCommunities()
{
    // initially distributes users uniformly into communities
    // Added a random distribution over the beginning of the cycle
    // or else re-wiring generates always the same communities

    for (int i = 0; i < numberOfCommunities; i++)
    {
        numberOfMembers[i] = 0;
    }

    // this revised way introduces some more randomization in the community selection
    // not really random, one between the previous of node X and the next of
    // node X will always be in the community of node X.

    int rnd = uniform(0, numHosts);
    int communitySize = numHosts / numberOfCommunities;
    int communityReminder = numHosts % numberOfCommunities;
    for (int i = 0; i < numberOfCommunities; i++)
    {
        for (int j = i * communitySize; j < i * communitySize + communitySize; j++)
        {
            communities[i][numberOfMembers[i]] = (j + rnd) % numHosts + 1;
            numberOfMembers[i] += 1;
        }
    }
    if (communityReminder) {
        for (int i = 0; i < communityReminder; i++)
        {
            communities[i][numberOfMembers[i]] = (numberOfCommunities * communitySize + i + rnd) % numHosts + 1;
            numberOfMembers[i] += 1;
        }
    }

    for (int i = 0; i < numHosts; i++)
        for (int j = 0; j < numHosts; j++)
            interaction[i][j] = -1;

    // for each node in the community, rewire with probability probRewiring with
    // somebody in another community
    double scalefactor = 0;
    if (threshold < 0.5)  // little number that depends on the threshold
        scalefactor = threshold;
    else
        scalefactor = (1 - threshold);

    for (int i = 0; i < numHosts; i++)
    {
        for (int j = 0; j < numHosts; j++)
        {
            if (interaction[i][j] < 0)
            {
                if (areInTheSameCommunity(i + 1, j + 1) == true)
                {
                    // choose a couple of nodes in the community
                    if (uniform(0, 1) < rewiringProb)
                    {
                        // do rewiring
                        bool found = false;
                        for (int z = 0; z < numHosts; z++)
                            if ((areInTheSameCommunity(i + 1, z + 1) == false)
                                    && (interaction[i][z] < threshold) && (found == false) && i != z)
                            {
                                interaction[i][z] = interaction[z][i] = 1.0 - uniform(0, 1) * scalefactor; // give i,z a good score
                                found = true;
                            }
                        interaction[i][j] = interaction[j][i] = (uniform(0, 1) * scalefactor);  // give i,j a bad score
                    }
                    else
                    {
                        // no rewiring
                        interaction[i][j] = interaction[j][i] = 1.0 - (uniform(0, 1) * scalefactor); // give i,j a good score
                    }
                }
                else
                {
                    // the hosts are not in the same community
                    interaction[i][j] = interaction[j][i] = (uniform(0, 1) * scalefactor); // give i,j a bad score
                }
            }
        } // if weight != -1 this number has already been assigned by previous iterations
    }
}

// Given the communities of nodes and two nodes "node1" and "node2" returns true if the hosts
// are in the same community, false otherwise
bool MusolesiMobility::areInTheSameCommunity(int node1, int node2)
{
    for (int k = 0; k < numberOfCommunities; k++)
        if (isInCommunity(node1, communities[k], numberOfMembers[k]) && isInCommunity(node2, communities[k], numberOfMembers[k]))
            return true;
    return false;
}

// Given a community of nodes and a node, stored in the array community of size equal
// to numberOfMembers, returns true if the node is in the community or false otherwise
bool MusolesiMobility::isInCommunity(int node, std::vector<int>& community, int numberOfMembers)
{
    for (int k = 0; k < numberOfMembers; k++)
        if (community[k] == node)
            return true;
    return false;
}
