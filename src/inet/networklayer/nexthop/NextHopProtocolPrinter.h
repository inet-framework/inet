//
// Copyright (C) 2018 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_NEXTHOPPROTOCOLPRINTER_H
#define __INET_NEXTHOPPROTOCOLPRINTER_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/printer/ProtocolPrinter.h"

namespace inet {

class INET_API NextHopProtocolPrinter : public ProtocolPrinter
{
  public:
    virtual void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const override;
};

} // namespace inet

#endif // __INET_NEXTHOPPROTOCOLPRINTER_H

