/**
 * @short This class displays the communication radius of a host and provides
 *  the interference distance.
 * @author Isabel Dietrich
*/

#define DEBUG

#ifndef __INET_DISPLAY_H
#define __INET_DISPLAY_H

// SYSTEM INCLUDES
#include "INETDefs.h"


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

#endif  // __INET_DISPLAY_H

