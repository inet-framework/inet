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

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.mpls.RsvpMplsRouter;


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
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        LSR0: RsvpMplsRouter {
            parameters:
        }
        LSR1: RsvpMplsRouter {
            parameters:
        }
        LSR2: RsvpMplsRouter {
            parameters:
        }
        LSR3: RsvpMplsRouter {
            parameters:
        }
        LSR4: RsvpMplsRouter {
            parameters:
        }
        LSR5: RsvpMplsRouter {
            parameters:
        }
        LSR6: RsvpMplsRouter {
            parameters:
        }
        LSR7: RsvpMplsRouter {
            parameters:
        }
        LSR8: RsvpMplsRouter {
            parameters:
        }
        LSR9: RsvpMplsRouter {
            parameters:
        }
        LSR10: RsvpMplsRouter {
            parameters:
        }
        LSR11: RsvpMplsRouter {
            parameters:
        }
        LSR12: RsvpMplsRouter {
            parameters:
        }
        LSR13: RsvpMplsRouter {
            parameters:
        }
        LSR14: RsvpMplsRouter {
            parameters:
        }
        LSR15: RsvpMplsRouter {
            parameters:
        }
        LSR16: RsvpMplsRouter {
            parameters:
        }
        LSR17: RsvpMplsRouter {
            parameters:
        }
        LSR18: RsvpMplsRouter {
            parameters:
        }
        LSR19: RsvpMplsRouter {
            parameters:
        }
        LSR20: RsvpMplsRouter {
            parameters:
        }
        LSR21: RsvpMplsRouter {
            parameters:
        }
        LSR22: RsvpMplsRouter {
            parameters:
        }
        LSR23: RsvpMplsRouter {
            parameters:
        }
        LSR24: RsvpMplsRouter {
            parameters:
        }
        LSR25: RsvpMplsRouter {
            parameters:
        }
        LSR26: RsvpMplsRouter {
            parameters:
        }
        LSR27: RsvpMplsRouter {
            parameters:
        }
        LSR28: RsvpMplsRouter {
            parameters:
        }
        LSR29: RsvpMplsRouter {
            parameters:
        }
        LSR30: RsvpMplsRouter {
            parameters:
        }
        LSR31: RsvpMplsRouter {
            parameters:
        }
        LSR32: RsvpMplsRouter {
            parameters:
        }
        LSR33: RsvpMplsRouter {
            parameters:
        }
        LSR34: RsvpMplsRouter {
            parameters:
        }
        LSR35: RsvpMplsRouter {
            parameters:
        }
        LSR36: RsvpMplsRouter {
            parameters:
        }
        host1: StandardHost {
            parameters:
                @display("i=device/laptop");
        }
        host2: StandardHost {
            parameters:
                @display("i=device/laptop");

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

