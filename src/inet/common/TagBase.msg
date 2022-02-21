//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

cplusplus{{
#include "inet/common/TagBase.h"
}}

namespace inet;

//
// This is an abstract base class that should not be directly added to a message as a tag.
// It is used as the base class of all tags, thus it doesn't have any fields.
//
class TagBase extends cObject
{
    @existingClass;
}
