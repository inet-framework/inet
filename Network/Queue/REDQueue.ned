//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// RED (Random Early Detection) queue, to be used in routers' network
// interfaces. Conforms to the OutputQueue interface.
//
// Implements the original RED algorithm from
// "Random Early Detection Gateways for Congestion Avoidance" by
// Sally Floyd and Van Jacobson, 1993.
//
// The code is based on the algorithm described on p10 of the paper:
//
// <pre>
// Initialization:
//    avg <- 0
//    count <- -1
// for each packet arrival
//    calculate the new average queue size avg:
//        if the queue is nonempty
//            avg <- (1-wq)*avg + wq*q
//        else
//            m <- f(time-q_time)
//            avg <- (1-wq)^m * avg
//    if minth <= avg < maxth
//        increment count
//        calculate probability pa:
//            pb <- maxp*(avg-minth) / (maxth-minth)
//            pa <- pb / (1-count*pb)
//        with probability pa:
//            mark the arriving packet
//            count <- 0
//    else if maxth <= avg
//        mark the arriving packet
//        count <- 0
//    else count <- -1
// when queue becomes empty
//    q_time <- time
//
// Saved Variables:
//    - avg: average queue size
//    - q_time: start of the queue idle time
//    - count: packets since last marked packet
//
// Fixed parameters:
//    - wq: queue weight
//    - minth: minimum threshold for queue
//    - maxth: maximum threshold for queue
//    - maxp: maximum value for pb
//
// Other:
//    - pa: current packet-marking probability
//    - q: current queue size
//    - time: current time
//    - f(t): a linear function of the time t
// </pre>
//
// The function f() is supposed to estimate the number of packets
// that could have arrived during the idle interval.
// We use <i>f(t) = pkrate * t</i>, where <i>pkrate = 1/s</i>,
// s being the typical transmission time for a small packet
// as mentioned in Section 11 of the paper.
//
// Example parameter values: wq = 0.002, minth = 5 packets, maxth = 15 packets,
// maxp = 1/50 = 0.02 and pkrate = 12000 (~1K packets on 100Mbps link).
//
simple REDQueue
    parameters:
        wq: numeric const,    // queue weight
        minth: numeric const, // minimum threshold for avg queue length
        maxth: numeric const, // maximum threshold for avg queue length (=buffer capacity)
        maxp: numeric const,  // maximum value for pb
        pkrate: numeric const;  // arrivals per sec (see comment above)
    gates:
        in: in;
        out: out;
endsimple



