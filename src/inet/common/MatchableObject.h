//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#endif

