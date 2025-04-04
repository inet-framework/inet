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
    class INET_API IDirectiveResolver {
      public:
        virtual std::string resolveDirective(char directive) const = 0;
        virtual std::string resolveExpression(const char *expression) const { return ""; }
    };

  protected:
    std::string format;

  public:
    void parseFormat(const char *format);

    std::string formatString(IDirectiveResolver *resolver) const;
    std::string formatString(std::function<std::string(char)>& resolver) const;
    
    static std::string formatString(const char *format, const IDirectiveResolver *resolver);
    static std::string formatString(const char *format, const std::function<std::string(char)> resolver);
    static std::string formatString(const char *format, const std::function<std::string(char)> resolver, const std::function<std::string(const char *)> expressionResolver);
};

} // namespace inet

#endif

