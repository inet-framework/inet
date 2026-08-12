//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// Creates the branch of each traffic class on demand. Each branch is one element of
// the `submoduleName` submodule vector, of type `moduleType` (which may be a
// compound module), wired between this classifier's output and a downstream aggregator
// submodule (`aggregatorSubmoduleName`, a push multiplexer by default). The aggregator may also
// be a pull scheduler; one that has to take notice of an input appearing at runtime learns
// about it from the model change notification of the connection being made, so no extra
// contract is needed between the two.
//
// The submodule vector must be declared in the enclosing compound module, where it may be
// empty; the classifier extends it as branches are created. A branch is created with its final
// name and index, so parameter assignments (both from the enclosing NED declaration and from
// the ini file), display string configuration and result recording all address it as
// `<vector name>[k]`.
//
simple DynamicClassifier extends PacketClassifier
{
    parameters:
        string moduleType; // NED type of the per-class branch module, may be a compound module
        string submoduleName; // Name of the submodule vector that holds the branches
        string aggregatorSubmoduleName = default("multiplexer"); // Name of the downstream aggregator submodule the branches are wired into
        @class(DynamicClassifier);
}
