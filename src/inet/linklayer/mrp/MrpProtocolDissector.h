//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MRPPROTOCOLDISSECTOR_H
#define __INET_MRPPROTOCOLDISSECTOR_H

#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet {

class INET_API MrpProtocolDissector: public ProtocolDissector
{
public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback &callback) const override;
};

} // namespace inet

#endif
