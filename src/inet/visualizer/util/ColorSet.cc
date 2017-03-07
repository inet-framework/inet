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

#include "inet/visualizer/util/ColorSet.h"

namespace inet {

namespace visualizer {

void ColorSet::parseColors(const char *colorNames)
{
    colors.clear();
    if (!strcmp(colorNames, "dark")) {
        for (auto color : cFigure::GOOD_DARK_COLORS)
            colors.push_back(color);
    }
    else if (!strcmp(colorNames, "light")) {
        for (auto color : cFigure::GOOD_LIGHT_COLORS)
            colors.push_back(color);
    }
    else {
        cStringTokenizer tokenizer(colorNames, " ,");
        while (tokenizer.hasMoreTokens()) {
            colors.push_back(cFigure::parseColor(tokenizer.nextToken()));
        }
    }
}

cFigure::Color ColorSet::getColor(int index) const
{
    return colors[index % colors.size()];
}

} // namespace visualizer

} // namespace inet

