//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

class LabelsTag extends TagBase
{
    string labels[];
}

cplusplus(LabelsTag) {{
#if (OMNETPP_BUILDNUM < 1530)
  public:
    void appendLabels(const char *labels) { insertLabels(labels); }
#endif
}}
