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

#ifndef __INET_MATCHABLEFIELD_H
#define __INET_MATCHABLEFIELD_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Wrapper around a cObject field to make it matchable with cMatchExpression.
 * The default attribute is the field name.
 */
class INET_API MatchableField : public cMatchExpression::Matchable
{
  protected:
    cObject *object;
    mutable cClassDescriptor *classDescriptor;
    int fieldIndex;

  public:
    MatchableField(cObject *object = nullptr, int fieldIndex = -1);
    void setField(cObject *object, int fieldIndex);
    void setField(cObject *object, const char *fieldName);
    virtual const char *getAsString() const override;
    virtual const char *getAsString(const char *attribute) const override;
};

} // namespace inet

#endif // #ifdef __INET_MATCHABLEOBJECT_H

