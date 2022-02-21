//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

class FlowTag extends TagBase
{
    string names[];
}

cplusplus(FlowTag) {{
#if (OMNETPP_BUILDNUM < 1530)
  public:
    void appendNames(const char *names) { insertNames(names); }
#endif
}}
