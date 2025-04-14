//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STRINGFORMAT_H
#define __INET_STRINGFORMAT_H

#include <functional>

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API StringFormat
{
  public:
    class INET_API IResolver {
      public:
        virtual std::string resolveDirective(char directive) const = 0;
        virtual std::string resolveExpression(const char *expression) const { return ""; } // TODO: make this = 0 when derived classes are updated
    };

  protected:
    std::string format;

  public:
    void parseFormat(const char *format);

    std::string formatString(const IResolver *resolver) const;
    std::string formatString(std::function<std::string(char)>& directiveResolver) const;
    std::string formatString(std::function<std::string(const char*)>& expressionResolver) const;
    std::string formatString(std::function<std::string(char)>& directiveResolver, std::function<std::string(const char*)>& expressionResolver) const;

    static std::string formatString(const char *format, const IResolver *resolver);
    static std::string formatString(const char *format, const std::function<std::string(char)> directiveResolver);
    static std::string formatString(const char *format, const std::function<std::string(const char *)> expressionResolver);
    static std::string formatString(const char *format, const std::function<std::string(char)> directiveResolver, const std::function<std::string(const char *)> expressionResolver);
};

} // namespace inet

#endif

