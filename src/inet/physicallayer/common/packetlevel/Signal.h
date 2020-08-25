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

#ifndef __INET_SIGNAL_H
#define __INET_SIGNAL_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

class INET_API Signal : public cPacket
{
  public:
    explicit Signal(const char *name=nullptr, short kind=0, int64_t bitLength=0);
    Signal(const Signal& other);

    virtual Signal *dup() const override { return new Signal(*this); }

    virtual std::string str() const override;
};

inline std::ostream& operator<<(std::ostream& os, const Signal *signal) {
    if (!signal)
        return os << "(Signal)\x1b[3m<nullptr>\x1b[0m";
    return os << signal->str();
}

inline std::ostream& operator<<(std::ostream& os, const Signal& signal) {
    return os << signal.str();
}

} // namespace physicallayer
} // namespace inet

#endif

