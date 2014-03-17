#ifndef BASECONNECTIONMANAGER_H_
#define BASECONNECTIONMANAGER_H_

#include <map>
#include <vector>
#include <string>

#include "INETDefs.h"
#include "NicEntry.h"
#include "RadioChannelBase.h"

class DetailedRadioChannelAccess;

/**
 * @brief Module to control the channel and handle all connection
 * related stuff
 *
 * The central module that coordinates the connections between all
 * nodes, and handles dynamic gate creation. BaseConnectionManager therefore
 * periodically communicates with the ConnectionManagerAccess modules
 *
 * You may not instantiate BaseConnectionManager!
 * Use ConnectionManager instead.
 *
 * @ingroup connectionManager
 * @author Steffen Sroka, Daniel Willkomm, Karl Wessel
 * @author Christoph Sommer ("unregisterNic()"-method)
 * @sa ConnectionManagerAccess
 */
class INET_API BaseConnectionManager : public RadioChannelBase
{
private:
	/**
	 * @brief Represents a position inside a grid.
	 *
	 * Internal helper class of BaseConnectionManager.
	 * This class provides some converting functions from a Coord
	 * to a GridCoord.
	 */
	class GridCoord
	{
	public:
		/** @name Coordinates in the grid.*/
		/*@{*/
		int x;
		int y;
		int z;
		/*@}*/

	public:
		/**
		 * @brief Initialize this GridCoord with the origin.
		 * Creates a 3-dimensional coord.
		 */
		GridCoord()
			:x(0), y(0), z(0) {};

		/**
		 * @brief Initialize a 2-dimensional GridCoord with x and y.
		 */
		GridCoord(int x, int y)
			:x(x), y(y), z(0) {};

		/**
		 * @brief Initialize a 3-dimensional GridCoord with x, y and z.
		 */
		GridCoord(int x, int y, int z)
			:x(x), y(y), z(z) {};

		/**
		 * @brief Simple copy-constructor.
		 */
		GridCoord(const GridCoord& o)
			:x(o.x), y(o.y), z(o.z) {};

		/**
		 * @brief Creates a GridCoord from a given Coord by dividing the
		 * x,y and z-values by "gridCellWidth".
		 * The dimension of the GridCoord depends on the Coord.
		 */
		GridCoord(const Coord& c, const Coord& gridCellSize = Coord(1.0,1.0,1.0))
			: x( static_cast<int>(c.x / gridCellSize.x) )
			, y( static_cast<int>(c.y / gridCellSize.y) )
			, z( static_cast<int>(c.z / gridCellSize.z) )
		{}

		/** @brief Output string for this coordinate.*/
		std::string info() const {
			std::stringstream os;
			os << "(" << x << "," << y << "," << z << ")";
			return os.str();
		}

		/** @brief Comparison operator for coordinates.*/
		friend bool operator==(const GridCoord& a, const GridCoord& b) {
			return a.x == b.x && a.y == b.y && a.z == b.z;
		}

		/** @brief Comparison operator for coordinates.*/
		friend bool operator!=(const GridCoord& a, const GridCoord& b) {
			return !(a==b);
		}
	};

	/**
	 * @brief Represents an minimalistic (hash)set of GridCoords.
	 *
	 * It is a workaround because c++ doesn't come with an hash set.
	 */
	class CoordSet {
	protected:
		/** @brief Holds the hash table.*/
		std::vector<GridCoord*> data;
		/** @brief maximum size of the hash table.*/
		unsigned maxSize;
		/** @brief Current number of entries in the hash table.*/
		unsigned size;
		/** @brief Holds the current element when iterating over this table.*/
		unsigned current;

	protected:

		/**
		 * @brief Tries to insert a GridCoord at the specified position.
		 *
		 * If the same Coord already exists there nothing happens.
		 * If an other Coord already exists there calculate
		 * a new Position to insert end recursively call this Method again.
		 * If the spot is empty the Coord is inserted.
		 */
		void insert(const GridCoord& c, unsigned pos) {
			if(data[pos] == 0) {
				data[pos] = new GridCoord(c);
				size++;
			} else {
				if(*data[pos] != c) {
					insert(c, (pos + 2) % maxSize);
				}
			}
		}

	public:
		/**
		 * @brief Initializes the set (hashtable) with the a specified size.
		 */
		CoordSet(unsigned sz)
			:data(), maxSize(sz), size(0), current(0)
		{
			data.resize(maxSize);
		}

		/**
		 * @brief Delete every created GridCoord
		 */
		~CoordSet() {
			for(std::vector<GridCoord*>::const_iterator it = data.begin(); it != data.end(); ++it) {
				if(*it) {
					delete *it;
				}
			}
		}

		/**
		 * @brief Adds a GridCoord to the set.
		 * If a GridCoord with the same value already exists in the set
		 * nothing happens.
		 */
		void add(const GridCoord& c) {
			unsigned hash = (c.x * 10000 + c.y * 100 + c.z) % maxSize;
			insert(c, hash);
		}

		/**
		 * @brief Returns the next GridCoord in the set.
		 * You can iterate through the set only one time with this function!
		 */
		GridCoord* next() {
			for(;current < maxSize; current++) {
				if(data[current] != 0) {
					return data[current++];
				}
			}
			return 0;
		}

		/**
		 * @brief Returns the number of GridCoords currently saved in this set.
		 */
		unsigned getSize() const { return size; }

		/**
		 * @brief Returns the maximum number of elements which can be stored inside
		 * this set.
		 * To prevent collisions the set should never be more than 75% filled.
		 */
		unsigned getmaxSize() const { return maxSize; }
	};

protected:
	/** @brief Type for map from nic-module id to nic-module pointer.*/
	typedef std::map<NicEntry::t_nicid, NicEntry*> NicEntries;

	/** @brief Map from nic-module ids to nic-module pointers.*/
	NicEntries nics;

	/** @brief Does the ConnectionManager use sendDirect or not?*/
	bool sendDirect;

	/** @brief Stores the size of the playground.*/
	Coord playgroundSize;

	/** @brief the biggest interference distance in the network.*/
	double maxInterferenceDistance;

	/** @brief Square of maxInterferenceDistance cache a value that
	 * is often used */
	double maxDistSquared;

	/** @brief Stores the useTorus flag of the WorldUtility */
	bool useTorus;

	/** @brief Stores if maximum interference distance should be displayed in
	 * TkEnv.*/
	bool drawMIR;

	/** @brief Type for 1-dimensional array of NicEntries.*/
	typedef std::vector<NicEntries> RowVector;
	/** @brief Type for 2-dimensional array of NicEntries.*/
	typedef std::vector<RowVector> NicMatrix;
	/** @brief Type for 3-dimensional array of NicEntries.*/
    typedef std::vector<NicMatrix> NicCube;

	/**
	 * @brief Register of all nics
     *
     * This matrix keeps all nics according to their position.  It
     * allows to restrict the position update to a subset of all nics.
     */
    NicCube nicGrid;

    /**
     * @brief Distance that helps to find a node under a certain
     * position.
     *
     * Can be larger then @see maxInterferenceDistance to
     * allow nodes to be placed into the same square if the playground
     * is too small for the grid speedup to work.
	 */
    Coord findDistance;

    /** @brief The size of the grid */
    GridCoord gridDim;

private:
	/** @brief Manages the connections of a registered nic. */
    void updateNicConnections(NicEntries& nmap, NicEntries::mapped_type nic);

    /**
     * @brief Check connections of a nic in the grid
     */
    void checkGrid(GridCoord&           oldCell,
                   GridCoord&           newCell,
                   NicEntry::t_nicid_cref id);

    /**
     * @brief Calculates the corresponding cell of a coordinate.
     */
    GridCoord getCellForCoordinate(const Coord& c) const;

    /**
     * @brief Returns the NicEntries of the cell with specified
     * coordinate.
     */
    NicEntries& getCellEntries(const GridCoord& cell);

	/**
	 * If the value is outside of its bounds (zero and max) this function
	 * returns -1 if useTorus is false and the wrapped value if useTorus is true.
	 * Otherwise its just returns the value unchanged.
	 */
    int wrapIfTorus(int value, int max) const;

	/**
	 * @brief Adds every direct Neighbor of a GridCoord to a union of coords.
	 */
    void fillUnionWithNeighbors(CoordSet& gridUnion, const GridCoord& cell) const;
protected:

	/**
	 * @brief Calculate interference distance
	 *
	 * Called by BaseConnectionManager already during initialization stage 0.
	 * Implementations therefore have to make sure that everything necessary
	 * for calculation is either already initialized or has to be initialized in
	 * this method!
	 *
	 * This method has to be overridden by any derived class.
	 */
	virtual double calcInterfDist() = 0;

	/**
	 * @brief Called by "registerNic()" after the nic has been
	 * unregistered. That means that the NicEntry for the nic has already been
	 * disconnected and removed from nic map.
	 *
	 * You better know what you are doing if you want to override this
	 * method. Most time you won't need to.
	 *
	 * See BaseConnectionManager::registerNicExt() for an example.
	 *
	 * @param nicID - the id of the NicEntry
	 */
	virtual void registerNicExt(NicEntry::t_nicid_cref nicID);

	/**
	 * @brief Called by "unregisterNic()" after the nic has been
	 * registered. That means that the NicEntry for the nic has already been
	 * created and added to nics map.
	 *
	 * You better know what you are doing if you want to override this
	 * method. Most time you won't need to.
	 *
	 * See BaseConnectionManager::unregisterNicExt() for an example.
	 *
	 * @param nicID - the id of the NicEntry
	 */
	virtual void unregisterNicExt(NicEntry::t_nicid_cref nicID);

	/**
	 * @brief Updates the connections of the nic with "nicID".
	 *
	 * This method is called by "updateNicPos()" after the
	 * new Position is stored in the corresponding nic.
	 *
	 * Most time you won't need to override this method.
	 *
	 * @param nicID the id of the NicEntry
	 * @param oldPos the old position of the nic
	 * @param newPos the new position of the nic
	 */
	virtual void updateConnections(NicEntry::t_nicid_cref nicID, const Coord* oldPos, const Coord* newPos);
	/**
	 * @brief Check if the two nic's are in range.
	 *
	 * This function will be used to decide if two nic's shall be connected or not. It
	 * is simple to overload this function to enhance the decision for connection or not.
	 *
	 * @param pFromNic Nic source point which should be checked.
	 * @param pToNic   Nic target point which should be checked.
	 * @return true if the nic's are in range and can be connected, false if not.
	 */
	virtual bool isInRange(NicEntries::mapped_type pFromNic, NicEntries::mapped_type pToNic);

private:
	/** @brief Copy constructor is not allowed.
	 */
	BaseConnectionManager(const BaseConnectionManager&);
	/** @brief Assignment operator is not allowed.
	 */
	BaseConnectionManager& operator=(const BaseConnectionManager&);

public:
	BaseConnectionManager();
	virtual ~BaseConnectionManager();

	/** @brief Needs two initialization stages.*/
	virtual int numInitStages() const { return NUM_INIT_STAGES; }

	/**
	 * @brief Reads init parameters and calculates a maximal interference
	 * distance
	 **/
	virtual void initialize(int stage);

	bool getUseTorus() { return useTorus; }

	Coord getPgs() { return playgroundSize; }

	/**
	 * @brief Registers a nic to have its connections managed by ConnectionManager.
	 *
	 * If you want to do your own stuff at the registration of a nic see
	 * "registerNicExt()".
	 */
    bool registerNic(cModule* nic, DetailedRadioChannelAccess* chAccess, const Coord* nicPos);

	/**
	 * @brief Unregisters a NIC such that its connections aren't managed by the CM
	 * anymore.
	 *
	 * NOTE: This method asserts that the passed NIC module was previously registered
	 * with this ConnectionManager!
	 *
	 * This method should be used for dynamic networks were hosts can actually disappear.
	 *
	 * @param nic the NIC module to be unregistered
	 * @return returns true if the NIC was unregistered successfully
	 */
	bool unregisterNic(cModule* nic);

	/** @brief Updates the position information of a registered nic.*/
	void updateNicPos(NicEntry::t_nicid_cref nicID, const Coord* newPos);

	/** @brief Returns the ingates of all nics in range*/
	const NicEntry::GateList& getGateList(NicEntry::t_nicid_cref nicID) const;

	/** @brief Returns the ingate of the with id==targetID, or 0 if not in range*/
	const cGate* getOutGateTo(const NicEntry* nic, const NicEntry* targetNic) const;
};

#endif /*BASECONNECTIONMANAGER_H_*/
