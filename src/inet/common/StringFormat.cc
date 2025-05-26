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

std::string StringFormat::formatString(const IResolver *resolver) const
{
    return formatString(format.c_str(), resolver);
}

std::string StringFormat::formatString(std::function<std::string(char)>& directiveResolver) const
{
    return formatString(format.c_str(), directiveResolver, [](const char *) { return std::string(); });
}

std::string StringFormat::formatString(std::function<std::string(const char*)>& expressionResolver) const
{
    return formatString(format.c_str(), [](const char) { return std::string(); }, expressionResolver);
}

std::string StringFormat::formatString(std::function<std::string(char)>& directiveResolver, std::function<std::string(const char*)>& expressionResolver) const
{
    return formatString(format.c_str(), directiveResolver, expressionResolver);
}

std::string StringFormat::formatString(const char *format, const IResolver *resolver)
{
    return formatString(format, [&] (char directive) { return resolver->resolveDirective(directive); }, [&] (const char *expression) { return resolver->resolveExpression(expression); });
}

std::string StringFormat::formatString(const char *format, const std::function<std::string(char)> directiveResolver)
{
    return formatString(format, directiveResolver, [] (const char *) { return std::string(); });
}

std::string StringFormat::formatString(const char *format, const std::function<std::string(const char *)> expressionResolver)
{
    return formatString(format, [] (const char) { return std::string(); }, expressionResolver);
}

std::string StringFormat::formatString(const char *format, const std::function<std::string(char)> directiveResolver, const std::function<std::string(const char *)> expressionResolver)
{
    std::string result;
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
                result.append(directiveResolver(ch));
            previous = current + 1;
        }
        else if (ch == '{') {
            // Check for escaped opening brace ({{)
            if (format[current + 1] == '{') {
                // Append text up to and including the first '{'
                if (previous != current)
                    result.append(format + previous, current - previous);
                result.append("{");
                previous = current + 2; // Skip both braces
                current += 2;
                continue;
            }

            // Handle normal expression
            if (previous != current)
                result.append(format + previous, current - previous);
            previous = current + 1; // Skip the opening brace
            current++;

            // Find the closing brace
            while (format[current] != '}' && format[current] != '\0')
                current++;

            if (format[current] == '}') {
                // Extract the expression inside braces
                std::string expression(format + previous, current - previous);
                result.append(expressionResolver(expression.c_str()));
                previous = current + 1; // Skip the closing brace
            }
            else {
                // No closing brace found, treat as normal text
                previous = previous - 1; // Include the opening brace
                result.append(format + previous, current - previous);
                previous = current;
            }
        }
        else if (ch == '}') {
            // Check for escaped closing brace (}})
            if (format[current + 1] == '}') {
                // Append text up to and including the first '}'
                if (previous != current)
                    result.append(format + previous, current - previous);
                result.append("}");
                previous = current + 2; // Skip both braces
                current += 2;
                continue;
            }
            else {
                // Unmatched closing brace, treat as normal text
                if (previous != current)
                    result.append(format + previous, current - previous);
                result.append("}");
                previous = current + 1;
                current++;
                continue;
            }
        }
        current++;
    }
    return result;
}

} // namespace inet

