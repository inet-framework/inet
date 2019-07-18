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

#include "inet/common/MatchableField.h"

namespace inet {

MatchableField::MatchableField(cObject *object, int fieldIndex)
{
    setField(object, fieldIndex);
}

void MatchableField::setField(cObject *object, int fieldIndex)
{
    ASSERT(object);
    this->object = object;
    this->fieldIndex = fieldIndex;
    this->classDescriptor = object->getDescriptor();
}

void MatchableField::setField(cObject *object, const char *fieldName)
{
    ASSERT(object);
    this->object = object;
    this->fieldIndex = -1;
    this->classDescriptor = object->getDescriptor();
    this->fieldIndex = classDescriptor->findField(fieldName);
    ASSERT(fieldIndex != -1);
}

const char *MatchableField::getAsString() const
{
    ASSERT(object && classDescriptor);
    return classDescriptor->getFieldName(fieldIndex);
}

const char *MatchableField::getAsString(const char *attribute) const
{
    ASSERT(object && classDescriptor);

    if (!strcmp("name", attribute))
        return classDescriptor->getFieldName(fieldIndex);
    else if (!strcmp("type", attribute))
        return classDescriptor->getFieldTypeString(fieldIndex);
    else if (!strcmp("declaredOn", attribute))
        return classDescriptor->getFieldDeclaredOn(fieldIndex);
    else
        return nullptr;
}

}  // namespace inet

