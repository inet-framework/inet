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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/visualizer/util/ModuleFilter.h"

namespace inet {

namespace visualizer {

void ModuleFilter::setPattern(const char* pattern)
{
    matchExpression.setPattern(pattern, true, true, true);
}

bool ModuleFilter::matches(const cModule *module) const
{
    MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLPATH, module);
    // TODO: eliminate const_cast when cMatchExpression::matches becomes const
    return const_cast<ModuleFilter *>(this)->matchExpression.matches(&matchableObject);
}

} // namespace visualizer

} // namespace inet
