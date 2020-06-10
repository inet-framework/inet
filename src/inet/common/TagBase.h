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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_TAGBASE_H
#define __INET_TAGBASE_H

#include "inet/common/Ptr.h"

namespace inet {

class INET_API TagBase : public cObject, public SharedBase<TagBase>
{
  public:
    virtual const Ptr<TagBase> dupShared() const { return Ptr<TagBase>(static_cast<TagBase *>(dup())); };

    virtual void parsimPack(cCommBuffer *buffer) const override { }
    virtual void parsimUnpack(cCommBuffer *buffer) override { }

    virtual std::string str() const override { return getClassName(); }
};

} // namespace inet

#endif // ifndef __INET_TAGBASE_H

