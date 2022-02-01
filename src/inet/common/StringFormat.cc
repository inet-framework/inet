//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

