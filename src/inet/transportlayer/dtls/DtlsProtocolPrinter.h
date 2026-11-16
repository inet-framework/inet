// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSPROTOCOLPRINTER_H
#define INET_DTLSPROTOCOLPRINTER_H

#include "inet/common/packet/printer/ProtocolPrinter.h"

namespace inet::dtls {

class INET_API DtlsProtocolPrinter : public ProtocolPrinter
{
  public:
    void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const override;
};

} // namespace inet::dtls
#endif
