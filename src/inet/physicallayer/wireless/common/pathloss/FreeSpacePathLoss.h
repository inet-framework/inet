//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/***************************************************************************
* author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
***************************************************************************/

#ifndef __INET_FREESPACEPATHLOSS_H
#define __INET_FREESPACEPATHLOSS_H

#include "inet/physicallayer/wireless/common/base/packetlevel/PathLossBase.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements the deterministic free space path loss model.
 *
 * @author Oliver Graute
 */
class INET_API FreeSpacePathLoss : public PathLossBase
{
  protected:
    double alpha;
    double systemLoss;

  protected:
    virtual void initialize(int stage) override;
    virtual double computeFreeSpacePathLoss(m waveLength, m distance, double alpha, double systemLoss) const;

  public:
    FreeSpacePathLoss();
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

