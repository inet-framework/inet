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

#ifndef __INET_NAKAGAMIFADING_H
#define __INET_NAKAGAMIFADING_H

#include "inet/physicallayer/wireless/common/pathloss/FreeSpacePathLoss.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements the Nakagami fading model.
 */
class INET_API NakagamiFading : public FreeSpacePathLoss
{
  protected:
    double shapeFactor;

  protected:
    virtual void initialize(int stage) override;

  public:
    NakagamiFading();
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

