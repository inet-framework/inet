//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//
// A dummy LTE/NR cellular model on the INET radio medium. It shows how carrier aggregation,
// per-allocation transmissions, a (symbol, PRB) power spectral density lattice and shared
// panel antennas map onto INET's radio medium architecture. The physics, the scheduling and
// the error model are deliberately simple; the event structure is realistic.
//
package inet.prototype.cellular;

@namespace(inet::cellular);
