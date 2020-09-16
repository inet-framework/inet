//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_SIGNAL_H
#define __INET_SIGNAL_H

#include "inet/common/IPrintableObject.h"

namespace inet {
namespace physicallayer {

class INET_API Signal : public cPacket, public IPrintableObject
{
  public:
    explicit Signal(const char *name=nullptr, short kind=0, int64_t bitLength=0);
    Signal(const Signal& other);

    virtual Signal *dup() const override { return new Signal(*this); }

    virtual const char *getFullName() const override;

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual std::string str() const override;
};

} // namespace physicallayer
} // namespace inet

#endif

