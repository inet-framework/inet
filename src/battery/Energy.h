/***************************************************************************
 * Simple battery model for inetmanet framework
 * Author:  Isabel Dietrich
 *
 * This software is provided `as is' and without any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 ***************************************************************************/

/**
 * @short Class to hold the current energy level of the node.
*/

#ifndef ENERGY_H
#define ENERGY_H

// SYSTEM INCLUDES
#include <omnetpp.h>

class Energy : public cPolymorphic
{
  public:
    // LIFECYCLE
    Energy(double e=250) : cPolymorphic(), mEnergy(e) {};

    // OPERATIONS
    double  GetEnergy() const        { return mEnergy; }
    void    SetEnergy(double e)      { mEnergy = e; }
    void    SubtractEnergy(double e) { mEnergy -= e; }


  private:
    // MEMBER VARIABLES
    double mEnergy;

};

#endif
