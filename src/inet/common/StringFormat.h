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
        virtual const char *resolveDirective(char directive) const = 0;
    };

  protected:
    std::string format;

  public:
    void parseFormat(const char *format);

    const char *formatString(IDirectiveResolver *resolver) const;
    const char *formatString(std::function<const char *(char)>& resolver) const;

    static const char *formatString(const char *format, const IDirectiveResolver *resolver);
    static const char *formatString(const char *format, const std::function<const char *(char)> resolver);
};

} // namespace inet

#endif

