#include "MusolesiMobility.h"
#include "sstream"
/*
 * Test done: nodes 1..100, groups 10
 *  nodes 100, groups 1..100
 *  nodes 30 thre 0.1..0.9
 *  nodes 30 thre 0.1 rewire 0.1..0.9 (rewiring doesn't change that much...)
 *  30 nodes, with deterministicOn 1/0. Always use 0, otherwise the nodes
 *  converge
 *
 */

Define_Module(MusolesiMobility);

// this members must be the same for each node. They are static and
// allocated/deallocated always by node zero

std::vector<std::vector<int> > MusolesiMobility::adjacency;
std::vector<hostsItem> MusolesiMobility::hosts;
std::vector<std::vector<cellsItem> > MusolesiMobility::cells;
std::vector<int> MusolesiMobility::numberOfMembers;
std::vector<std::vector<double> > MusolesiMobility::interaction;
std::vector<std::vector<int> > MusolesiMobility::groups;
std::map<int, int> MusolesiMobility::intervalDistribution;
std::map<int, int> MusolesiMobility::interContactDistribution;

simsignal_t MusolesiMobility::blocksHistogram = SIMSIGNAL_NULL;
simsignal_t MusolesiMobility::walkedMeters = SIMSIGNAL_NULL;
simsignal_t MusolesiMobility::blockChanges = SIMSIGNAL_NULL;

#define DETERMINISTIC 0
#define PSEUDODETERMINISTIC 1
#define PROPORTIONAL 2

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
        numberOfGroups = par("numberOfGroups");
        girvanNewmanOn = par("girvanNewmanOn");
        numHosts = par("numHosts");
        hcmm = par("hcmm");
        recordStartTime = par("recordStartTime");
        recordStatistics = par("recordStatistics");
        drift = par("drift");
        expmean = par("expmean");
        myGroup = -1;

        constraintAreaMin.y = par("constraintAreaMinY");
        constraintAreaMax.x = par("constraintAreaMaxX");
        constraintAreaMin.x = par("constraintAreaMinX");
        constraintAreaMax.y = par("constraintAreaMaxY");

        constraintAreaMax += constraintAreaMin;

        nodeId = getParentModule()->getIndex();
        RWP = par("RWP");

        initialrewiringPeriod = rewiringPeriod = par("rewiringPeriod");
        initialreshufflePeriod = reshufflePeriod = par("reshufflePeriod");
        reshufflePositionsOnly = par("reshufflePositionsOnly");

        if (constraintAreaMin.y != 0 || constraintAreaMin.x != 0)
            error("Musolesi implementation does not currently support top \
                        coordinates != from (0,0), please set constraintAreaY =\
                        constraintAreaX = 0");

        if (numberOfGroups > numHosts)
            error("You have set a number of hosts lower than the number of groups: %d < %d",numHosts,numberOfGroups);

        if (girvanNewmanOn == true)
            error("Girvan Newman algorithm not supported");

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
        if (hcmm > 0)
        {
            EV << "Using Boldrini/Conti/Passarella fix to original mobility" << std::endl;
            if (hcmm > 1)
                error("HCMM parameter is a probability, must be below 1");
        }

        // Note origin is top-left
        // Calculates the cell's side length
        sideLengthX = constraintAreaMax.x / ((double)numberOfColumns);
        sideLengthY = constraintAreaMax.y / ((double)numberOfRows);

        DefineStaticMembers();// just allocate static memory
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
    {
        setInitialPosition();
    }
    lastPosition.x = hosts[nodeId].currentX;
    lastPosition.y = hosts[nodeId].currentY;
}
void MusolesiMobility::DefineStaticMembers()
{
    //memory allocation
    a.resize(numberOfRows);
    CA.resize(numberOfRows);

    for (int i = 0; i < numberOfRows; i++)
    {
        a[i].resize(numberOfColumns);
        CA[i].resize(numberOfColumns);
    }
    //cell attractivity
    if (nodeId == 0)
    {
        //memory allocation for static data members
        hosts.resize(numHosts);
        numberOfMembers.resize(numHosts);
        cells.resize(numberOfRows);
        for (int i = 0; i < numberOfRows; i++)
            cells[i].resize(numberOfColumns);

        adjacency.resize(numHosts);
        interaction.resize(numHosts);
        groups.resize(numHosts);
        for (int i = 0; i < numHosts; i++)
        {
            adjacency[i].resize(numHosts);
            interaction[i].resize(numHosts);
            groups[i].resize(numHosts);
        }
        // setup the speeds and areas
        for (int i = 0; i < numHosts; i++)
            hosts[i].speed = uniform(minHostSpeed, maxHostSpeed);
        for (int i = 0; i < numberOfRows; i++)
        {
            for (int j = 0; j < numberOfColumns; j++)
            {
                cells[i][j].pos.x = 1.0 * j * sideLengthX;
                cells[i][j].pos.y = 1.0 * i * sideLengthY;
                cells[i][j].numberOfHosts = 0;
            }
        }
    }
}

void MusolesiMobility::setTargetPosition()
{
    int selectedGoalCellX = 0;
    int selectedGoalCellY = 0;
    int previousGoalCellX = hosts[nodeId].cellIdX;
    int previousGoalCellY = hosts[nodeId].cellIdY;

    if (RWP)
    {
        selectedGoalCellX = uniform(0, numberOfRows);
        selectedGoalCellY = uniform(0, numberOfColumns);
    }
    else if ((hcmm > uniform(0, 1)) && (previousGoalCellY != hosts[nodeId].homeCellIdX)
            && (previousGoalCellX != hosts[nodeId].homeCellIdY))
    {
        // Boldrini variation, set a probability for each node to go back home
        // when a target has been reached. In Boldrini it could be anytime when
        // the node is out of home sampled at the rate
        selectedGoalCellX = hosts[nodeId].homeCellIdX;
        selectedGoalCellY = hosts[nodeId].homeCellIdY;
    }
    else
    {
        // update attraction matrix
        for (int c = 0; c < numberOfRows; c++)
        {
            for (int r = 0; r < numberOfColumns; r++)
            {
                CA[c][r] = 0.0;
                a[c][r] = 0.0;
            }
        }
        for (int n = 0; n < numHosts; n++)
            if (n != nodeId)
                CA[hosts[n].cellIdX][hosts[n].cellIdY] += interaction[nodeId][n];

        // The deterministic targetChoice only works with high hcmm
        if (targetChoice == DETERMINISTIC)
        {
            double CAMax = 0;
            for (int c = 0; c < numberOfRows; c++)
            {
                for (int r = 0; r < numberOfColumns; r++)
                {
                    if (cells[c][r].numberOfHosts != 0)
                    {
                        CA[c][r] = CA[c][r] / (double) cells[c][r].numberOfHosts;
                    }
                    else
                        CA[c][r] = 0;
                }
            }
            for (int c = 0; c < numberOfRows; c++)
            {
                for (int r = 0; r < numberOfColumns; r++)
                {
                    // this iteration has the best cell
                    if (CA[c][r] > CAMax)
                    {
                        selectedGoalCellX = c;
                        selectedGoalCellY = r;

                        CAMax = CA[c][r];

                    }
                }
            }
        }
        //end deterministic;
        // Pseudodeterministic choice. Order the cells with attraction != 0
        // for their attraction, then extract a random value with exponential
        // distribution (expmean defined before, recall average = 1/expmean)
        // that has a strong bias on the most attractive ones, but adds some
        // turbulence to the choice, (not that much as the uniform)
        else if (targetChoice == PSEUDODETERMINISTIC)
        {
            std::map<double, std::pair<int, int> > orderedCellSet;
            std::map<double, std::pair<int, int> >::iterator oSeti;

            for (int c = 0; c < numberOfRows; c++)
                for (int r = 0; r < numberOfColumns; r++)
                    if (CA[c][r] != 0)
                        orderedCellSet[CA[c][r]] = std::pair<int, int>(c, r);

            unsigned int rnd = exponential((double) expmean);
            if (rnd >= orderedCellSet.size())
            {
                rnd = orderedCellSet.size() - 1;
            }
            oSeti = orderedCellSet.begin();
            std::advance(oSeti,rnd);
            selectedGoalCellX = (oSeti->second.first);
            selectedGoalCellY = (oSeti->second.second);
        }
        else if (targetChoice == PROPORTIONAL)
        {
            // Probabilistic target selection. Each node selects a target in a
            // square. The choice is random with probability distribution
            // proportional to the attractivity of the squares.

            //Algorithm of the selection of the new cell
            //Denominator for the normalization of the values
            float denNorm = 0.00;
            // this is the added probability to choose an empty square
            // calculate normalized attractivity denomitor, plus drift for squares
            // that have 0 attractivity.
            for (int c = 0; c < numberOfRows; c++)
            {
                for (int r = 0; r < numberOfColumns; r++)
                {
                    denNorm = denNorm + CA[c][r] + drift;
                }
            }
            // calculate normalized attractivity for each square,
            // assign a value of a distribution from 0 to 1 proportional to the
            // normalized attractivity to each square, get a random square with
            // probability proportional to the normalized attractivity. This
            // has been rewritten compared to original code. It was made of
            // too much for(), now it is quicker, just one cycle.
            // You can imagine squares as an array, extract a random value
            // (total ca is normalized by deNorm to 1), get the element of
            // the array for which cumulative probability is higher than random
            //
            // Note, each cell with some node has some attractivity, in the
            // previous case the best cell is chosen, in this case, even a cell
            // with little attractivity may be chosen
            float infiniteDice = (float) uniform(0.0, 1.0);
            double totalInterest = 0;
            bool goOut = 0;
            for (int c = 0; c < numberOfRows; c++)
            {
                for (int r = 0; r < numberOfColumns; r++)
                {
                    a[c][r] = (CA[c][r] + drift) / (+denNorm);
                    totalInterest += a[c][r];
                    if (infiniteDice < totalInterest)
                    {
                        selectedGoalCellX = c;
                        selectedGoalCellY = r;
                        goOut = 1;
                        break;
                    }

                }
                if (goOut)
                    break;
            }
        }
    }

    //Re-definition of the number of hosts in each cell
    cells[previousGoalCellX][previousGoalCellY].numberOfHosts -= 1;
    cells[selectedGoalCellX][selectedGoalCellY].numberOfHosts += 1;

    if (recordStatistics && (previousGoalCellX != selectedGoalCellX || previousGoalCellY != selectedGoalCellY))
        emit(blockChanges, 1);

    //refresh of the information
    hosts[nodeId].cellIdX = selectedGoalCellX;
    hosts[nodeId].cellIdY = selectedGoalCellY;

    Coord randomPoint = getRandomPoint(cells[selectedGoalCellX][selectedGoalCellY].pos);

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
        sprintf(buf, "group:%d, tgt:%d,%d", myGroup, hosts[nodeId].cellIdX, hosts[nodeId].cellIdY);
        getParentModule()->getDisplayString().setTagArg("t", 0, buf);

        for (int i = 0; i < numberOfGroups; i++)
        {
            for (int j = 0; j < numberOfMembers[i]; j++)
                if (groups[i][j] - 1 == nodeId)
                    myGroup = i + 1;
        }
        int groupSpan = 256 / numberOfGroups;

        sprintf(buf, "#%.2x%.2x%.2x", groupSpan * myGroup % 256,
                (groupSpan * (numberOfGroups / 3) + groupSpan * myGroup) % 256,
                (groupSpan * (numberOfGroups * 2 / 3) + groupSpan * myGroup) % 256);
        getParentModule()->getDisplayString().setTagArg("t", 2, buf);
        getParentModule()->getDisplayString().setTagArg("i", 1, buf);
    }
    else
        MobilityBase::handleMessage(msg);
}

void MusolesiMobility::handleSelfMsg(cMessage * msg)
{
    if (simTime() > rewiringPeriod && initialrewiringPeriod > 0 && nodeId == 0)
    {
        setPosition(false);
        rewire();
        rewiringPeriod += initialrewiringPeriod;
    }
    if (simTime() > reshufflePeriod && initialreshufflePeriod > 0 && nodeId == 0)
    {
        if(reshufflePositionsOnly)
            setPosition(true);
        else
            rewire();

        reshufflePeriod += initialreshufflePeriod;
    }
    int myCurrCell = -1;
    if (recordStatistics)
    {
        if (simTime() > recordStartTime)
        {
            for (int i = 0; i < numHosts; i++)
            {
                if (i == nodeId)
                    continue;
                if (hosts[nodeId].cellIdX == hosts[i].cellIdX && hosts[nodeId].cellIdY == hosts[i].cellIdY)
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
        myCurrCell = ((int) (lastPosition.x / sideLengthX)) % numberOfColumns
                + (((int) (lastPosition.y / sideLengthY)) % numberOfRows) * numberOfColumns;
        if (recordStatistics && ((int) simTime().dbl()) % 10 == 0) // log every 10 sec
            emit(blocksHistogram, myCurrCell);
    }
    scheduleAt(simTime() + 1, msg);
}

// @Brief Set the initial position of the nodes, or shuffle the groups (if shuffle==1)
void MusolesiMobility::setInitialPosition()
{
    // NOTE: the playground for omnet has zero coordinate on top left.
    // consequently, the cells are always addressed as a matrix indexed
    // using top-left element as 0,0
    double gridSizex = (constraintAreaMax.x - constraintAreaMin.x) / numberOfColumns;

    cDisplayString& parentDispStr = getParentModule()->getParentModule()->getDisplayString();

    std::stringstream buf;
    buf << "bgg=" << int(gridSizex);
    parentDispStr.parse(buf.str().c_str());

    refresh_weight_array_ingroups();
    generate_adjacency();

    for (int i = 0; i < numberOfGroups; i++)
    {
        ev << "The members of group "<< i + 1 << " are: ";
        for (int j = 0; j < numberOfMembers[i]; j++)
            ev << groups[i][j]-1 << " ";
        ev << std::endl;
    }
    for (int i = 0; i < numberOfGroups; i++)
    {
        int cellIdX = uniform(0, numberOfRows);
        int cellIdY = uniform(0, numberOfColumns);
        for (int j = 0; j < numberOfMembers[i]; j++)
        {
            int hostId = groups[i][j];
            hosts[hostId - 1].homeCellIdX = cellIdX;
            hosts[hostId - 1].homeCellIdY = cellIdY;
            //increment the number of the hosts in that cell
            cells[cellIdX][cellIdY].numberOfHosts += 1;
            hosts[hostId - 1].cellIdX = cellIdX;
            hosts[hostId - 1].cellIdY = cellIdY;
            ev<< "Setting home of " << hostId << " in position " << hosts[hostId-1].homeCellIdX << " " << hosts[hostId-1].homeCellIdY << std::endl;
            ev<< "Setting pos  of " << hostId << " in position " << hosts[hostId-1].cellIdX << " " << hosts[hostId-1].cellIdY << std::endl;
        }
    }
    //definition of the initial position of the hosts
    for (int k = 0; k < numHosts; k++)
    {
        Coord randomPoint = getRandomPoint(cells[hosts[k].cellIdX][hosts[k].cellIdY].pos);
        hosts[k].currentX = randomPoint.x;
        hosts[k].currentY = randomPoint.y;
    }
}

void MusolesiMobility::setPosition(bool reshufflePositionOnly)
{
    //communities based on the initial number of caves in the Caveman model
    //i.e., w=0
    if(!reshufflePositionOnly)
    {
        refresh_weight_array_ingroups();
        generate_adjacency();
        for (int i = 0; i < numberOfGroups; i++)
        {
            ev << "The members of group " << i+1 << " are: ";
            for (int j = 0;j < numberOfMembers[i]; j++)
                ev << groups[i][j]-1 << " ";
            ev << std::endl;
        }
    }
    for (int i = 0; i < numberOfGroups; i++)
    {
        int cellIdX = uniform(0, numberOfRows);
        int cellIdY = uniform(0, numberOfColumns);
        for (int j = 0; j < numberOfMembers[i]; j++)
        {
            int hostId = groups[i][j];
            hosts[hostId - 1].homeCellIdX = cellIdX;
            hosts[hostId - 1].homeCellIdY = cellIdY;
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
        {
            if (i == nodeId)
                continue;
            if (nodesInMyBlock[i].first != 0)
            {
                intervalDistribution[(int) (10 * (simTime().dbl() - nodesInMyBlock[i].first))]++;
            }
        }
    }
    if (nodeId == 0)
    {
        int totalFreq = 0;
        double partialFreq = 0;
        if (recordStatistics)
        {
            std::cout << "#CONTACTTIME" << std::endl;
            // print statistics
            for (std::map<int, int>::iterator ii = intervalDistribution.begin(); ii != intervalDistribution.end(); ii++)
            {
                totalFreq += ii->second;
            }
            for (std::map<int, int>::iterator ii = intervalDistribution.begin(); ii != intervalDistribution.end(); ii++)
            {
                partialFreq += ii->second;
                std::cout << ii->first << " " << ii->second << " " << 1 - partialFreq / totalFreq << std::endl;
            }
            totalFreq = 0;
            partialFreq = 0;
            std::cout << "#INTERCTIME" << std::endl;
            for (std::map<int, int>::iterator ii = interContactDistribution.begin();
                    ii != interContactDistribution.end(); ii++)
            {
                totalFreq += ii->second;
            }
            for (std::map<int, int>::iterator ii = interContactDistribution.begin();
                    ii != interContactDistribution.end(); ii++)
            {
                partialFreq += ii->second;
                std::cout << ii->first << " " << ii->second << " " << 1 - partialFreq / totalFreq << std::endl;
            }
            std::cout << "#ENDSTATISTICS" << std::endl;

        }
    }
    cancelAndDelete(moveMessage);
}

MusolesiMobility::~MusolesiMobility()
{
    for(int i = 0; i < numberOfRows; i++)
    {
        a[i].clear();
        CA[i].clear();
        cells[i].clear();
    }
    a.clear();
    CA.clear();
    cells.clear();
    for (int i = 0; i < numHosts; i++)
    {
        adjacency[i].clear();
        interaction[i].clear();
        groups[i].clear();
    }
    adjacency.clear();
    interaction.clear();
    groups.clear();
    numberOfMembers.clear();
    hosts.clear();
    intervalDistribution.clear();
    interContactDistribution.clear();
}

// in this rewiring I do not unlink the nodes from their current group, I just
// add links to other nodes in other groups. This way the network keeps being
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
            if (areInTheSameGroup(i + 1, j + 1) == true)
            { // chose a couple of nodes in the group
                if (uniform(0, 1) < rewiringProb)
                {   // do rewiring
                    bool found = false;
                    for (int z = 0; z < numHosts; z++)
                        if ((areInTheSameGroup(i + 1, z + 1) == false)
                                && (interaction[i][z] < threshold) && (found == false) && i != z)
                        {
                            interaction[i][z] = interaction[z][i] = 1.0 - uniform(0, 1) * scalefactor; // give i,z a good score
                            found = true;
                        }
                }
            }
        }
}

//This function initialize the matrix creating a matrix composed of n disjointed groups
//with nodes uniformly distributed, then performs re-wiring for each link with
//probability probRewiring. threshold is the value under which the relationship
//is not considered important

void MusolesiMobility::refresh_weight_array_ingroups()
{

    // initially distributes users uniformly into groups
    // Added a random distribution over the beginning of the cycle
    // or else re-wiring generates always the same groups

    for (int i = 0; i < numberOfGroups; i++)
    {
        numberOfMembers[i] = 0;
    }

    // this revised way introduces some more randomization in the group selection
    // not really random, one between the previous of node X and the next of
    // node X will always be in the group of node X.

    int rnd = uniform(0, numHosts);
    int groupSize = numHosts / numberOfGroups;
    int groupReminder = numHosts % numberOfGroups;
    for (int i = 0; i < numberOfGroups; i++)
    {
        for (int j = i * groupSize; j < i * groupSize + groupSize; j++)
        {
            groups[i][numberOfMembers[i]] = (j + rnd) % numHosts + 1;
            numberOfMembers[i] += 1;
        }
    }
    if (groupReminder)
        for (int i = 0; i < groupReminder; i++)
        {
            groups[i][numberOfMembers[i]] = (numberOfGroups * groupSize + i + rnd) % numHosts + 1;
            numberOfMembers[i] += 1;
        }

    for (int i = 0; i < numHosts; i++)
        for (int j = 0; j < numHosts; j++)
            interaction[i][j] = -1;

    // for each node in the group, rewire with probability probRewiring with
    // somebody in another group
    double scalefactor = 0;
    if (threshold < 0.5)  // little number that depends on the threshold
        scalefactor = threshold;
    else
        scalefactor = (1 - threshold);

    for (int i = 0; i < numHosts; i++)
        for (int j = 0; j < numHosts; j++)
        {
            if (interaction[i][j] < 0)
            {
                if (areInTheSameGroup(i + 1, j + 1) == true)
                { // chose a couple of nodes in the group
                    if (uniform(0, 1) < rewiringProb)
                    {   // do rewiring

                        bool found = false;
                        for (int z = 0; z < numHosts; z++)
                            if ((areInTheSameGroup(i + 1, z + 1) == false)
                                    && (interaction[i][z] < threshold) && (found == false) && i != z)
                            {
                                interaction[i][z] = interaction[z][i] = 1.0 - uniform(0, 1) * scalefactor; // give i,z a good score
                                found = true;
                            }
                        interaction[i][j] = interaction[j][i] = (uniform(0, 1) * scalefactor);  // give i,j a bad score
                    }
                    else
                    {  //no rewiring
                        interaction[i][j] = interaction[j][i] = 1.0 - (uniform(0, 1) * scalefactor); // give i,j a good score
                    }
                }
                else
                { //the hosts are not in the same cluster
                    interaction[i][j] = interaction[j][i] = (uniform(0, 1) * scalefactor); // give i,j a bad score
                }
            }
        } // if weight != -1 this number has already been assigned by previous iterations
}

//generate the adjacency matrix from the weight matrix of size array_size given a certain threshold
void MusolesiMobility::generate_adjacency()
{

    for (int i = 0; i < numHosts; i++)
    {
        for (int j = 0; j < numHosts; j++)
        {
            if (interaction[i][j] > threshold)
                adjacency[i][j] = 1;
            else
                adjacency[i][j] = 0;
        }
    }
}

//Given the groups of nodes and two nodes "node1" and "node2" returns true if the hosts
//are in the same group, false otherwise
bool MusolesiMobility::areInTheSameGroup(int node1, int node2)
{
    for (int k = 0; k < numberOfGroups; k++)
    {

        if (isInGroup(node1, groups[k], numberOfMembers[k]))
        {
            if (isInGroup(node2, groups[k], numberOfMembers[k]))
            {
                return true;
            }
        }
    }
    return false;
}

//Given a group of nodes and a node, stored in the array group of size equal
//to numberOfMembers, returns true if the node is in the group or false otherwise
bool MusolesiMobility::isInGroup(int node, std::vector<int>& group, int numberOfMembers)
{
    for (int k = 0; k < numberOfMembers; k++)
    {
        if (group[k] == node)
        {
            return true;
        }
    }
    return false;
}
