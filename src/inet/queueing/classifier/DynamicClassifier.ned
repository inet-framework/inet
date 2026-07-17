//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// A packet classifier that creates the branch for each traffic class on demand. Each branch
// is a submodule of type `moduleType`, wired between this classifier's output and a downstream
// aggregator submodule (`aggregatorSubmoduleName`, a push multiplexer by default). When the
// aggregator is a pull scheduler implementing ~IDynamicInputScheduler, the new input is
// registered with it. With `spliceBranchSubmodules`, a compound `moduleType` forming a linear
// in->...->out chain is flattened: its inner submodules are instantiated directly in the
// parent (as vector elements named after them) so a matched-pair scheduler can address them.
//
simple DynamicClassifier extends PacketClassifier
{
    parameters:
        string moduleType; // NED type of the per-class branch module (may be a compound)
        string submoduleName = default(""); // submodule vector for the branch module (non-splice); unused when splicing
        string aggregatorSubmoduleName = default("multiplexer"); // downstream aggregator submodule to wire branches into
        bool spliceBranchSubmodules = default(false); // if true, flatten the compound moduleType's submodules into the branch
        @class(DynamicClassifier);
}
