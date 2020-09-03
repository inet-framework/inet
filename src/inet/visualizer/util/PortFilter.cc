//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/visualizer/util/PortFilter.h"

namespace inet {

namespace visualizer {

void PortFilter::setPattern(const char* pattern)
{
    matchExpression.setPattern(pattern, false, true, true);
}

bool PortFilter::matches(int value) const
{
    std::string text = std::to_string(value);
    cMatchableString matchableString(text.c_str());
    // TODO: eliminate const_cast when cMatchExpression::matches becomes const
    return const_cast<PortFilter *>(this)->matchExpression.matches(&matchableString);
}

} // namespace visualizer

} // namespace inet
