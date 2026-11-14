// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_DSDVPROTOCOLPRINTER_H
#define __INET_DSDVPROTOCOLPRINTER_H
#include "inet/common/packet/printer/ProtocolPrinter.h"
namespace inet {
namespace dsdv {
class INET_API DsdvProtocolPrinter : public ProtocolPrinter
{
  public:
    virtual void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const override;
};
} // namespace dsdv
} // namespace inet
#endif
