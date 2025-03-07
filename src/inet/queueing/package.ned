//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Contains modules for building packet queueing models in the INET Framework.
// The queueing framework provides reusable modules for various packet processing tasks
// such as buffering, filtering, scheduling, classifying, dropping, marking, and delaying.
//
// Key features:
// - Modular architecture with composable elements
// - Support for both push and pull packet processing modes
// - Extensive collection of queueing disciplines
// - Traffic conditioning and shaping capabilities
// - Packet marking and classification for QoS
// - Token bucket and leaky bucket implementations
// - RED and other active queue management techniques
//
package inet.queueing;

@namespace(inet::queueing);
