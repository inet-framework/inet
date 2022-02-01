//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMRADIO_H
#define __INET_IEEE80211OFDMRADIO_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatRadioBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmRadio : public FlatRadioBase
{
  protected:
    virtual void encapsulate(Packet *packet) const override;
    virtual void decapsulate(Packet *packet) const override;

  public:
    Ieee80211OfdmRadio();
};

} // namespace physicallayer

} // namespace inet

#endif

