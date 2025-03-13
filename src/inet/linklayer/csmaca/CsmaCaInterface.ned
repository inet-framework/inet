//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.csmaca;

import inet.linklayer.common.WirelessInterface;

//
// Provides a complete wireless network interface that uses the CSMA/CA protocol
// for medium access control. This interface combines the CSMA/CA MAC protocol
// with appropriate radio and queue components to create a ready-to-use network
// interface.
//
// The CSMA/CA (Carrier Sense Multiple Access with Collision Avoidance) protocol
// is widely used in wireless networks to manage shared medium access. It works by
// sensing the channel before transmission and using random backoff mechanisms to
// reduce collision probability.
//
// The interface uses:
// - CsmaCaMac: A configurable implementation of the CSMA/CA protocol with optional
//   acknowledgments and retry mechanisms
// - ApskRadio: Default radio model for wireless transmission
// - DropTailQueue: Simple FIFO queue with configurable capacity
//
// This interface can be used for general wireless network simulations and can
// approximate basic 802.11b ad-hoc mode operation with appropriate settings.
//
// @see ~CsmaCaMac, ~WirelessInterface
//
module CsmaCaInterface extends WirelessInterface
{
    parameters:
        radio.typename = default("ApskRadio");
        mac.typename = default("CsmaCaMac");
        queue.typename = default("DropTailQueue");
        queue.packetCapacity = default(-1);
}
