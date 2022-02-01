//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#endif

