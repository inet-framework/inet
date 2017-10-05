/***************************************************************************
* author:      Andreas Kuntz
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
*              This program is free software; you can redistribute it
*              and/or modify it under the terms of the GNU General Public
*              License as published by the Free Software Foundation; either
*              version 2 of the License, or (at your option) any later
*              version.
*              For further information see file COPYING
*              in the top level directory
***************************************************************************/

#ifndef __INET_TWORAYGROUNDREFLECTION_H
#define __INET_TWORAYGROUNDREFLECTION_H

#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/physicallayer/pathloss/FreeSpacePathLoss.h"

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
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual double computePathLoss(const ITransmission *transmission, const IArrival *arrival) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_TWORAYGROUNDREFLECTION_H

