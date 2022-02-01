//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

} // namespace inet

