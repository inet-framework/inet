// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_AODVSUBNETPROTOCOLPRINTER_H
#define __INET_AODVSUBNETPROTOCOLPRINTER_H

#include "inet/common/packet/printer/ProtocolPrinter.h"

namespace inet {

class INET_API AodvSubnetProtocolPrinter : public ProtocolPrinter
{
  public:
    void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const override;
};

} // namespace inet

#endif
