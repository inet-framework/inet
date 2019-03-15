/*
 * LoRaReception.h
 *
 *  Created on: Feb 17, 2017
 *      Author: slabicm1
 */

#ifndef LORAPHY_LORARECEPTION_H_
#define LORAPHY_LORARECEPTION_H_

#include "inet/physicallayer/analogmodel/packetlevel/ScalarReception.h"

namespace inet {

namespace physicallayer {

class INET_API LoRaReception : public ScalarReception
{
protected:
    const Hz LoRaCF;
    const int LoRaSF;
    const Hz LoRaBW;
    const double LoRaCR;
    const W receivedPower;
  public:
    LoRaReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, Hz LoRaCF, Hz LoRaBW, W receivedPower, int LoRaSF, int LoRaCR);

    Hz getLoRaCF() const { return LoRaCF; }
    int getLoRaSF() const { return LoRaSF; }
    Hz getLoRaBW() const { return LoRaBW; }
    double getLoRaCR() const { return LoRaCR; }

    virtual W getPower() const override { return receivedPower; }
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif /* LORAPHY_LORARECEPTION_H_ */
