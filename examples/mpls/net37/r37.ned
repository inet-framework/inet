//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        LSR0: RsvpMplsRouter;
        LSR1: RsvpMplsRouter;
        LSR2: RsvpMplsRouter;
        LSR3: RsvpMplsRouter;
        LSR4: RsvpMplsRouter;
        LSR5: RsvpMplsRouter;
        LSR6: RsvpMplsRouter;
        LSR7: RsvpMplsRouter;
        LSR8: RsvpMplsRouter;
        LSR9: RsvpMplsRouter;
        LSR10: RsvpMplsRouter;
        LSR11: RsvpMplsRouter;
        LSR12: RsvpMplsRouter;
        LSR13: RsvpMplsRouter;
        LSR14: RsvpMplsRouter;
        LSR15: RsvpMplsRouter;
        LSR16: RsvpMplsRouter;
        LSR17: RsvpMplsRouter;
        LSR18: RsvpMplsRouter;
        LSR19: RsvpMplsRouter;
        LSR20: RsvpMplsRouter;
        LSR21: RsvpMplsRouter;
        LSR22: RsvpMplsRouter;
        LSR23: RsvpMplsRouter;
        LSR24: RsvpMplsRouter;
        LSR25: RsvpMplsRouter;
        LSR26: RsvpMplsRouter;
        LSR27: RsvpMplsRouter;
        LSR28: RsvpMplsRouter;
        LSR29: RsvpMplsRouter;
        LSR30: RsvpMplsRouter;
        LSR31: RsvpMplsRouter;
        LSR32: RsvpMplsRouter;
        LSR33: RsvpMplsRouter;
        LSR34: RsvpMplsRouter;
        LSR35: RsvpMplsRouter;
        LSR36: RsvpMplsRouter;
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

