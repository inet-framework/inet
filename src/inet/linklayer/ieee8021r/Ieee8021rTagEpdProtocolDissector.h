//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021RTAGEPDPROTOCOLDISSECTOR_H
#define __INET_IEEE8021RTAGEPDPROTOCOLDISSECTOR_H

#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee8021rTagEpdProtocolDissector : public DefaultProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

