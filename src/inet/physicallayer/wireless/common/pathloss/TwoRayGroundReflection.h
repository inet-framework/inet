//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/***************************************************************************
* author:      Andreas Kuntz
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
***************************************************************************/

#ifndef __INET_TWORAYGROUNDREFLECTION_H
#define __INET_TWORAYGROUNDREFLECTION_H

#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/physicallayer/wireless/common/pathloss/FreeSpacePathLoss.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements the two ray ground radio path loss model.
 */
class INET_API TwoRayGroundReflection : public FreeSpacePathLoss
{
  protected:
    const physicalenvironment::IPhysicalEnvironment *physicalEnvironment = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computePathLoss(const ITransmission *transmission, const IArrival *arrival) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

