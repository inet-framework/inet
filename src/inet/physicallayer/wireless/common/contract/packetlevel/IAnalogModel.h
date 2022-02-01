//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IANALOGMODEL_H
#define __INET_IANALOGMODEL_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IInterference.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IListening.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ISnir.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {
namespace physicallayer {

/**
 * This interface models how a radio signal attenuates during propagation. It
 * includes various effects such as free-space path loss, shadowing, refraction,
 * reflection, absorption, diffraction and others.
 */
class INET_API IAnalogModel : public IPrintableObject
{
  public:
    /**
     * Returns the reception for the provided transmission at the receiver.
     * The result incorporates all modeled attenuation. This function never
     * returns nullptr.
     */
    virtual const IReception *computeReception(const IRadio *receiver, const ITransmission *transmission, const IArrival *arrival) const = 0;

    /**
     * Returns the total noise summing up all the interfering receptions and
     * noises. This function never returns nullptr.
     */
    virtual const INoise *computeNoise(const IListening *listening, const IInterference *interference) const = 0;

    /**
     * Returns the total noise summing up all the reception and the noise.
     * This function never returns nullptr.
     */
    virtual const INoise *computeNoise(const IReception *reception, const INoise *noise) const = 0;

    /**
     * Returns the signal to noise and interference ratio. This function never
     * returns nullptr.
     */
    virtual const ISnir *computeSNIR(const IReception *reception, const INoise *noise) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

