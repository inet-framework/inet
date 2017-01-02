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

#ifndef __INET_MATCHABLEOBJECT_H
#define __INET_MATCHABLEOBJECT_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Wrapper around a cObject to make it matchable with cMatchExpression.
 */
class INET_API MatchableObject : public cMatchExpression::Matchable
{
  public:
    enum Attribute {
        ATTRIBUTE_FULLNAME,
        ATTRIBUTE_FULLPATH,
        ATTRIBUTE_CLASSNAME
    };

  protected:
    Attribute defaultAttribute;
    const cObject *object;
    mutable cClassDescriptor *classDescriptor;
    mutable std::string attributeValue;

  protected:
    static void splitIndex(char *indexedName, int& index);
    static bool findDescriptorField(cClassDescriptor *classDescriptor, const char *attribute, int& fieldId, int& index);

  public:
    MatchableObject(Attribute defaultAttribute = ATTRIBUTE_FULLPATH, const cObject *object = nullptr);

    void setObject(const cObject *object);
    void setDefaultAttribute(Attribute defaultAttribute) { this->defaultAttribute = defaultAttribute; }

    virtual const char *getAsString() const override;
    virtual const char *getAsString(const char *attribute) const override;
};

} // namespace inet

#endif // #ifdef __INET_MATCHABLEOBJECT_H

