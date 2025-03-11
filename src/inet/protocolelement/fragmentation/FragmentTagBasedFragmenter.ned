//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.protocolelement.fragmentation.base.FragmenterBase;
import inet.protocolelement.fragmentation.contract.IPacketFragmenter;

//
// Fragments packets using packet tags to track fragment relationships instead
// of explicit headers. Provides a lightweight fragmentation mechanism that
// doesn't modify packet contents.
//
// Operation:
// - Divides packets into fragments according to fragmentation policy
// - Attaches FragmentTag to each fragment for identification
// - Maintains original packet data in the tags rather than headers
// - Preserves packet structure while enabling fragmentation
// - Forwards fragments to lower layers with appropriate tags
//
// Particularly useful for:
// - Implementing fragmentation in simulation-only scenarios
// - Testing fragmentation without protocol header overhead
// - Creating simplified fragmentation mechanisms
// - Studying fragmentation effects without protocol specifics
// - Implementing custom fragmentation schemes
//
// @see ~FragmentTagBasedDefragmenter, ~FragmenterBase
//
module FragmentTagBasedFragmenter extends FragmenterBase like IPacketFragmenter
{
    parameters:
        @class(FragmentTagBasedFragmenter);
}
