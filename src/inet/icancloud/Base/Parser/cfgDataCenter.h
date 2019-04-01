/*
 * @class CfgDataCenter CfgDataCenter.h "CfgDataCenter.h"
 *
 * Module to contains all computing and storage nodes that composes the underlying architecture of a data center
 *
 * @author Gabriel Gonzalez Castane
 * @date 2013-05-04
 */


#ifndef CFGDC_H_
#define CFGDC_H_


#include <omnetpp.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "inet/icancloud/Base/include/icancloud_types.h"

namespace inet {

namespace icancloud {


using std::string;
using std::vector;

class CfgDataCenter {

protected:

	// Node Instances structure
		struct nodeStructure{
			string nodeType;
			int quantity;
		};

		vector <nodeStructure*> nodes;

		vector <nodeStructure*> st_nodes;

public:
	CfgDataCenter();
	virtual ~CfgDataCenter();

	virtual void setNodeType (string nodeType, int quantity);
	virtual void setStorageNodeType (string nodeType, int quantity);

	// To obtain the size of the main structures
	int getNumberOfNodeTypes(){return nodes.size();};

	// To obtain the parameters of a Node structure
	string getNodeType (int index);
	int getNodeQuantity (int index);

	// To obtain the parameters of a Node structure
	int getNumberOfStorageNodeTypes(){return st_nodes.size();};
	string getNodeStorageType (int index);
	int getNodeStorageQuantity (int index);

	// To obtain all structures if the nodes are grouped by structures.
	vector<string> generateStructureOfNodes (int index);
    vector<string> generateStructureOfStorageNodes (int index);

protected:

	void printNodes();
	void printVms();

};

} // namespace icancloud
} // namespace inet

#endif /* CFGCLOUD_H_ */
