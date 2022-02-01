//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICMPPROTOCOLPRINTER_H
#define __INET_ICMPPROTOCOLPRINTER_H

#include "inet/common/packet/printer/ProtocolPrinter.h"

namespace inet {

class INET_API IcmpProtocolPrinter : public ProtocolPrinter
{
  public:
    virtual void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const override;
};

} // namespace inet

#endif

