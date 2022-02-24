//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ILAYEREDERRORMODEL_H
#define __INET_ILAYEREDERRORMODEL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ISnir.h"

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
//TODO non-contract included and used in contract. maybe create and use ILayeredTransmission?
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredTransmission.h"
#endif

namespace inet {

namespace physicallayer {

#ifndef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
//TODO non-contract included and used in contract. maybe create and use ILayeredTransmission?
class LayeredTransmission;
#endif

/**
 * The layered error model computes the erroneous bits, symbols, or samples
 * based on the SNIR and a simplified model of the signal processing.
 */
class INET_API ILayeredErrorModel : public IPrintableObject
{
  public:
    /**
     * Computes the packet domain representation at the receiver using a simplified
     * model for the underlying domains. This result includes all potential
     * errors that were not corrected by the underlying domains.
     */
    virtual const IReceptionPacketModel *computePacketModel(const LayeredTransmission *transmission, const ISnir *snir) const = 0;

    /**
     * Computes the bit domain representation at the receiver using a simplified
     * model for the underlying domains. This result includes all potential
     * errors that were not corrected by the underlying domains.
     */
    virtual const IReceptionBitModel *computeBitModel(const LayeredTransmission *transmission, const ISnir *snir) const = 0;

    /**
     * Computes the symbol domain representation at the receiver using a simplified
     * model for the underlying domains. This result includes all potential
     * errors that were not corrected by the underlying domains.
     */
    virtual const IReceptionSymbolModel *computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const = 0;

    /**
     * Computes the sample domain representation at the receiver using a simplified
     * model for the underlying domains. This result includes all potential
     * errors that were not corrected by the underlying domains.
     */
    virtual const IReceptionSampleModel *computeSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

