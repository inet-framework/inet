//
// Copyright (C) 2005 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_IPV6HEADER_H
#define __INET_IPV6HEADER_H

#include <list>

#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"

namespace inet {

std::ostream& operator<<(std::ostream& out, const Ipv6ExtensionHeader&);

} // namespace inet

#endif

