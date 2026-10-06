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
 * correlation distance, the value is kept per pair of radios, the same in
 * both directions, and drawn again only when either radio has moved farther
 * than that distance from where it was when the value was drawn. The values
 * of a radio's links are dropped when the radio is removed from its medium.
 * The correlation needs a medium analog model that computes the path loss
 * per link, like ScalarMediumAnalogModel; with DimensionalMediumAnalogModel it
 * is an error.
 */
class INET_API LogNormalShadowing : public FreeSpacePathLoss, public cListener
{
  protected:
    struct LinkShadowing {
        Coord position1; // the position of the radio with the smaller id at the draw
        Coord position2; // the position of the radio with the larger id at the draw
        double shadowing; // dB
    };

  protected:
    double sigma;
    m correlationDistance = m(NaN);
    mutable std::map<std::pair<int, int>, LinkShadowing> linkShadowings; // by (smaller radio id, larger radio id)

  protected:
    virtual void initialize(int stage) override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance, double shadowing) const;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;

  public:
    LogNormalShadowing();
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computePathLoss(const ITransmission *transmission, const IArrival *arrival) const override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

