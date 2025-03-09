//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.protocolelement.fragmentation.base.FragmenterBase;
import inet.protocolelement.fragmentation.contract.IPacketFragmenter;

//
// Fragments packets and adds fragment number headers to each fragment for
// identification and reassembly. Implements a header-based fragmentation
// mechanism similar to IP fragmentation.
//
// Operation:
// - Divides packets into fragments according to fragmentation policy
// - Adds FragmentNumberHeader to each fragment with sequence information
// - Marks first and last fragments with special flags
// - Inserts headers at configurable position (front or back)
// - Forwards fragments to lower layers with appropriate headers
//
// @see ~FragmentNumberHeaderBasedDefragmenter, ~FragmentNumberHeaderInserter, ~FragmenterBase
//
module FragmentNumberHeaderBasedFragmenter extends FragmenterBase like IPacketFragmenter
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(FragmentNumberHeaderBasedFragmenter);
}
