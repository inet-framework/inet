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

#include "inet/common/StringFormat.h"

namespace inet {

void StringFormat::parseFormat(const char *format)
{
    this->format = format;
}

const char *StringFormat::formatString(IDirectiveResolver *resolver) const
{
    return formatString(format.c_str(), resolver);
}

const char *StringFormat::formatString(std::function<const char *(char)>& resolver) const
{
    return formatString(format.c_str(), resolver);
}

const char *StringFormat::formatString(const char *format, const IDirectiveResolver *resolver)
{
    return formatString(format, [&] (char directive) { return resolver->resolveDirective(directive); });
}

const char *StringFormat::formatString(const char *format, const std::function<const char *(char)> resolver)
{
    static std::string result;
    result.clear();
    int current = 0;
    int previous = current;
    while (true) {
        char ch = format[current];
        if (ch == '\0') {
            if (previous != current)
                result.append(format + previous, current - previous);
            break;
        }
        else if (ch == '%') {
            if (previous != current)
                result.append(format + previous, current - previous);
            previous = current;
            current++;
            ch = format[current];
            if (ch == '%')
                result.append(format + previous, current - previous);
            else
                result.append(resolver(ch));
            previous = current + 1;
        }
        current++;
    }
    return result.c_str();
}

} // namespace inet

