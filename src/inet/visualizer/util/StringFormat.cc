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

#include "inet/visualizer/util/StringFormat.h"

namespace inet {

namespace visualizer {

void StringFormat::parseFormat(const char *format)
{
    this->format = format;
}

const char *StringFormat::formatString(IDirectiveResolver *directiveResolver) const
{
    result.clear();
    int current = 0;
    int previous = current;
    while (true) {
        char ch = format[current];
        if (ch == '\0') {
            if (previous != current)
                result.append(format.begin() + previous, format.begin() + current);
            break;
        }
        else if (ch == '%') {
            if (previous != current)
                result.append(format.begin() + previous, format.begin() + current);
            previous = current;
            current++;
            ch = format[current];
            if (ch == '%')
                result.append(format.begin() + previous, format.begin() + current);
            else
                result.append(directiveResolver->resolveDirective(ch));
            previous = current + 1;
        }
        current++;
    }
    return result.c_str();
}

} // namespace visualizer

} // namespace inet

