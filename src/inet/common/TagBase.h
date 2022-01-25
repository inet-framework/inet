//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#ifndef __INET_TAGBASE_H
#define __INET_TAGBASE_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/Ptr.h"
#include "inet/common/Units.h"

namespace inet {

using namespace inet::units::values;

class INET_API TagBase : public cObject, public SharedBase<TagBase>, public IPrintableObject
{
  public:
    virtual const Ptr<TagBase> dupShared() const { return Ptr<TagBase>(static_cast<TagBase *>(dup())); }

    virtual const Ptr<TagBase> changeRegion(b offsetDelta, b lengthDelta) const { return const_cast<TagBase *>(this)->shared_from_this(); }

    virtual void parsimPack(cCommBuffer *buffer) const override {}
    virtual void parsimUnpack(cCommBuffer *buffer) override {}

    virtual std::ostream& printToStream(std::ostream &stream, int level, int evFlags) const override;

    virtual std::ostream& printFieldsToStream(std::ostream &stream, int level, int evFlags) const;

    virtual std::string str() const override;
};

} // namespace inet

#endif

