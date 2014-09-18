/***************************************************************************
* author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
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

#ifndef __INET_RAYLEIGHFADING_H
#define __INET_RAYLEIGHFADING_H

#include "inet/physicallayer/pathloss/FreeSpacePathLoss.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements the probabilistic Rayleigh fading model, see Rappaport
 * for more details.
 *
 * @author Oliver Graute
 */
class INET_API RayleighFading : public FreeSpacePathLoss
{
  public:
    virtual void printToStream(std::ostream& stream) const;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RAYLEIGHFADING_H

