/**
 * @short This class displays the communication radius of a host and provides
 *  the interference distance.
 * @author Isabel Dietrich
*/

#define DEBUG

#ifndef DISPLAY_H
#define DISPLAY_H

// SYSTEM INCLUDES
#include <omnetpp.h>

class Display : public cSimpleModule
{
  public:
    // LIFECYCLE

    virtual void initialize(int);

    // OPERATIONS
    double calcInterfDist();

  private:
    // OPERATIONS
    /** @brief Function to get a pointer to the host module*/
    cModule *findHost(void) const;

};

#endif

