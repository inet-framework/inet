//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MQAMMODULATION_H
#define __INET_MQAMMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/MqamModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements gray coded rectangular quadrature amplitude modulation
 * that arranges symbols evenly.
 *
 * http://en.wikipedia.org/wiki/Quadrature_amplitude_modulation#Rectangular_QAM
 */
class INET_API MqamModulation : public MqamModulationBase
{
  public:
    MqamModulation(unsigned int codeWordSize);
    virtual ~MqamModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

