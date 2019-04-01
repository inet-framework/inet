/*
 * @class cGateManager cGateManager.h "cGateManager.h"
 *
 * This class manages the dynamic creation gates, increasement and decreasement of gates vectors, and
 * link and unlink gates.
 *
 * @author Gabriel Gonzalez Castane
 * @date 2014-22-10
 */

#ifndef CGATEMANAGER_H_
#define CGATEMANAGER_H_

#include <omnetpp.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdexcept>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"

namespace inet {

namespace icancloud {


using std::vector;
using std::string;

using namespace omnetpp;

class cGateManager {

    struct gateStructure_t{
        omnetpp::cGate* gate;
        bool gateHit;
    };

    typedef gateStructure_t gateStructure;

    vector <int> holes;

    vector <gateStructure*> gates;

    omnetpp::cModule* mod;

public:

     cGateManager(omnetpp::cModule* mod);

      /*
       * Destructor
       */
      virtual ~cGateManager();

      /*
       * This method check if a gate is free.
       */
    int isGateEmpty(int index);

    /*
     * This method link a real gate with its virtual gate at vector's position
     */
    void linkGate(string gateName, int index);

    /*
     * This method create a new gate or give a free gate. It returns the position where
     * it will be created
     */
    int newGate(string gateName);

    /*
     * This method returns the gate by a given position as parameter
     */
    omnetpp::cGate* getGate(int index);

    /*
     * This method free the gate at position given as parameter disconnecting its possible
     * conexions
     */
    omnetpp::cGate* freeGate(int index);

    /*
     *  This method connects a gate (IN) with the gate given as parameter
     */
    void connectIn(omnetpp::cGate* gate, int index);

    /*
     *  This method connects a gate (OUT) with the gate given as parameter
     */
    void connectOut(omnetpp::cGate* gate, int index);

    /*
     * This method returns the gate position by a given gate identification
     */
    int searchGate(int gateId);


};

} // namespace icancloud
} // namespace inet

#endif /* GATEDATA_H_ */
