/*
 * LoRaTransmission.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: slabicm1
 */

#include "inet/lora/loraphy/LoRaTransmission.h"

namespace inet {
namespace lora {

LoRaTransmission::LoRaTransmission(const IRadio *transmitter, const Packet *macFrame, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, W LoRaTP, Hz LoRaCF, int LoRaSF, Hz LoRaBW, int LoRaCR):
        TransmissionBase(transmitter, macFrame, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation),
        LoRaTP(LoRaTP),
        LoRaCF(LoRaCF),
        LoRaSF(LoRaSF),
        LoRaBW(LoRaBW),
        LoRaCR(LoRaCR)
{
    // TODO Auto-generated constructor stub

}

std::ostream& LoRaTransmission::printToStream(std::ostream& stream, int level) const
{
    /*if (level <= PRINT_LEVEL_DETAIL)
        stream << ", carrierFrequency = " << carrierFrequency;
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", bandwidth = " << bandwidth
               << ", modulation = " << printObjectToString(modulation, level + 1) ;
               */
    return TransmissionBase::printToStream(stream, level);
}


} /* namespace physicallayer */
} /* namespace inet */
