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

#ifndef __INET_STRINGFORMAT_H
#define __INET_STRINGFORMAT_H

#include <functional>
#include "inet/common/INETDefs.h"

namespace inet {

class INET_API StringFormat
{
  public:
    class INET_API IDirectiveResolver
    {
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

#endif // ifndef __INET_STRINGFORMAT_H

