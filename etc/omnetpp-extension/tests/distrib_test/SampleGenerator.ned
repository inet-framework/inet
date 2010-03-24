//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


//
// Generates samples from a given probability distribution.
//
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2010-03-24
//  
simple SampleGenerator
{
    parameters:
        int numSamples = default(1000);	// number of samples to generate
        volatile double distribution;
        @display("i=block/source");
}


