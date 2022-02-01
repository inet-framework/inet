//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROTOCOLPRINTER_H
#define __INET_PROTOCOLPRINTER_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * Protocol printer classes print protocol specific chunks into a context.
 */
class INET_API ProtocolPrinter : public cObject
{
  public:
    class INET_API Context {
      public:
        std::stringstream sourceColumn;
        std::stringstream destinationColumn;
        std::stringstream typeColumn;
        std::stringstream infoColumn;
    };

  public:
    /**
     * Prints the given chunk of protocol according to options into context.
     */
    virtual void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const = 0;
};

class INET_API DefaultProtocolPrinter : public ProtocolPrinter
{
  public:
    virtual void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const override;
};

} // namespace

#endif

