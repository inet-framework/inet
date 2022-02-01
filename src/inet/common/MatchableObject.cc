//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/MatchableObject.h"

namespace inet {

MatchableObject::MatchableObject(Attribute defaultAttribute, const cObject *object)
{
    this->defaultAttribute = defaultAttribute;
    this->object = object;
    classDescriptor = nullptr;
}

void MatchableObject::setObject(const cObject *object)
{
    this->object = object;
    classDescriptor = nullptr;
}

const char *MatchableObject::getAsString() const
{
    switch (defaultAttribute) {
        case ATTRIBUTE_FULLPATH:  attributeValue = object->getFullPath(); return attributeValue.c_str();
        case ATTRIBUTE_FULLNAME:  return object->getFullName();
        case ATTRIBUTE_CLASSNAME: return object->getClassName();
        default: throw cRuntimeError("Unknown setting for default attribute");
    }
}

void MatchableObject::splitIndex(char *indexedName, int& index)
{
    index = 0;
    char *startbracket = strchr(indexedName, '[');
    if (startbracket) {
        char *lastcharp = indexedName + strlen(indexedName) - 1;
        if (*lastcharp != ']')
            throw cRuntimeError("Unmatched '['");
        *startbracket = '\0';
        char *end;
        index = strtol(startbracket + 1, &end, 10);
        if (end != lastcharp)
            throw cRuntimeError("Brackets [] must contain numeric index");
    }
}

bool MatchableObject::findDescriptorField(cClassDescriptor *classDescriptor, const char *attribute, int& fieldId, int& index)
{
    // attribute may be in the form "fieldName[index]"; split the two
    char *fieldNameBuf = new char[strlen(attribute) + 1];
    strcpy(fieldNameBuf, attribute);
    splitIndex(fieldNameBuf, index);

    // find field by name
    fieldId = classDescriptor->findField(fieldNameBuf);
    delete[] fieldNameBuf;
    return fieldId != -1;
}

const char *MatchableObject::getAsString(const char *attribute) const
{
    if (!classDescriptor) {
        classDescriptor = object->getDescriptor();
        if (!classDescriptor)
            return nullptr;
    }

/*FIXME
    // start tokenizing the path
    cStringTokenizer tokenizer(attribute, ".");
    const char *token;
    void *currentObj = obj;
    cClassDescriptor *currentDesc = desc;
    int currentFieldId = id
    while ((token = tokenizer.nextToken())!=nullptr)
    {
        bool found = findDescriptorField(d, token, fid, index);
        if (!found) return nullptr;
    }
*/

    int fieldId;
    int index;
    bool found = findDescriptorField(classDescriptor, attribute, fieldId, index);
    if (!found)
        return nullptr;

    attributeValue = classDescriptor->getFieldValueAsString(toAnyPtr(object), fieldId, index);
    return attributeValue.c_str();
}

} // namespace inet

