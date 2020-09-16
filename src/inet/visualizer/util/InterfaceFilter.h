//
// Copyright (C) 2020 OpenSim Ltd.
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

#ifndef __INET_INTERFACEFILTER_H
#define __INET_INTERFACEFILTER_H

#include "inet/common/MatchableObject.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

namespace visualizer {

/**
 * This class provides a generic filter for interfaces. The filter is expressed
 * as a pattern using the cMatchExpression format.
 */
class INET_API InterfaceFilter
{
  protected:
    cMatchExpression matchExpression;

  public:
    void setPattern(const char *pattern);

    bool matches(const NetworkInterface *networkInterface) const;
};

} // namespace visualizer

} // namespace inet

#endif

