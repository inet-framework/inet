//
// Copyright (C) 2010-2011 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.examples.test.distrib_test;

import inet.util.SampleGenerator;


//
// A simple network for distribution test which consists of a sample generator.
//
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2010-03-24
//  
network DistributionTest
{
    @display("bgb=100,100");
    submodules:
        gen: SampleGenerator {
            parameters:
                @display("p=50,50");
        }
}


