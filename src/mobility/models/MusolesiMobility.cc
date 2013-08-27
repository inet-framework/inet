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

int ** MusolesiMobility::adjacency;
hostsItem * MusolesiMobility::hosts;
cellsItem ** MusolesiMobility::cells;
int * MusolesiMobility::numberOfMembers;
double ** MusolesiMobility::interaction;
int ** MusolesiMobility::groups;
std::map<int,int> MusolesiMobility::intervalDistribution;
std::map<int,int> MusolesiMobility::interContactDistribution;

simsignal_t MusolesiMobility::blocksHistogram = SIMSIGNAL_NULL;
simsignal_t MusolesiMobility::walkedMeters = SIMSIGNAL_NULL;
simsignal_t MusolesiMobility::blockChanges = SIMSIGNAL_NULL;


#define DETERMINISTIC 0
#define PSEUDODETERMINISTIC 1
#define PROPORTIONAL 2

// If Girvan Newman is actived, uncomment this. GN, in this release does not
// work, feel free to debug it.
//
//#define GN


void MusolesiMobility::initialize(int stage)
{

    EV << "initializing MusolesiMobility stage " << stage << endl;
    stationary = false; // this stuff is needed by LineSegmentsMobilityBase
	obstacleAvoidance = par("obstacleAvoidance").boolValue();
	LineSegmentsMobilityBase::initialize(stage, obstacleAvoidance);

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

        constraintAreaMin.y = par("constraintAreaY");
        constraintAreaMax.x = par("constraintAreaSizeX");
        constraintAreaMin.x = par("constraintAreaX");
        constraintAreaMax.y = par("constraintAreaSizeY");
        constraintAreaMax += constraintAreaMin;
        nodeId = getParentModule()->getIndex();
        RWP = par("RWP"); 

        rewiringPeriod = par("rewiringPeriod");
        reshufflePeriod = par("reshufflePeriod");
        reshufflePositionsOnly = par("reshufflePositionsOnly");

        initialrewiringPeriod = rewiringPeriod;
        initialreshufflePeriod = reshufflePeriod;
        if(constraintAreaMin.y != 0 ||  constraintAreaMin.x != 0)
            error("Musolesi implementation does not currently support top \
                    coordinates != from (0,0), please set constraintAreaY =\
                    constraintAreaX = 0"); 

                if(numberOfGroups > numHosts)
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

        if (hcmm > 0){
            EV <<  "Using Boldrini/Conti/Passarella fix to original mobility" << std::endl;
            if (hcmm > 1)
                error("HCMM parameter is a probability, must be below 1");
        }

        // Note origin is top-left
        sideLengthX=constraintAreaMax.x/((double)numberOfColumns);
        sideLengthY=constraintAreaMax.y/((double)numberOfRows);
        DefineStaticMembers(); // just allocate static memory
        if(recordStatistics)
            for (int i = 0; i< numHosts; i++)
                nodesInMyBlock[i].first = nodesInMyBlock[i].second = 0;


        moveMessage = new cMessage("MusolesiHartbeat");
        scheduleAt(simTime() + 1, moveMessage); // do statistics, reshuffle, rewire...
        blocksHistogram = registerSignal("blocksHistogram");
        walkedMeters= registerSignal("walkedMeters");
        blockChanges= registerSignal("blockChanges");


    }


}

void MusolesiMobility::initializePosition(){ 
        if (nodeId == 0){ // maps are static so just one node is required to calculate them 
            setInitialPosition(false);
        }
        lastPosition.x = hosts[nodeId].currentX;
        lastPosition.y = hosts[nodeId].currentY;
}
void MusolesiMobility::DefineStaticMembers(){

    //cell attractivity
    if (nodeId == 0){
        cells = (cellsItem**)malloc(sizeof(cellsItem*)*numberOfRows);
        hosts = (hostsItem*)malloc(sizeof(hostsItem)*numHosts);
        numberOfMembers = (int*)malloc(sizeof(int)*numHosts);

        for (int i = 0; i< numberOfRows; i++){
            cells[i] = (cellsItem*)malloc(sizeof(cellsItem)*numberOfColumns);
        }
        for (int i=0; i<numHosts; i++)
            hosts[i].speed=uniform(minHostSpeed,maxHostSpeed);
        

        // setup the areas
        for (int i=0;i<numberOfRows;i++)
            for (int j=0;j<numberOfColumns;j++) {
                cells[i][j].minX=((double)j)*sideLengthX;
                cells[i][j].minY=((double)i)*sideLengthY;
                cells[i][j].numberOfHosts=0;
            }	
        adjacency=initialise_int_array(numHosts);
        interaction=initialise_double_array(numHosts);
    }

    //probability of moving to the cell [c][r]
    a = (double**)malloc(sizeof(double*)*numberOfRows);
    CA = (double**)malloc(sizeof(double*)*numberOfRows);

    for (int i = 0; i< numberOfRows; i++){
        a[i] = (double*)malloc(sizeof(double)*numberOfColumns);
        CA[i] = (double*)malloc(sizeof(double)*numberOfColumns);
    }
}

//
void MusolesiMobility::setTargetPosition()
{
    int selectedGoalCellX=0;
    int selectedGoalCellY=0;	
    int previousGoalCellX=hosts[nodeId].cellIdX;
    int previousGoalCellY=hosts[nodeId].cellIdY;

    if (RWP){
        selectedGoalCellX = uniform(0,numberOfRows);
        selectedGoalCellY = uniform(0,numberOfColumns);
    } else if ((hcmm > uniform(0,1)) && (previousGoalCellY != hosts[nodeId].myCellIdX) && (previousGoalCellX != hosts[nodeId].myCellIdY)){ 
        // Boldrini variation, set a probability for each node to go back home
        // when a target has been reached. In Boldrini it could be anytime when
        // the node is out of home sampled at the rate 
        selectedGoalCellX = hosts[nodeId].myCellIdX;
        selectedGoalCellY = hosts[nodeId].myCellIdY;
    } else {

        // update attraction matrix
        for (int c=0;c<numberOfRows;c++)
            for (int r=0;r<numberOfColumns;r++){
                CA[c][r]=0.0;
                a[c][r]=0.0;
            }

        for (int n=0;n<numHosts;n++)
            if ( n != nodeId )
                CA[hosts[n].cellIdX][hosts[n].cellIdY]+=interaction[nodeId][n];

        // This does not work, the nodes after a while converge to some heavyweight
        // squares and get stuck there.
        if (targetChoice == DETERMINISTIC) { 

            int selectedGoalCellX2=0;
            int selectedGoalCellY2=0;
            double CAMax1=0;
            double CAMax2=0;
            for (int c=0;c<numberOfRows;c++) 
                for (int r=0;r<numberOfColumns;r++) {
                    if (cells[c][r].numberOfHosts!=0){
                        CA[c][r]=CA[c][r]/(double)cells[c][r].numberOfHosts;
                    }
                    else
                        CA[c][r]=0;
                }
            for (int c=0;c<numberOfRows;c++)
                for (int r=0;r<numberOfColumns;r++)
                    if (CA[c][r]>CAMax1) { // this iteration has the best cell 

                        //set the second best
                        selectedGoalCellX2=selectedGoalCellX;
                        selectedGoalCellY2=selectedGoalCellY;
                        CAMax2=CAMax1;

                        selectedGoalCellX=c;
                        selectedGoalCellY=r;

                        CAMax1=CA[c][r];

                    }
                    else if (CA[c][r]>CAMax2) { // this iteration has the second best cell
                        selectedGoalCellX2=c;
                        selectedGoalCellY2=r;
                        CAMax2=CA[c][r];
                    }	

        } //end deterministic; 
          // pseudo deterministic choice. Order the cells with attraction != 0
          // for their attraction, then estract a random value with exponential
          // distribution (expmean defined before, recall average = 1/expmean)
          // that has a strong bias on the most attractive ones, but adds some
          // turbolence to the choice, (not that much as the uniform)
        else if (targetChoice == PSEUDODETERMINISTIC) { 
            std::map<double, std::pair<int,int> > orderedCellSet;
            std::map<double, std::pair<int,int> >::iterator oSeti;

            for (int c=0;c<numberOfRows;c++) 
                for (int r=0;r<numberOfColumns;r++) 
                    if (CA[c][r] != 0)
                        orderedCellSet[CA[c][r]] = std::pair<int,int>(c,r);
            unsigned int rnd = exponential((double)expmean);
            if (rnd >= orderedCellSet.size()){
                rnd = orderedCellSet.size()-1;
            } 
            // god forgive me for this
            oSeti = orderedCellSet.begin();
            for (int i = 0; i<rnd; i++)
                oSeti++;
            selectedGoalCellX=(oSeti->second.first);
            selectedGoalCellY=(oSeti->second.second);
        }else if (targetChoice == PROPORTIONAL) { 
            // Probabilistic target selection. Each node selects a target in a
            // square. The choice is randomic with probability distribution
            // proportional to the attracrivity of the squares.

            //Algorithm of the selection of the new cell
            //Denonmiantor for the normalization of the values
            float denNorm=0.00;
            // this is the added probability to choose an empty square
            // calculate normalized attractivity denomitor, plus drift for squares
            // that have 0 attractivity. 
            for (int c=0;c<numberOfRows;c++)
                for (int r=0;r<numberOfColumns;r++) {
                    denNorm=denNorm+CA[c][r]+drift;
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
            // with little atractivity may be chosen
            float infiniteDice = (float) uniform(0.0,1.0);
            double totalInterest = 0;
            bool goOut = 0;
            for (int c=0;c<numberOfRows;c++){
                for (int r=0;r<numberOfColumns;r++){
                    a[c][r]=(CA[c][r]+drift)/(+denNorm);
                    totalInterest += a[c][r];
                    if (infiniteDice < totalInterest){
                        selectedGoalCellX=c;
                        selectedGoalCellY=r;
                        goOut =1;
                        break;
                    }

                }
                if (goOut)
                    break;
            }
        } 
    }

    //Re-definition of the number of hosts in each cell
    cells[previousGoalCellX][previousGoalCellY].numberOfHosts-=1;
    cells[selectedGoalCellX][selectedGoalCellY].numberOfHosts+=1;

    if (recordStatistics && (previousGoalCellX != selectedGoalCellX ||
    		previousGoalCellY != selectedGoalCellY) )
    {
    	emit(blockChanges,1);
    }
    //refresh of the information
    hosts[nodeId].cellIdX=selectedGoalCellX;
    hosts[nodeId].cellIdY=selectedGoalCellY;

    Coord randomPoint = getRandomPointWithObstacles(
    		cells[selectedGoalCellX][selectedGoalCellY].minX,
    		cells[selectedGoalCellX][selectedGoalCellY].minY);

    //double newGoalRelativeX=uniform(0,sideLengthX);
    //double newGoalRelativeY=uniform(0,sideLengthY);

    hosts[nodeId].goalCurrentX=randomPoint.x;
    hosts[nodeId].goalCurrentY=randomPoint.y;

    targetPosition.x =  hosts[nodeId].goalCurrentX;
    targetPosition.y =  hosts[nodeId].goalCurrentY;

    hosts[nodeId].speed = uniform(minHostSpeed,maxHostSpeed);
    double distance = lastPosition.distance(targetPosition);
    emit(walkedMeters, distance);
    simtime_t travelTime = distance / hosts[nodeId].speed;
    if (nextChange > 0 )
        nextChange += travelTime;
    else 
        nextChange = travelTime;
}

void MusolesiMobility::fixIfHostGetsOutside()
{
    if (lastPosition.x < 0 || lastPosition.y < 0 || lastPosition.x > constraintAreaMax.x || lastPosition.y > constraintAreaMax.y)
        EV << "The last step of the mobility was pointing out of the playground, adjusted to margin" << std::endl;
    if (lastPosition.x < 0 )
        lastPosition.x = 0;
    if (lastPosition.y < 0 )
        lastPosition.y = 0;
    if (lastPosition.x > constraintAreaMax.x)
        lastPosition.x = constraintAreaMax.x;
    if (lastPosition.y > constraintAreaMax.y)
        lastPosition.y  = constraintAreaMax.y;
}

void MusolesiMobility::handleMessage(cMessage * msg){
    if (msg == moveMessage){
        handleSelfMsg(msg);
        char buf[40];

        double relXcolor = hosts[nodeId].myCellIdX;
        double relYcolor = hosts[nodeId].myCellIdY;
        relXcolor = 256*(relXcolor/numberOfRows);
        relYcolor = 256*(relYcolor/numberOfColumns);
        sprintf(buf, "group:%d, tgt:%d,%d",myGroup,
        		hosts[nodeId].cellIdX, hosts[nodeId].cellIdY);
        getParentModule()->getDisplayString().setTagArg("t", 0, buf);

        for (int i=0;i<numberOfGroups;i++) {
            for (int j=0;j<numberOfMembers[i];j++)
                if (groups[i][j]-1 == nodeId)
                	myGroup = i+1;
        }
        int groupSpan = 256/numberOfGroups;

        sprintf(buf, "#%.2x%.2x%.2x",groupSpan*myGroup%256,
        		(groupSpan*(numberOfGroups/3)+groupSpan*myGroup)%256,
        		(groupSpan*(numberOfGroups*2/3)+groupSpan*myGroup)%256);
        getParentModule()->getDisplayString().setTagArg("t", 2, buf);
        getParentModule()->getDisplayString().setTagArg("i", 1, buf);
    }
    else 
        MobilityBase::handleMessage(msg);
}

void MusolesiMobility::handleSelfMsg(cMessage * msg){
    if (simTime() > rewiringPeriod && initialrewiringPeriod > 0 && nodeId == 0)
    {
            setInitialPosition(true);
            rewire(interaction,  numHosts, rewiringProb, 
                     threshold,  groups,  numberOfMembers, numberOfGroups);
        rewiringPeriod += initialrewiringPeriod;
    }
    if (simTime() > reshufflePeriod && initialreshufflePeriod > 0 && nodeId == 0)
    {
        if(reshufflePositionsOnly)
        {
            for (int i=0;i<numberOfGroups;i++) {
                int cellIdX=uniform(0,numberOfRows);
                int cellIdY=uniform(0,numberOfColumns);
                for (int j=0;j<numberOfMembers[i];j++) {
                    int hostId=groups[i][j];
                    hosts[hostId-1].myCellIdX = cellIdX;
                    hosts[hostId-1].myCellIdY = cellIdY;
                }
            }
   
        } else
            rewire(interaction,  numHosts, rewiringProb, 
                    threshold,  groups,  numberOfMembers, numberOfGroups);

        reshufflePeriod += initialreshufflePeriod;
    }

    int myCurrCell = -1;
    if(recordStatistics){
        if (simTime() > recordStartTime){
            for (int i=0; i<numHosts; i++)
            {
                if (i == nodeId)
                    continue;
                if (hosts[nodeId].cellIdX == hosts[i].cellIdX &&
                        hosts[nodeId].cellIdY == hosts[i].cellIdY ) {
                    if (nodesInMyBlock[i].first == 0){ 
                        // was him in my block?, no
                        nodesInMyBlock[i].first = simTime().dbl(); // add in the map
                        interContactDistribution[(int)(10*(simTime().dbl()-nodesInMyBlock[i].second))]++;
                        nodesInMyBlock[i].second = simTime().dbl(); // add in the map
                    }
                } else  if (nodesInMyBlock[i].first != 0){ // somebody is not in my block, but used to be
                    // the map maps decades of seconds to adjacency intervals
                    intervalDistribution[(int)(10*(simTime().dbl()-nodesInMyBlock[i].first))]++;
                    nodesInMyBlock[i].second = simTime().dbl();
                    nodesInMyBlock[i].first = 0;
                }
            }
        }
        myCurrCell = ((int)(lastPosition.x/sideLengthX))%numberOfColumns + 
            (((int)(lastPosition.y/sideLengthY))%numberOfRows)*numberOfColumns;
        if (recordStatistics &&
        		((int)simTime().dbl())%10 == 0) // log every 10 sec
        	emit(blocksHistogram,myCurrCell);

    }

    scheduleAt(simTime() + 1, msg);

}

// @Brief Set the initial position of the nodes, or shuffle the groups (if shuffle==1)
void MusolesiMobility::setInitialPosition(bool shuffle)
{
    bool splitted=true;
    double previousModth=0.0;
    double modth=0.1;
    int initialNumberOfGroups = numberOfGroups;

    if(!shuffle){
        for (int i=0;i<numberOfRows;i++) // NOTE: the playground for omnet has zero coordinate on top left.
        								 // consequently, the cells are always addressed as a matrix indexed
        								 // using top-left element as 0,0
            for (int j=0;j<numberOfColumns;j++) 
                cells[i][j].numberOfHosts=0;
        groups=initialise_int_array(numHosts);
        double gridSizex, gridSizeY;
        gridSizex = (constraintAreaMax.x - constraintAreaMin.x)/numberOfColumns;

        cDisplayString& parentDispStr = getParentModule()->getParentModule()->getDisplayString();

        std::stringstream buf;
        buf << "bgg=" << int(gridSizex);
        parentDispStr.parse(buf.str().c_str());
    }

    //clustering using the Girvan-Newman algorithm
    //
    //I have doubts about the use of GN algorithm. It seems that it does
    //only one iteration, and I've verified it also in the original code
    //by Musolesi. So, Or there is something I'm missing in the
    //configuration of the parameters, or this doesn't work. I Disabled
    //it by now (see initialize)
    if (girvanNewmanOn==true) {	
#ifdef GN
        //This function initialise the matrix creating a matrix composed of n disjointed groups 
        //with nodes uniformely distributed, then performs rewiring for each link with
        //prob probRewiring. threshold is the value under which the relationship 
        //is not considered important
        initialise_weight_array_ingroups(interaction,numHosts,initialNumberOfGroups,rewiringProb,threshold);

        // transform the float matrix into a binary matrix
        generate_adjacency(interaction,adjacency,threshold,numHosts);					
        do {
            for (int i=0;i<numHosts; i++)
                numberOfMembers[i]=0;
            splitted=false;
            double betw[numHosts];
            for (int i=0;i<numHosts; i++)
                betw[i]=0;
            calculate_betweenness(betw,adjacency,numHosts);

            for (int i=0;i<numHosts; i++)
                numberOfMembers[i]=0;
            numberOfGroups=getGroups(adjacency, groups ,numberOfMembers,numHosts);
            previousModth=modth;
            modth=splitNetwork_Threshold(adjacency, betw, numHosts, modth);
        } while ( (previousModth<modth) && (modth > -1) );
#endif 
    }
    else {
        //communities based on the initial number of caves in the Caveman model		
        //i.e., w=0
        refresh_weight_array_ingroups(interaction,numHosts,initialNumberOfGroups,rewiringProb,threshold, groups, numberOfMembers);
        generate_adjacency(interaction,adjacency,threshold,numHosts);					
    }
    for (int i=0;i<numberOfGroups;i++) {
        ev <<"The members of group "<<i+1<<" are: ";
        for (int j=0;j<numberOfMembers[i];j++)
            ev <<groups[i][j]-1<<" ";
        ev << std::endl;
    }
    for (int i=0;i<numberOfGroups;i++) {
        int cellIdX=uniform(0,numberOfRows);
        int cellIdY=uniform(0,numberOfColumns);
        for (int j=0;j<numberOfMembers[i];j++) {
            int hostId=groups[i][j];
            hosts[hostId-1].myCellIdX = cellIdX;
            hosts[hostId-1].myCellIdY = cellIdY;
            //increment the number of the hosts in that cell
            if (!shuffle){
                cells[cellIdX][cellIdY].numberOfHosts+=1;
                hosts[hostId-1].cellIdX=cellIdX;
                hosts[hostId-1].cellIdY=cellIdY;
                ev << "Setting home of " << hostId << " in position " 
                    << hosts[hostId-1].myCellIdX << " " << hosts[hostId-1].myCellIdY << std::endl;
                ev << "Setting pos  of " << hostId << " in position " 
                    << hosts[hostId-1].cellIdX << " " << hosts[hostId-1].cellIdY << std::endl;
            }
        }
    }
    //definition of the initial position of the hosts
    if (!shuffle)
        for (int k=0;k<numHosts;k++) {
        	Coord randomPoint = getRandomPointWithObstacles(
        			cells[hosts[k].cellIdX][hosts[k].cellIdY].minX,
        			cells[hosts[k].cellIdX][hosts[k].cellIdY].minY);
            hosts[k].currentX=randomPoint.x;
            hosts[k].currentY=randomPoint.y;
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

Coord MusolesiMobility::getRandomPointWithObstacles(int minX, int minY){
	Coord upLeft, downRight;
	upLeft.x = minX;
	upLeft.y = minY;
	upLeft.z = 0;
	downRight.x = minX+sideLengthX;
	downRight.y = minY+sideLengthY;
	downRight.z = 1;

	return  LineSegmentsMobilityBase::getRandomPositionWithObstacles(upLeft, downRight);
}

void MusolesiMobility::finish(){

    if(recordStatistics){
        for (int i=0; i<numHosts; i++)
        {
            if (i == nodeId)
                continue;
            if (nodesInMyBlock[i].first != 0){  
                intervalDistribution[(int)(10*(simTime().dbl()-nodesInMyBlock[i].first))]++;
            }
        }
    }
    if(nodeId == 0){
        for (int i = 0; i< numberOfRows; i++){
            free(cells[i]);
            free(CA[i]);
            free(a[i]);
        }
        free(cells);
        free(CA);
        free(a);
        int totalFreq = 0;
        double partialFreq = 0;
        if(recordStatistics){
        std::cout << "#CONTACTTIME" << std::endl;
            // print statistics
            for (std::map<int,int>::iterator ii = intervalDistribution.begin();
                    ii != intervalDistribution.end(); ii++){
                totalFreq += ii->second;
            }
            for (std::map<int,int>::iterator ii = intervalDistribution.begin();
                    ii != intervalDistribution.end(); ii++){
                partialFreq += ii->second;
                std::cout << ii->first << " " << ii->second << " " << 1 - partialFreq/totalFreq << std::endl;
            }
            totalFreq = 0;
            partialFreq = 0;
        std::cout << "#INTERCTIME" << std::endl;
            for (std::map<int,int>::iterator ii = interContactDistribution.begin();
                    ii != interContactDistribution.end(); ii++){
                totalFreq += ii->second;
            }
            for (std::map<int,int>::iterator ii = interContactDistribution.begin();
                    ii != interContactDistribution.end(); ii++){
                partialFreq += ii->second;
                std::cout << ii->first << " " << ii->second << " " << 1 - partialFreq/totalFreq << std::endl;
            }
        std::cout << "#ENDSTATISTICS" << std::endl;

        }
    }
    cancelAndDelete(moveMessage);
}


// global useful functions from Musolesi's code
//
int **initialise_int_array(int array_size) {
	
	int **result = new int * [array_size];
	
	for (int i = 0; i < array_size; i++){
		result[i] = new int [array_size];
        bzero(result[i],sizeof(int)*array_size);
    }
	return result;
}


double **initialise_double_array(int array_size)
{	double **result = new double * [array_size];
	
	for (int i = 0; i < array_size; i++){
		result[i] = new double [array_size];
        bzero(result[i],sizeof(int)*array_size);
    }

	return result;
}

void print_int_array(int **array, int array_size)
{	for (int i = 0; i < array_size; i++)
	{ ev << array[i][0];
	  for (int j = 1; j < array_size; j++)
		ev << ", " << array[i][j];
	  ev << "\n";
	}
	ev << "\n"; std::cout.flush();
}

void print_double_array(double **array, int array_size)
{	for (int i = 0; i < array_size; i++)
	{ printf("%.3f", array[i][0]);
	  for (int j = 1; j < array_size; j++)
		printf(", %.3f", array[i][j]);
	  std::cout << "\n";
	}
	std::cout << "\n"; std::cout.flush();
}

// in this rewiring I do not unlink the nodes from their current group, I just
// add links to other nodes in other groups. This way the network keeps being
// clustered but with more inter-cluster connections, else, the network becomes
// unclustered.

void rewire(double **weight, int array_size, double probRewiring, double threshold, int ** groups, int *numberOfMembers, int numberOfGroups)
{
    double scalefactor = 0;
    if (threshold < 0.5)  // little number that depends on the threshold
        scalefactor = threshold;
    else 
        scalefactor = (1-threshold);

    for (int i=0;i<array_size;i++)
        for (int j=0;j<array_size;j++) {
                if (areInTheSameGroup (i+1,j+1,groups,numberOfGroups,numberOfMembers)==true) { // chose a couple of nodes in the group
                    if (uniform(0,1)<probRewiring) {	// do rewiring
                        bool found=false;
                        for (int z=0;z<array_size;z++)
                            if ((areInTheSameGroup (i+1,z+1,groups,numberOfGroups,numberOfMembers)==false)
                                    &&(weight[i][z]<threshold)&&(found==false) && i!= z) {
                                weight[i][z]=weight[z][i]=1.0-uniform(0,1)*scalefactor; // give i,z a good score
                                found=true;
                            }
                    }
                }
        }  
}


//This class initialise the matrix creating a matrix composed of n disjointed groups 
//with nodes uniformely distributed, then performs rewiring for each link with
//prob probRewiring. threshold is the value under which the relationship 
//is not considered important
void refresh_weight_array_ingroups(double ** weight, int array_size, int numberOfGroups, double probRewiring, 
        double threshold, int ** groups, int* numberOfMembers) {


    // initially distribute users uniformly into groups
    // Added a randomic distribution over the beginning of the cycle
    // or else rewiring generates always the same groups

    for (int i=0;i<numberOfGroups;i++) {
        numberOfMembers[i]=0;
    }

    // this revised way introduces some more randomicity in the group selection
    // not really random, one beteewn the previous of node X and the next of
    // node X wil always be in the group of node X.

    int rnd = uniform(0,array_size);
    int groupSize = array_size/numberOfGroups;
    int groupReminder = array_size%numberOfGroups;
    for (int i = 0; i < numberOfGroups; i++){
        for (int j = i*groupSize; j < i*groupSize+groupSize; j++){
            groups[i][numberOfMembers[i]]=(j+rnd)%array_size + 1;
            numberOfMembers[i]+=1;	
        }
    } 
    if (groupReminder)
        for (int i = 0; i < groupReminder; i++){
            groups[i][numberOfMembers[i]]=(numberOfGroups*groupSize + i + rnd)%array_size + 1;
            numberOfMembers[i]+=1;	
        }
    
    for (int i=0;i<array_size;i++)
        for (int j=0;j<array_size;j++) 
            weight[i][j] = -1;

    // for each node in the group, rewire with probability probRewiring with
    // somebody in another group
    double scalefactor = 0;
    if (threshold < 0.5)  // little number that depends on the threshold
        scalefactor = threshold;
    else 
        scalefactor = (1-threshold);

    for (int i=0;i<array_size;i++)
        for (int j=0;j<array_size;j++) {
            if (weight[i][j] < 0){
                if (areInTheSameGroup (i+1,j+1,groups,numberOfGroups,numberOfMembers)==true) { // chose a couple of nodes in the group
                    if (uniform(0,1)<probRewiring) {	// do rewiring

                        bool found=false;
                        for (int z=0;z<array_size;z++)
                            if ((areInTheSameGroup (i+1,z+1,groups,numberOfGroups,numberOfMembers)==false)
                                    && (weight[i][z]<threshold) && (found==false) && i!= z) {
                                weight[i][z]= weight[z][i]=1.0-uniform(0,1)*scalefactor; // give i,z a good score
                                found=true;
                            }
                        weight[i][j]=weight[j][i]=(uniform(0,1)*scalefactor);  // give i,j a bad score
                    }
                    else {//no rewiring 
                        weight[i][j]=weight[j][i]=1.0-(uniform(0,1)*scalefactor); // give i,j a good score
                    }
                } else {//the hosts are not in the same cluster
                    weight[i][j]=weight[j][i]=(uniform(0,1)*scalefactor);  // give i,j a bad score
                }
            }
        } // if weight != -1 this number has already been assigned by previous iterations 
}


//generate the adjacency matrix from the weight matrix of size array_size given a certain threshold
void generate_adjacency (double** weightMat, int** adjacencyMat, double threshold, int array_size) {
	
	for (int i=0; i<array_size; i++)  
		for (int j=0; j<array_size; j++) {
			if (weightMat[i][j]>threshold)
				adjacencyMat[i][j]=1; 
			else
				adjacencyMat[i][j]=0;
		}
}


//Given the groups of nodes and two nodes "node1" and "node2" returns true if the hosts
//are in the same group, false otherwise
bool areInTheSameGroup (int node1, int node2, int** groups, int numberOfGroups, int* numberOfMembers) {
	
	bool result=false;
	for (int k=0;k<numberOfGroups;k++) {
		
		if (isInGroup(node1,groups[k],numberOfMembers[k]))
			if (isInGroup(node2,groups[k],numberOfMembers[k])) {
				result=true;
				//break;
			}
		}
	
	return result;
}

//Given a group of nodes and a node, stored in the array group of size equal
//to numberOfMembers, returns true if the node is in the group or false otherwise
bool isInGroup(int node, int* group, int numberOfMembers) {
	
	bool result=false;
	
	for (int k=0;k<numberOfMembers;k++)	
		if (group[k]==node) {
			result=true;
			//break;
		}
 	return result;
}
