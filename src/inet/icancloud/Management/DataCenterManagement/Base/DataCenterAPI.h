/**
 * @class DataCenterAPI DataCenterAPI.cc DataCenterAPI.h
 *
 * This api defines all the operations related to the nodes of the data center (compute and storage).
 * It has two machine map structures in order to group the different elements of the underlying architecture.
 * It is also possible to know the state of each hardware device, the amount of energy consumed, and the power
 * that is consuming by them.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2014-06-06
 */

#ifndef DATACENTERAPI_H_
#define DATACENTERAPI_H_

#include "inet/icancloud/Management/MachinesStructure/MachinesMap.h"
#include "inet/icancloud/Architecture/Node/AbstractNode.h"
#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {


class AbstractNode;

class DataCenterAPI : virtual public icancloud_Base {

protected:

    /** Structures to resources management  **/
    MachinesMap* nodesMap;

    /** Structures to resources management  **/
    MachinesMap* storage_nodesMap;

    // ---------------------- API of nodes from dataCenter  ------------------------------------
    /*
     *  The parameter storage at all methods represents:
     *  storage -> storageMap; .- storage nodes
     *  !storage -> map; .- compute nodes.
     */

    /*
    *  These methods returns the size of the nodesMap associated to the storage or compute nodes structures
    */
    int getDifferentTypesSize(bool storage = false);

    /*
    * This method returns the index of a nodeSet
    */
    int getSetPosition (string setId, bool storage = false);

    int getSetSize(bool storage, string setId);
    int getSetSize(bool storage, int setIndex);

    /*
     * Returns the set size
     */
    int getMapSize(){return nodesMap->size();};
    int getStorageMapSize(){return storage_nodesMap->size();};

    /*
    * This method returns the name of the set positioned at index
    */
    string getSetName (int index, bool storage = false);

    /*
    * These methods returns the quantity of nodes that a nodeSet has
    */
    int getSetSize (string setId, bool storage = false);
    int getSetSize (int nodeIndex, bool storage = false);

    /*
    * These methods returns the number of CPUs that all the nodes of the set has.
    */
    int getSetNumCores(string setId, bool storage = false);
    int getSetNumCores(int nodeIndex, bool storage = false);

    /*
    * These methods returns the size of the memory that all the nodes of the set has.
    */
    int getSetMemorySize(string setId, bool storage = false);
    int getSetMemorySize(int nodeIndex, bool storage = false);

    /*
    * These methods returns the amount of free storage that all the nodes of the set has.
    */
    int getSetStorageSize(string setId, bool storage = false);
    int getSetStorageSize(int nodeIndex, bool storage = false);

    /*
    * These methods returns the state of a node into a nodeSet
    */
    string getNodeState (string setId, int nodeIndex, bool storage = false);
    string getNodeState (int setIndex, int nodeIndex, bool storage = false);

    /*
    * These methods returns the number of nodes in ON "state"
    */
    int countONNodes (string setId, bool storage = false);
    int countONNodes (int setIndex, bool storage = false);

    /*
    * These methods returns the number of nodes in OFF "state"
    */
    int countOFFNodes (string setId, bool storage = false);
    int countOFFNodes (int setIndex, bool storage = false);

    /*
    * These methods return the node positioned at nodeIndex into the requested Set.
    */
    AbstractNode* getNodeByIndex (string setId, int nodeIndex, bool storage = false);
    AbstractNode* getNodeByIndex (int setIndex, int nodeIndex, bool storage = false);

    /*
     * This method returns the node ip.
     */
    string getNodeIp (string setIndex, int nodeIndex, bool storage = false);

    /*
    * These methods returns the number of nodes at given state requested in the nodeSet
    */
    vector<AbstractNode*> getONNodes (string setId, bool storage = false);
    vector<AbstractNode*> getONNodes (int setIndex, bool storage = false);
    vector<AbstractNode*> getOFFNodes (string setId, bool storage = false);
    vector<AbstractNode*> getOFFNodes (int setIndex, bool storage = false);

    /*
     * This method returns the node that has the same ip than given as parameter
     */
    AbstractNode* getNodeByIP (string ip);

    /*
     * The method returns the DCElementtype
     */
    elementType* getElementType (int index, bool storageNodes);
    void setElementType(elementType* element, int index, bool storageNodes = false);

    int getNumberOfProcesses(int nodeSet, int nodeID, bool storageNodes = false);


    /*********************************************************************************************
    ************************************  Inheritance methods *********************************
    *********************************************************************************************/
    /*
     * Destructor
     */
    virtual ~DataCenterAPI();

    void printNodeMapInfo();

};

} // namespace icancloud
} // namespace inet

#endif /* DATACENTERAPI_H_ */
