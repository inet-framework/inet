//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PATHLOSSBASE_H
#define __INET_PATHLOSSBASE_H

#include "inet/common/Module.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IPathLoss.h"

namespace inet {

namespace physicallayer {

class INET_API PathLossBase : public Module, public IPathLoss
{
  protected:
    /**
     * The path loss of a reception reduced to its distance: computed by
     * computeDistancePathLoss() at each frequency.
     */
    class INET_API DistanceReceptionPathLoss : public IReceptionPathLoss {
      protected:
        const PathLossBase *pathLoss;
        mps propagationSpeed;
        m distance;

      public:
        DistanceReceptionPathLoss(const PathLossBase *pathLoss, mps propagationSpeed, m distance) : pathLoss(pathLoss), propagationSpeed(propagationSpeed), distance(distance) {}
        virtual double computePathLoss(Hz frequency) const override { return pathLoss->computeDistancePathLoss(propagationSpeed, frequency, distance); }
        virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    };

  public:
    virtual double computeDistancePathLoss(mps propagationSpeed, Hz frequency, m distance) const override = 0;
    virtual Ptr<const IReceptionPathLoss> computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

