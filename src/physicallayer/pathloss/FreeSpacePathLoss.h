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

#ifndef __INET_FREESPACEPATHLOSS_H
#define __INET_FREESPACEPATHLOSS_H

#include "inet/physicallayer/contract/IPathLoss.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements the deterministic free space path loss model.
 *
 * @author Oliver Graute
 */
class INET_API FreeSpacePathLoss : public cModule, public IPathLoss
{
  protected:
    double alpha;
    double systemLoss;

  protected:
    virtual void initialize(int stage);
    virtual double computeFreeSpacePathLoss(m waveLength, m distance, double alpha, double systemLoss) const;

  public:
    FreeSpacePathLoss();
    virtual void printToStream(std::ostream& stream) const;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const;
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_FREESPACEPATHLOSS_H

