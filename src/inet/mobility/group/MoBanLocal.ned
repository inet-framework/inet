//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//***************************************************************************
//* file:        MoBanLocal.ned
//*
//* author:      Majid Nabi <m.nabi@tue.nl>
//*
//*              http://www.es.ele.tue.nl/nes
//*
//*
//* copyright:   (C) 2010 Electronic Systems group(ES),
//*              Eindhoven University of Technology (TU/e), the Netherlands.
//*
//*
//***************************************************************************
//* part of:    MoBAN (Mobility Model for wireless Body Area Networks)
//* description:     Local module of the MoBAN mobility model
//***************************************************************************
//* Citation of the following publication is appreciated if you use MoBAN for
//* a publication of your own.
//*
//* M. Nabi, M. Geilen, T. Basten. MoBAN: A Configurable Mobility Model for Wireless Body Area Networks.
//* In Proc. of the 4th Int'l Conf. on Simulation Tools and Techniques, SIMUTools 2011, Barcelona, Spain, 2011.
//*
//* BibTeX:
//*    @inproceedings{MoBAN,
//*     author = "M. Nabi and M. Geilen and T. Basten.",
//*     title = "{MoBAN}: A Configurable Mobility Model for Wireless Body Area Networks.",
//*        booktitle = "Proceedings of the 4th Int'l Conf. on Simulation Tools and Techniques.",
//*        series = {SIMUTools '11},
//*        isbn = {978-963-9799-41-7},
//*    year = {2011},
//*        location = {Barcelona, Spain},
//*    publisher = {ICST} }
//*
//**************************************************************************/

package inet.mobility.group;

import inet.mobility.base.MovingMobilityBase;

// This is the local mobility module of MoBAN. It should be instantiated in each node that belongs to a WBAN. The NED parameter "coordinatorIndex"
// determine to which WBAN (MoBanCoordinator) it belongs.
// The current implementation uses the Random Walk Mobility Model (RWMM) for individual (local) movement with a sphere around the node, with given speed
// and sphere radius of the current posture. The reference point of the node it the current posture, the sphere radius, and the speed is given by the
// corresponding coordinator. RWMM determines the location of node at ant time relative to the given reference point.
simple MoBanLocal extends MovingMobilityBase
{
    parameters:
        int coordinatorIndex; // The index of the belonging coordinator module
        @class(MoBanLocal);
}

