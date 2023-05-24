//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GENERICRADIO_H
#define __INET_GENERICRADIO_H

#include "inet/physicallayer/wireless/common/radio/packetlevel/Radio.h"

namespace inet {

namespace physicallayer {

class INET_API GenericRadio : public Radio
{
  protected:
    virtual void encapsulate(Packet *packet) const override;
    virtual void decapsulate(Packet *packet) const override;

  public:
    GenericRadio();
};

} // namespace physicallayer

} // namespace inet

#endif

