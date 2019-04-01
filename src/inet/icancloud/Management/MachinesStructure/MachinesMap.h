/**
 *
 *
 * @class MachinesMap MachinesMap.cc MachinesMap.h
 *
 *  This class groups all the machines in a map. It supply operations to search, select, erase, turn on, off and many other
 *  operations over resources allocated at map.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2011-06-08
 */


#ifndef MACHINES_MAP_H_
#define MACHINES_MAP_H_

#include "inet/icancloud/Management/MachinesStructure/HeterogeneousSet.h"

namespace inet {

namespace icancloud {


class MachinesMap {

protected:

		/** The different virtual machines set **/
		vector<HeterogeneousSet*> machineMap;

public:

		/*
		 * Constructor
		 */
		MachinesMap();

		/*
		 * Destructor.
		 */
		virtual ~MachinesMap();

		/*
		 * This method allocate a new virtual machines set.
		 * This allows to the cloud manager to insert the VMsets ordered by
		 * some parameter.
		 * @param: the new set of virtual machines.
		 */
		void setInstances (HeterogeneousSet* newmachineSet);

		/*
		 * Return the position of a set with typename given as parameter as identifier
		 */
		int isSet(string typeName);

		/*
		 * This method set a new group of machines at map.
		 */
        void increaseInstances (vector<Machine*> machines, string typeName);

        /*
         * This method destroy an instance with identifier and type given as parameters from the map.
         */
        void destroyInstance (string type, int id);

		// ------------------ To obtain the features of a set --------------------------

        /*
         * This method returns the quantity of different machines allocated at map
         */
		int getMapQuantity ();
		/*
		 * This method returns the index of a machineSet
		 */
		int getSetIndex (string machineSetId);

		/*
		 * This method returns the name of the set positioned at index
		 */
		string getSetId (int index);

		/*
		 * These methods returns the quantity of machines that a machineSet has
		 */
		int getSetQuantity (string machineSetId);
		int getSetQuantity (int machineIndex);

		/*
		 * These methods returns the number of CPUs that all the machines of the set have.
		 */
		int getSetNumberOfCPUs(string machineSetId);
		int getSetNumberOfCPUs(int machineIndex);

		/*
		 * These methods returns the size of the memory that all the machines of the set have.
		 */
		int getSetTotalMemory(string machineSetId);
		int getSetTotalMemory(int machineIndex);

        /*
         * These methods returns the amount of storage that all the machines of the set have.
         */
		int getSetTotalStorage(int machineIndex);
		int getSetTotalStorage(string machineSetId);

		/*
		 * These methods returns the name (setIdentifier) of the machineSet
		 */

		string getSetIdentifier(int machineIndex);

		/*
		 * These methods returns the state of a machine into a machineSet
		 */
		bool isMachineON (string machineSetId, int machineIndex);
		bool isMachineON (int machineSetIndex, int machineIndex);

		/*
		 * These methods returns the number of machines in on state
		 */
		int countONMachines (string machineSetId);
		int countONMachines (int machineSetIndex);

		/*
         * These methods returns the number of machines in on state
         */
        int countOFFMachines (string machineSetId);
        int countOFFMachines (int machineSetIndex);

		// ------------------ To obtain machines from the set -------------------------

		/*
		 * These methods return the machine positioned at machineIndex into the requested Set.
		 */
		Machine* getMachineByIndex (string machineSetId, int machineIndex);
		Machine* getMachineByIndex (int machineSetIndex, int machineIndex);
		Machine* getMachineById(int id);

		/*
		 * These methods returns the number of machines at given state requested in the machineSet
		 */
		vector<Machine*> getOFFMachines (string machineSetId);
		vector<Machine*> getONMachines (string machineSetId);

		vector<Machine*> getOFFMachines (int machineSetIndex);
        vector<Machine*> getONMachines (int machineSetIndex);

		/*
		 * This method returns the number of elements of the Map (each machine set)
		 */
		unsigned int size ();


		/*
		 * This method returns the machine with the same ip that the given as parameter.
		 * If the machine is not in the map structure, the method will return nullptr
		 */
		Machine* getMachineByIP (string ip);


        /*
         * Getter and setter for element type
         */
		elementType* getElementType (int index){return machineMap[index]->getElementType();};
		void setElementType (int index, elementType* element);


};

} // namespace icancloud
} // namespace inet

#endif /* MACHINESINSTANCESLIST_H_ */
