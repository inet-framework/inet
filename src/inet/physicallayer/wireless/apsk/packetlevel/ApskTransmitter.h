//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKTRANSMITTER_H
#define __INET_APSKTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h"

namespace inet {

namespace physicallayer {

class INET_API ApskTransmitter : public FlatTransmitterBase
{
  public:
    ApskTransmitter();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

