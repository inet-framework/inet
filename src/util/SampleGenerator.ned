//
// Copyright (C) 2010-2011 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.util;


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
        @display("i=block/source");
		@signal[sample](type="double");	// sample value from a given distribution
		@statistic[sample](title="sample"; record=stats,vector?,percentile?);

        int numSamples = default(1000);	// number of samples to generate
        volatile double distribution;
}


