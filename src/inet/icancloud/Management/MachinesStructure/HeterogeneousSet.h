/**
 * @class HeterogeneousNodeSet HeterogeneousNodeSet.cc HeterogeneousNodeSet.h
 *
 *  This class group a set of heterogeneous resources with same element type.
 *
 * @author: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2014-06-06
 */

#ifndef HETEROGENEOUS_SET_H_
#define HETEROGENEOUS_SET_H_

#include "inet/icancloud/Architecture/Machine/Machine.h"

namespace inet {

namespace icancloud {


class HeterogeneousSet{

	protected:

		vector<Machine*> machinesSet;
		elementType* type;

	public:

		/*
		 * Constructor.
		 */
		HeterogeneousSet();

		/*
		 * This method sets the type
		 */
		void setType (elementType* newType){type = newType;};

        /*
         * Destructor.
         */
		virtual ~HeterogeneousSet();

	   /**
		* Module initialization. It sets the machine at machines set
		*/
		void initMachine (Machine* node);

		/*
		 * The method returns the number of CPUs that the machine type has.
		 */

		int getNumberOfCPUs(){return type->getNumCores();};

		/*
		 * The method returns the total size of memory that the machine type has.
		 */

		int getTotalMemory(){return type->getMemorySize();};

        /*
         * The method returns the total amount of storage size that the machine type has.
         */
        int getStorageSize(){return type->getStorageSize();};

		/*
		 * The method returns the name of the machine set.
		 */

		string getSetIdentifier(){return type->getType();};

	    /*
         * The method returns the state of the machine at position index
         */

        bool isON (int index);

        /*
         * The method returns the number of machines in ON state.
         */

        int countON ();

        /*
         * The method returns the number of machines in ON state.
         */

        int countOFF ();

        /*
         * Returns the size of the virtual machines set vector.
         */

        unsigned int size ();

		/*
		 * The method returns the machine at position in the machineSet vector = index
		 */

		Machine* getMachineByIndex (int index);

		/*
		 * The method returns all the machines in state = OFF to the parameter state
		 */

		vector<Machine*> getOFFMachines ();

        /*
         * The method returns all the machines in state = STATE_IDLE || STATE_RUNNING to the parameter state
         */

        vector<Machine*> getONMachines ();

        /*
         * The method returns the DCElementtype
         */

        elementType* getElementType (){return type;};

        void setElementType(elementType* element) {type = element;};
        /*
         * This method delete the machine ONLY from the structure!
         */
        void deleteMachine(int pId);

};

} // namespace icancloud
} // namespace inet

#endif /* HETEROGENEOUS_SET_H_ */
