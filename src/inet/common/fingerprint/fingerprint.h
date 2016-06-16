//==========================================================================
//   FINGERPRINT.H  - part of
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2016 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __INET_CUSTOMFINGERPRINT_H
#define __INET_CUSTOMFINGERPRINT_H

#include "inet/common/INETDefs.h"

#include "omnetpp/cfingerprint.h"

namespace inet {

/**
 * @brief This class calculates the "fingerprint" of a simulation.
 *
 * @see cSimulation::getFingerprintCalculator()
 * @ingroup ExtensionPoints
 */
class SIM_API CustomFingerprintCalculator : public cSingleFingerprintCalculator
{
  public:
    virtual CustomFingerprintCalculator *dup() const override { return new CustomFingerprintCalculator(); }
    virtual void addEvent(cEvent *event) override;
};

} // namespace omnetpp

#endif

