//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPPROTOCOLPRINTER_H
#define __INET_SCTPPROTOCOLPRINTER_H

#include "inet/common/packet/printer/ProtocolPrinter.h"

namespace inet {
namespace sctp {

class INET_API SctpProtocolPrinter : public ProtocolPrinter
{
  public:
    virtual void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const override;
};

} // namespace sctp
} // namespace inet

#endif

