/**
 * @class  elementType.cc elementType.h
 *
 * This class define the resources of an execution element unit (Machine) at data center system.
 * It has getters and setters for the devices.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2014-22-06
 */

#ifndef ELEMENTTYPE_H_
#define ELEMENTTYPE_H_

#include <string>

namespace inet {

namespace icancloud {

using namespace std;

class elementType {
    string name;			    							// node Name
	int memoryTotal;										// Size of memory
	int storageTotal;										// Size of memory
	int numCores;											// Number of CPUs
	int numStorageDevices;									// Number of Storage Servers
	int numNetIF;											// Number of network interfaces

public:
	elementType();
	virtual ~elementType();

	void setTypeParameters(int numTotalCores, int memorySize, int storageSize, int numIfEth, string newNodeType, int numStorageDev = 1);

	int getNumCores(){return numCores;};
	int getMemorySize(){return memoryTotal;};
	int getStorageSize(){return storageTotal;};
	int getNumStorageDevices(){return numStorageDevices;};
	int getNumNetIF (){return numNetIF;};
	string getType(){return name.c_str();};

	void setNumCores (int numCPU){numCores = numCPU;};
	void setMemorySize (int memorySize){memoryTotal = memorySize;};
	void setDiskSize (int diskSize){storageTotal = diskSize;};
	void setNumStorageDevices (int numStorageDev){numStorageDevices = numStorageDev;};
	void setNumNetIF (int numNicIF){numNetIF = numNicIF;};
	void setType(string id){name = id;};

	elementType* getDCElementType (){return this;};
};

} // namespace icancloud
} // namespace inet

#endif /* DCELEMENTTYPE_H_ */
