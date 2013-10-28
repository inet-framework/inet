#include "BaseConnectionManager.h"

#include <cassert>

#include "NicEntryDebug.h"
#include "NicEntryDirect.h"
#include "FindModule.h"

static const NicEntryDirect cEmptyNicDirect;
static const NicEntryDebug  cEmptyNicDebug;

BaseConnectionManager::BaseConnectionManager()
  : RadioChannelBase()
  , nics()
  , sendDirect(false)
  , playgroundSize()
  , maxInterferenceDistance(0.0)
  , maxDistSquared(0.0)
  , useTorus(false)
  , drawMIR(false)
  , nicGrid()
  , findDistance()
  , gridDim()
{}

void BaseConnectionManager::initialize(int stage)
{
	RadioChannelBase::initialize(stage);
	if (stage == 0)
	{
		drawMIR = hasPar("drawMaxIntfDist")
						? par("drawMaxIntfDist").boolValue() : false;

		EV_TRACE << "initializing BaseConnectionManager\n";

	    playgroundSize = Coord(par("playgroundSizeX").doubleValue(),
                               par("playgroundSizeY").doubleValue(),
                               par("playgroundSizeZ").doubleValue());
		useTorus = par("useTorus");

		if(hasPar("sendDirect"))
			sendDirect = par("sendDirect").boolValue();
		else
			sendDirect = false;

		maxInterferenceDistance = calcInterfDist();
		maxDistSquared = maxInterferenceDistance * maxInterferenceDistance;

		//----initialize node grid-----
		//step 1 - calculate dimension of grid
		//one cell should have at least the size of maxInterferenceDistance
		//but also should divide the playground in equal parts
		Coord dim(playgroundSize / maxInterferenceDistance);
		gridDim = GridCoord(dim);

		//A grid smaller or equal to 3x3 would mean that every cell has every
		//other cell as direct neighbor (if our playground is a torus, even if
		//not the most of the cells are direct neighbors of each other. So we
		//reduce the grid size to 1x1.
		if((gridDim.x <= 3) && (gridDim.y <= 3) && (gridDim.z <= 3))
		{
			gridDim.x = 1;
			gridDim.y = 1;
			gridDim.z = 1;
		} else {
			gridDim.x = std::max(1, gridDim.x);
			gridDim.y = std::max(1, gridDim.y);
			gridDim.z = std::max(1, gridDim.z);
		}

		//step 2 - initialize the matrix which represents our grid
		NicEntries entries;
		RowVector row;
		NicMatrix matrix;

		for (int i = 0; i < gridDim.z; ++i) {
			row.push_back(entries);			//copy empty NicEntries to RowVector
		}
		for (int i = 0; i < gridDim.y; ++i) {//fill the ColVector with copies of
			matrix.push_back(row);			 //the RowVector.
		}
		for (int i = 0; i < gridDim.x; ++i) {	//fill the grid with copies of
			nicGrid.push_back(matrix);			//the matrix.
		}
		EV_INFO << " using " << gridDim.x << "x" <<
							 gridDim.y << "x" <<
							 gridDim.z << " grid" << endl;

		//step 3 -	calculate the factor which maps the coordinate of a node
		//			to the grid cell
		//if we use a 1x1 grid every coordinate is mapped to (0,0, 0)
		findDistance = Coord(std::max(playgroundSize.x, maxInterferenceDistance),
							 std::max(playgroundSize.y, maxInterferenceDistance),
							 std::max(playgroundSize.z, maxInterferenceDistance));
		//otherwise we divide the playground into cells of size of the maximum
		//interference distance
		if (gridDim.x != 1)
			findDistance.x = playgroundSize.x / gridDim.x;
		if (gridDim.y != 1)
			findDistance.y = playgroundSize.y / gridDim.y;
		if (gridDim.z != 1)
			findDistance.z = playgroundSize.z / gridDim.z;

		//since the upper playground borders (at pg-size) are part of the
		//playground we have to assure that they are mapped to a valid
		//(the last) grid cell we do this by increasing the find distance
		//by a small value.
		//This also assures that findDistance is never zero.
		findDistance += Coord(EPSILON, EPSILON, EPSILON);

		//findDistance (equals cell size) has to be greater or equal
		//maxInt-distance
		assert(findDistance.x >= maxInterferenceDistance);
		assert(findDistance.y >= maxInterferenceDistance);
		assert(findDistance.z >= maxInterferenceDistance);

		//playGroundSize has to be part of the playGround
		assert(GridCoord(playgroundSize, findDistance).x == gridDim.x - 1);
		assert(GridCoord(playgroundSize, findDistance).y == gridDim.y - 1);
		assert(GridCoord(playgroundSize, findDistance).z == gridDim.z - 1);
		EV_INFO << "findDistance is " << findDistance.info() << endl;
	}
	else if (stage == 1)
	{

	}
}

BaseConnectionManager::GridCoord BaseConnectionManager
	::getCellForCoordinate(const Coord& c) const
{
    return GridCoord(c, findDistance);
}

void BaseConnectionManager::updateConnections(NicEntry::t_nicid_cref nicID,
                                              const Coord* oldPos,
                                              const Coord* newPos)
{
	GridCoord oldCell = getCellForCoordinate(*oldPos);
    GridCoord newCell = getCellForCoordinate(*newPos);

	checkGrid(oldCell, newCell, nicID );
}

BaseConnectionManager::NicEntries& BaseConnectionManager
	::getCellEntries(const BaseConnectionManager::GridCoord& cell)
{
    return nicGrid[cell.x][cell.y][cell.z];
}

void BaseConnectionManager::registerNicExt(NicEntry::t_nicid_cref nicID)
{
	NicEntries::mapped_type nicEntry = nics[nicID];

	GridCoord cell = getCellForCoordinate(nicEntry->pos);

	EV_DEBUG <<" registering (ext) nic at loc " << cell.info() << std::endl;

	// add to matrix
	NicEntries& cellEntries = getCellEntries(cell);
    cellEntries[nicID] = nicEntry;
}

void BaseConnectionManager::checkGrid(BaseConnectionManager::GridCoord& oldCell,
                                      BaseConnectionManager::GridCoord& newCell,
                                      NicEntry::t_nicid_cref            id)

{

    // structure to find union of grid squares
    CoordSet gridUnion(74);

    // find nic at old position
    NicEntries&             oldCellEntries = getCellEntries(oldCell);
    NicEntries::iterator    it  = oldCellEntries.find(id);
    NicEntries::mapped_type nic = it->second;


    // move nic to a new position in matrix
    if(oldCell != newCell) {
    	oldCellEntries.erase(it);
    	getCellEntries(newCell)[id] = nic;
    }

	if((gridDim.x == 1) && (gridDim.y == 1) && (gridDim.z == 1)) {
		gridUnion.add(oldCell);
    } else {
		//add grid around oldPos
		fillUnionWithNeighbors(gridUnion, oldCell);


        if(oldCell != newCell) {
            //add grid around newPos
            fillUnionWithNeighbors(gridUnion, newCell);
        }
    }

    GridCoord* c = gridUnion.next();
    while(c != 0) {
		EV_INFO << "Update cons in [" << c->info() << "]" << endl;
		updateNicConnections(getCellEntries(*c), nic);
		c = gridUnion.next();
    }
}

int BaseConnectionManager::wrapIfTorus(int value, int max) const {
	if(value < 0) {
		if(useTorus) {
			return max + value;
		} else {
			return -1;
		}
	} else if(value >= max) {
		if(useTorus) {
			return value - max;
		} else {
			return -1;
		}
	} else {
		return value;
	}
}

void BaseConnectionManager::fillUnionWithNeighbors(CoordSet&        gridUnion,
                                                   const GridCoord& cell) const
{
	for(int iz = (int)cell.z - 1; iz <= (int)cell.z + 1; iz++) {
		int cz = wrapIfTorus(iz, gridDim.z);
		if(cz == -1) {
			continue;
		}
		for(int ix = (int)cell.x - 1; ix <= (int)cell.x + 1; ix++) {
			int cx = wrapIfTorus(ix, gridDim.x);
			if(cx == -1) {
				continue;
			}
			for(int iy = (int)cell.y - 1; iy <= (int)cell.y + 1; iy++) {
				int cy = wrapIfTorus(iy, gridDim.y);
				if(cy != -1) {
					gridUnion.add(GridCoord(cx, cy, cz));
				}
			}
		}
	}
}

bool BaseConnectionManager::isInRange(BaseConnectionManager::NicEntries::mapped_type pFromNic, BaseConnectionManager::NicEntries::mapped_type pToNic)
{
	double dDistance = 0.0;

    if(useTorus) {
    	dDistance = pFromNic->pos.sqrTorusDist(pToNic->pos, playgroundSize);
    } else {
    	dDistance = pFromNic->pos.sqrdist(pToNic->pos);
    }
    return (dDistance <= maxDistSquared);
}

void BaseConnectionManager::updateNicConnections(NicEntries& nmap, BaseConnectionManager::NicEntries::mapped_type   nic)
{
    NicEntry::t_nicid_cref id = nic->nicId;

    for(NicEntries::iterator i = nmap.begin(); i != nmap.end(); ++i) {
    	NicEntries::mapped_type nic_i = i->second;

        // no recursive connections
        if ( nic_i->nicId == id ) continue;

        bool inRange   = isInRange(nic, nic_i);
        bool connected = nic->isConnected(nic_i);

        if ( inRange && !connected ) {
            // nodes within communication range: connect
            // nodes within communication range && not yet connected
            EV_INFO << "nic #" << id << " and #" << nic_i->nicId
            	 << " are in range" << endl;
            nic->connectTo( nic_i );
            nic_i->connectTo( nic );
        }
        else if ( !inRange && connected ) {
            // out of range: disconnect
            // out of range, and still connected
            EV_INFO << "nic #" << id << " and #" << nic_i->nicId
            	 << " are NOT in range" << endl;
            nic->disconnectFrom( nic_i );
            nic_i->disconnectFrom( nic );
        }
    }
}

bool BaseConnectionManager::registerNic(cModule*                 nic,
                                        DetailedRadioChannelAccess* chAccess,
                                        const Coord*             nicPos)
{
	assert(nic != NULL);

	const NicEntry::t_nicid nicID = nic->getId();
	EV_INFO << " registering nic #" << nicID << endl;

	// create new NicEntry
	NicEntries::mapped_type nicEntry;
	cModule *const          pHostModule = FindModule<>::findHost(nic);

	if(sendDirect)
		nicEntry = new NicEntryDirect();
	else
		nicEntry = new NicEntryDebug();

	// fill nicEntry
	nicEntry->nicPtr = nic;
	nicEntry->nicId = nicID;
	nicEntry->hostId   = pHostModule ? pHostModule->getId() : 0;
	nicEntry->pos      = *nicPos;
	nicEntry->chAccess = chAccess;

	// add to map
	nics[nicID] = nicEntry;

	registerNicExt(nicID);

	updateConnections(nicID, nicPos, nicPos);

	if(drawMIR && pHostModule) {
		cDisplayString& Displ = pHostModule->getDisplayString();
		if (!Displ.containsTag("r"))
			Displ.setTagArg("r", 0, maxInterferenceDistance);
	}

	return sendDirect;
}

bool BaseConnectionManager::unregisterNic(cModule* nicModule)
{
	assert(nicModule != 0);

	// find nicEntry
	const NicEntry::t_nicid nicID      = nicModule->getId();
	NicEntries::iterator    nicEntryIt = nics.find(nicID);
	EV_INFO << " unregistering nic #" << nicID << endl;

	if (nicEntryIt == nics.end()) {
		assert(false);
		return false;
	}

	NicEntries::mapped_type nicEntry = nicEntryIt->second;

	// get all affected grid squares
	CoordSet gridUnion(74);
	GridCoord cell = getCellForCoordinate(nicEntry->pos);
	if((gridDim.x == 1) && (gridDim.y == 1) && (gridDim.z == 1)) {
		gridUnion.add(cell);
	} else {
		fillUnionWithNeighbors(gridUnion, cell);
	}

	// disconnect from all NICs in these grid squares
	GridCoord* c = gridUnion.next();
	while(c != 0) {
		EV_INFO << "Update cons in [" << c->info() << "]" << endl;
		NicEntries& nmap = getCellEntries(*c);
		for(NicEntries::iterator i = nmap.begin(); i != nmap.end(); ++i) {
			NicEntries::mapped_type other = i->second;
			if (other == nicEntry)
				continue;
			if (other->isConnected(nicEntry)) {
				other->disconnectFrom(nicEntry);
			}
			if (nicEntry->isConnected(other)) {
				nicEntry->disconnectFrom(other);
			}
		}
		c = gridUnion.next();
	}

	// erase from grid
	NicEntries& cellEntries = getCellEntries(cell);
	cellEntries.erase(nicID);

	unregisterNicExt(nicID);

	// erase from list of known nics
	nics.erase(nicEntryIt);
	delete nicEntry;

	return true;
}

void BaseConnectionManager::unregisterNicExt(NicEntry::t_nicid_cref nicID)
{
	(void)nicID;
}

void BaseConnectionManager::updateNicPos(NicEntry::t_nicid_cref nicID, const Coord* newPos)
{
	NicEntries::iterator ItNic = nics.find(nicID);
	if (ItNic == nics.end()) {
		opp_warning("No nic with this ID (%d) is registered with this ConnectionManager, no position update done.", nicID);
		return;
	}
	Coord oldPos = ItNic->second->pos;
	ItNic->second->pos = *newPos;

	updateConnections(nicID, &oldPos, newPos);
}

const NicEntry::GateList& BaseConnectionManager::getGateList(NicEntry::t_nicid_cref nicID) const
{
	NicEntries::const_iterator ItNic = nics.find(nicID);
	if (ItNic == nics.end()) {
		opp_warning("No nic with this ID (%d) is registered with this ConnectionManager, return empty GateList", nicID);
		if(sendDirect)
			return cEmptyNicDirect.getGateList();
		return cEmptyNicDebug.getGateList();
	}

	return ItNic->second->getGateList();
}

const cGate* BaseConnectionManager::getOutGateTo(const NicEntry* nic,
                                                 const NicEntry* targetNic) const
{
	NicEntries::const_iterator ItNic = nics.find(nic->nicId);
	if (ItNic == nics.end())
		error("No nic with this ID (%d) is registered with this ConnectionManager.", nic->nicId);

	return ItNic->second->getOutGateTo(targetNic);
}

BaseConnectionManager::~BaseConnectionManager()
{
	for (NicEntries::iterator ne = nics.begin(); ne != nics.end(); ++ne) {
		delete ne->second;
	}
}
