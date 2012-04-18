//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

package inet.examples.mpls.net37;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.StandardHost;
import inet.nodes.mpls.RSVP_LSR;
import inet.util.ThruputMeteringChannel;


network R37
{
    types:
        channel line extends ThruputMeteringChannel
        {
            delay = 15ms;
            datarate = 128kbps;
            thruputDisplayFormat = "u";
        }
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,100");
        }
        LSR0: RSVP_LSR {
            parameters:
                @display("p=391,225");
        }
        LSR1: RSVP_LSR {
            parameters:
                @display("p=799,589");
        }
        LSR2: RSVP_LSR {
            parameters:
                @display("p=289,472");
        }
        LSR3: RSVP_LSR {
            parameters:
                @display("p=712,418");
        }
        LSR4: RSVP_LSR {
            parameters:
                @display("p=541,228");
        }
        LSR5: RSVP_LSR {
            parameters:
                @display("p=288,169");
        }
        LSR6: RSVP_LSR {
            parameters:
                @display("p=256,394");
        }
        LSR7: RSVP_LSR {
            parameters:
                @display("p=380,263");
        }
        LSR8: RSVP_LSR {
            parameters:
                @display("p=682,348");
        }
        LSR9: RSVP_LSR {
            parameters:
                @display("p=536,142");
        }
        LSR10: RSVP_LSR {
            parameters:
                @display("p=177,170");
        }
        LSR11: RSVP_LSR {
            parameters:
                @display("p=424,262");
        }
        LSR12: RSVP_LSR {
            parameters:
                @display("p=470,279");
        }
        LSR13: RSVP_LSR {
            parameters:
                @display("p=278,91");
        }
        LSR14: RSVP_LSR {
            parameters:
                @display("p=467,214");
        }
        LSR15: RSVP_LSR {
            parameters:
                @display("p=724,19");
        }
        LSR16: RSVP_LSR {
            parameters:
                @display("p=685,287");
        }
        LSR17: RSVP_LSR {
            parameters:
                @display("p=20,519");
        }
        LSR18: RSVP_LSR {
            parameters:
                @display("p=298,236");
        }
        LSR19: RSVP_LSR {
            parameters:
                @display("p=390,390");
        }
        LSR20: RSVP_LSR {
            parameters:
                @display("p=148,502");
        }
        LSR21: RSVP_LSR {
            parameters:
                @display("p=362,448");
        }
        LSR22: RSVP_LSR {
            parameters:
                @display("p=476,397");
        }
        LSR23: RSVP_LSR {
            parameters:
                @display("p=517,325");
        }
        LSR24: RSVP_LSR {
            parameters:
                @display("p=514,34");
        }
        LSR25: RSVP_LSR {
            parameters:
                @display("p=548,599");
        }
        LSR26: RSVP_LSR {
            parameters:
                @display("p=330,301");
        }
        LSR27: RSVP_LSR {
            parameters:
                @display("p=612,310");
        }
        LSR28: RSVP_LSR {
            parameters:
                @display("p=556,502");
        }
        LSR29: RSVP_LSR {
            parameters:
                @display("p=83,570");
        }
        LSR30: RSVP_LSR {
            parameters:
                @display("p=796,466");
        }
        LSR31: RSVP_LSR {
            parameters:
                @display("p=610,65");
        }
        LSR32: RSVP_LSR {
            parameters:
                @display("p=435,321");
        }
        LSR33: RSVP_LSR {
            parameters:
                @display("p=605,337");
        }
        LSR34: RSVP_LSR {
            parameters:
                @display("p=707,227");
        }
        LSR35: RSVP_LSR {
            parameters:
                @display("p=613,405");
        }
        LSR36: RSVP_LSR {
            parameters:
                @display("p=435,361");

        }
        host1: StandardHost {
            parameters:
                @display("p=299,522;i=device/laptop_s");
        }
        host2: StandardHost {
            parameters:
                @display("p=757,227;i=device/laptop_s");

        }
    connections:
        LSR7.pppg++ <--> line <--> LSR0.pppg++;   // cost 259
        LSR8.pppg++ <--> line <--> LSR3.pppg++;   // cost 474
        LSR9.pppg++ <--> line <--> LSR4.pppg++;   // cost 540
        LSR11.pppg++ <--> line <--> LSR7.pppg++;  // cost 264
        LSR12.pppg++ <--> line <--> LSR11.pppg++; // cost 275
        LSR13.pppg++ <--> line <--> LSR0.pppg++;  // cost 1067
        LSR13.pppg++ <--> line <--> LSR5.pppg++;  // cost 610
        LSR13.pppg++ <--> line <--> LSR10.pppg++; // cost 462
        LSR14.pppg++ <--> line <--> LSR0.pppg++;  // cost 552
        LSR14.pppg++ <--> line <--> LSR4.pppg++;  // cost 381
        LSR14.pppg++ <--> line <--> LSR12.pppg++; // cost 592
        LSR16.pppg++ <--> line <--> LSR8.pppg++;  // cost 436
        LSR18.pppg++ <--> line <--> LSR0.pppg++;  // cost 540
        LSR18.pppg++ <--> line <--> LSR5.pppg++;  // cost 239
        LSR18.pppg++ <--> line <--> LSR10.pppg++; // cost 690
        LSR18.pppg++ <--> line <--> LSR17.pppg++; // cost 1977
        LSR20.pppg++ <--> line <--> LSR2.pppg++;  // cost 760
        LSR20.pppg++ <--> line <--> LSR6.pppg++;  // cost 834
        LSR20.pppg++ <--> line <--> LSR17.pppg++; // cost 751
        LSR21.pppg++ <--> line <--> LSR2.pppg++;  // cost 508
        LSR21.pppg++ <--> line <--> LSR6.pppg++;  // cost 756
        LSR21.pppg++ <--> line <--> LSR19.pppg++; // cost 411
        LSR23.pppg++ <--> line <--> LSR4.pppg++;  // cost 757
        LSR23.pppg++ <--> line <--> LSR12.pppg++; // cost 456
        LSR23.pppg++ <--> line <--> LSR22.pppg++; // cost 522
        LSR24.pppg++ <--> line <--> LSR9.pppg++;  // cost 722
        LSR24.pppg++ <--> line <--> LSR15.pppg++; // cost 1182
        LSR25.pppg++ <--> line <--> LSR1.pppg++;  // cost 1363
        LSR26.pppg++ <--> line <--> LSR6.pppg++;  // cost 747
        LSR26.pppg++ <--> line <--> LSR7.pppg++;  // cost 393
        LSR26.pppg++ <--> line <--> LSR18.pppg++; // cost 514
        LSR26.pppg++ <--> line <--> LSR19.pppg++; // cost 594
        LSR27.pppg++ <--> line <--> LSR4.pppg++;  // cost 420
        LSR27.pppg++ <--> line <--> LSR8.pppg++;  // cost 668
        LSR28.pppg++ <--> line <--> LSR21.pppg++; // cost 907
        LSR28.pppg++ <--> line <--> LSR22.pppg++; // cost 720
        LSR28.pppg++ <--> line <--> LSR25.pppg++; // cost 638
        LSR29.pppg++ <--> line <--> LSR2.pppg++;  // cost 1244
        LSR29.pppg++ <--> line <--> LSR17.pppg++; // cost 471
        LSR30.pppg++ <--> line <--> LSR1.pppg++;  // cost 794
        LSR30.pppg++ <--> line <--> LSR3.pppg++;  // cost 487
        LSR31.pppg++ <--> line <--> LSR9.pppg++;  // cost 777
        LSR31.pppg++ <--> line <--> LSR15.pppg++; // cost 597
        LSR32.pppg++ <--> line <--> LSR12.pppg++; // cost 271
        LSR32.pppg++ <--> line <--> LSR26.pppg++; // cost 600
        LSR33.pppg++ <--> line <--> LSR23.pppg++; // cost 534
        LSR33.pppg++ <--> line <--> LSR27.pppg++; // cost 376
        LSR34.pppg++ <--> line <--> LSR4.pppg++;  // cost 775
        LSR34.pppg++ <--> line <--> LSR15.pppg++; // cost 1370
        LSR34.pppg++ <--> line <--> LSR16.pppg++; // cost 383
        LSR35.pppg++ <--> line <--> LSR1.pppg++;  // cost 1500
        LSR35.pppg++ <--> line <--> LSR3.pppg++;  // cost 551
        LSR35.pppg++ <--> line <--> LSR28.pppg++; // cost 783
        LSR35.pppg++ <--> line <--> LSR33.pppg++; // cost 400
        LSR36.pppg++ <--> line <--> LSR19.pppg++; // cost 507
        LSR36.pppg++ <--> line <--> LSR22.pppg++; // cost 327
        LSR36.pppg++ <--> line <--> LSR32.pppg++; // cost 218

        host1.pppg++ <--> line <--> LSR2.pppg++;
        host2.pppg++ <--> line <--> LSR34.pppg++;

}

