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

#ifndef __INET_SCTPPROTOCOLDISSECTOR_H
#define __INET_SCTPPROTOCOLDISSECTOR_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet {
namespace sctp {

class INET_API SctpProtocolDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};

} // namespace sctp
} // namespace inet

#endif // __INET_SCTPPROTOCOLDISSECTOR_H

