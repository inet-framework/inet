//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MACPROTOCOLDISSECTOR_H
#define __INET_IEEE80211MACPROTOCOLDISSECTOR_H

#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet {

class INET_API Ieee80211MacProtocolDissector : public ProtocolDissector
{
  protected:
    virtual const Protocol *computeLlcProtocol(Packet *packet) const;

  public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};

} // namespace inet

#endif

