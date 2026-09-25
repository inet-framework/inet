//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/* **************************************************************************
 * author:      Andreas Kuntz
 *
 * copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
 *
 * author:      Alfonso Ariza
 *              Malaga university
 *
 ***************************************************************************/

#ifndef __INET_LOGNORMALSHADOWING_H
#define __INET_LOGNORMALSHADOWING_H

#include <map>

#include "inet/physicallayer/wireless/common/pathloss/FreeSpacePathLoss.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements the log normal shadowing model.
 *
 * By default every path loss computation draws a new shadowing value. With a
 * correlation distance, the value is kept per link (transmitter radio,
 * receiver radio) and drawn again only when the receiver has moved farther
 * than that distance from where the link's last value was drawn.
 */
class INET_API LogNormalShadowing : public FreeSpacePathLoss
{
  protected:
    struct LinkShadowing {
        Coord position; // the receiver's position at the draw
        double shadowing; // dB
    };

  protected:
    double sigma;
    m correlationDistance = m(NaN);
    mutable std::map<std::pair<int, int>, LinkShadowing> linkShadowings; // by (transmitter radio id, receiver radio id)

  protected:
    virtual void initialize(int stage) override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance, double shadowing) const;

  public:
    LogNormalShadowing();
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computePathLoss(const ITransmission *transmission, const IArrival *arrival) const override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

