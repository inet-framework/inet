/*
 * cfgCloud.cpp
 *
 *  Created on: May 15, 2013
 *      Author: gabriel
 */

#include "inet/icancloud/Base/Parser/cfgDataCenter.h"

namespace inet {

namespace icancloud {


using namespace omnetpp;

CfgDataCenter::CfgDataCenter() {
	// TODO Auto-generated constructor stub
	nodes.clear();
	st_nodes.clear();
}

CfgDataCenter::~CfgDataCenter() {
    nodes.clear();
    st_nodes.clear();
}


void CfgDataCenter::setNodeType (string nodeType, int quantity){

    nodeStructure* node =new nodeStructure();
    node->nodeType = nodeType;
    node->quantity = quantity;

    nodes.push_back(node);
}

void CfgDataCenter::setStorageNodeType (string nodeType, int quantity){

    nodeStructure* n =new nodeStructure();
    n->nodeType = nodeType;
    n->quantity = quantity;

    st_nodes.push_back(n);
}


string CfgDataCenter::getNodeType (int index){

	vector <nodeStructure*>::iterator nodeIt;

	nodeIt = nodes.begin() + index;

	return (*nodeIt)->nodeType.c_str();

}

int CfgDataCenter::getNodeQuantity (int index){

	vector <nodeStructure*>::iterator nodeIt;

	nodeIt = nodes.begin() + index;

	return (*nodeIt)->quantity;

}


string CfgDataCenter::getNodeStorageType (int index){
	vector <nodeStructure*>::iterator nodeIt;

	nodeIt = st_nodes.begin() + index;

	return (*nodeIt)->nodeType.c_str();
}

int CfgDataCenter::getNodeStorageQuantity (int index){
	vector <nodeStructure*>::iterator nodeIt;

		nodeIt = st_nodes.begin() + index;

		return (*nodeIt)->quantity;
}

vector<string> CfgDataCenter::generateStructureOfStorageNodes  (int index){
    nodeStructure* structure;
    vector<string> names;
    int j;
    std::ostringstream str;

    names.clear();

    structure = (*(st_nodes.begin()+index));
    for (j = 0; j < structure->quantity; j++){
        str.str("");
        str << structure->nodeType.c_str() << "[" << j << "]";
        names.push_back(str.str());
    }

    return names;

}

vector<string> CfgDataCenter::generateStructureOfNodes (int index){
    nodeStructure* structure;
    vector<string> names;
    int j;
    std::ostringstream str;

    names.clear();

    structure = (*(nodes.begin()+index));
    for (j = 0; j < structure->quantity; j++){
        str.str("");
        str << structure->nodeType.c_str() << "[" << j << "]";
        names.push_back(str.str());
    }

    return names;

}
// ---------------------- Private methods ----------------

void CfgDataCenter::printNodes(){

	vector<nodeStructure*>::iterator nodeIt;
	printf ("------------------- SET OF NODES ---------------------\n");

	for (nodeIt = nodes.begin(); nodeIt < nodes.end(); nodeIt++){
		printf ("Node Type:\t %s\n", (*nodeIt) ->nodeType.c_str());
		printf ("Quantity of nodes:\t %i\n", (*nodeIt) ->quantity);

	}

    for (nodeIt = st_nodes.begin(); nodeIt < st_nodes.end(); nodeIt++){
        printf ("Storage Node Type:\t %s\n", (*nodeIt) ->nodeType.c_str());
        printf ("Quantity of nodes:\t %i\n", (*nodeIt) ->quantity);

    }

}




} // namespace icancloud
} // namespace inet
