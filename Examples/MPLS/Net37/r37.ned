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

channel net37ch
    delay 15ms;
    datarate 128000;
endchannel

module R37
    submodules:
        configurator: NetworkConfigurator;
            parameters:
                moduleTypes = "RSVP_LSR StandardHost";
            display: "p=100,100;i=block/cogwheel_s";
        channelInstaller: ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=u";
            display: "p=100,160;i=block/cogwheel_s";
        nam: NAMTrace;
            display: "p=100,220;i=old/floppy1";
        LSR0 : RSVP_LSR;  display: "p=391,225;i=abstract/router_s";
        LSR1 : RSVP_LSR;  display: "p=799,589;i=abstract/router_s";
        LSR2 : RSVP_LSR;  display: "p=289,472;i=abstract/router_s";
        LSR3 : RSVP_LSR;  display: "p=712,418;i=abstract/router_s";
        LSR4 : RSVP_LSR;  display: "p=541,228;i=abstract/router_s";
        LSR5 : RSVP_LSR;  display: "p=288,169;i=abstract/router_s";
        LSR6 : RSVP_LSR;  display: "p=256,394;i=abstract/router_s";
        LSR7 : RSVP_LSR;  display: "p=380,263;i=abstract/router_s";
        LSR8 : RSVP_LSR;  display: "p=682,348;i=abstract/router_s";
        LSR9 : RSVP_LSR;  display: "p=536,142;i=abstract/router_s";
        LSR10 : RSVP_LSR;  display: "p=177,170;i=abstract/router_s";
        LSR11 : RSVP_LSR;  display: "p=424,262;i=abstract/router_s";
        LSR12 : RSVP_LSR;  display: "p=470,279;i=abstract/router_s";
        LSR13 : RSVP_LSR;  display: "p=278,91;i=abstract/router_s";
        LSR14 : RSVP_LSR;  display: "p=467,214;i=abstract/router_s";
        LSR15 : RSVP_LSR;  display: "p=724,19;i=abstract/router_s";
        LSR16 : RSVP_LSR;  display: "p=685,287;i=abstract/router_s";
        LSR17 : RSVP_LSR;  display: "p=20,519;i=abstract/router_s";
        LSR18 : RSVP_LSR;  display: "p=298,236;i=abstract/router_s";
        LSR19 : RSVP_LSR;  display: "p=390,390;i=abstract/router_s";
        LSR20 : RSVP_LSR;  display: "p=148,502;i=abstract/router_s";
        LSR21 : RSVP_LSR;  display: "p=362,448;i=abstract/router_s";
        LSR22 : RSVP_LSR;  display: "p=476,397;i=abstract/router_s";
        LSR23 : RSVP_LSR;  display: "p=517,325;i=abstract/router_s";
        LSR24 : RSVP_LSR;  display: "p=514,34;i=abstract/router_s";
        LSR25 : RSVP_LSR;  display: "p=548,599;i=abstract/router_s";
        LSR26 : RSVP_LSR;  display: "p=330,301;i=abstract/router_s";
        LSR27 : RSVP_LSR;  display: "p=612,310;i=abstract/router_s";
        LSR28 : RSVP_LSR;  display: "p=556,502;i=abstract/router_s";
        LSR29 : RSVP_LSR;  display: "p=83,570;i=abstract/router_s";
        LSR30 : RSVP_LSR;  display: "p=796,466;i=abstract/router_s";
        LSR31 : RSVP_LSR;  display: "p=610,65;i=abstract/router_s";
        LSR32 : RSVP_LSR;  display: "p=435,321;i=abstract/router_s";
        LSR33 : RSVP_LSR;  display: "p=605,337;i=abstract/router_s";
        LSR34 : RSVP_LSR;  display: "p=707,227;i=abstract/router_s";
        LSR35 : RSVP_LSR;  display: "p=613,405;i=abstract/router_s";
        LSR36 : RSVP_LSR;  display: "p=435,361;i=abstract/router_s";

        host1 : StandardHost;  display: "p=299,522;i=device/laptop_s";
        host2 : StandardHost;  display: "p=757,227;i=device/laptop_s";

    connections:
        LSR7.out++ --> net37ch --> LSR0.in++;  LSR7.in++ <-- net37ch <-- LSR0.out++; // cost 259
        LSR8.out++ --> net37ch --> LSR3.in++;  LSR8.in++ <-- net37ch <-- LSR3.out++; // cost 474
        LSR9.out++ --> net37ch --> LSR4.in++;  LSR9.in++ <-- net37ch <-- LSR4.out++; // cost 540
        LSR11.out++ --> net37ch --> LSR7.in++;  LSR11.in++ <-- net37ch <-- LSR7.out++; // cost 264
        LSR12.out++ --> net37ch --> LSR11.in++;  LSR12.in++ <-- net37ch <-- LSR11.out++; // cost 275
        LSR13.out++ --> net37ch --> LSR0.in++;  LSR13.in++ <-- net37ch <-- LSR0.out++; // cost 1067
        LSR13.out++ --> net37ch --> LSR5.in++;  LSR13.in++ <-- net37ch <-- LSR5.out++; // cost 610
        LSR13.out++ --> net37ch --> LSR10.in++;  LSR13.in++ <-- net37ch <-- LSR10.out++; // cost 462
        LSR14.out++ --> net37ch --> LSR0.in++;  LSR14.in++ <-- net37ch <-- LSR0.out++; // cost 552
        LSR14.out++ --> net37ch --> LSR4.in++;  LSR14.in++ <-- net37ch <-- LSR4.out++; // cost 381
        LSR14.out++ --> net37ch --> LSR12.in++;  LSR14.in++ <-- net37ch <-- LSR12.out++; // cost 592
        LSR16.out++ --> net37ch --> LSR8.in++;  LSR16.in++ <-- net37ch <-- LSR8.out++; // cost 436
        LSR18.out++ --> net37ch --> LSR0.in++;  LSR18.in++ <-- net37ch <-- LSR0.out++; // cost 540
        LSR18.out++ --> net37ch --> LSR5.in++;  LSR18.in++ <-- net37ch <-- LSR5.out++; // cost 239
        LSR18.out++ --> net37ch --> LSR10.in++;  LSR18.in++ <-- net37ch <-- LSR10.out++; // cost 690
        LSR18.out++ --> net37ch --> LSR17.in++;  LSR18.in++ <-- net37ch <-- LSR17.out++; // cost 1977
        LSR20.out++ --> net37ch --> LSR2.in++;  LSR20.in++ <-- net37ch <-- LSR2.out++; // cost 760
        LSR20.out++ --> net37ch --> LSR6.in++;  LSR20.in++ <-- net37ch <-- LSR6.out++; // cost 834
        LSR20.out++ --> net37ch --> LSR17.in++;  LSR20.in++ <-- net37ch <-- LSR17.out++; // cost 751
        LSR21.out++ --> net37ch --> LSR2.in++;  LSR21.in++ <-- net37ch <-- LSR2.out++; // cost 508
        LSR21.out++ --> net37ch --> LSR6.in++;  LSR21.in++ <-- net37ch <-- LSR6.out++; // cost 756
        LSR21.out++ --> net37ch --> LSR19.in++;  LSR21.in++ <-- net37ch <-- LSR19.out++; // cost 411
        LSR23.out++ --> net37ch --> LSR4.in++;  LSR23.in++ <-- net37ch <-- LSR4.out++; // cost 757
        LSR23.out++ --> net37ch --> LSR12.in++;  LSR23.in++ <-- net37ch <-- LSR12.out++; // cost 456
        LSR23.out++ --> net37ch --> LSR22.in++;  LSR23.in++ <-- net37ch <-- LSR22.out++; // cost 522
        LSR24.out++ --> net37ch --> LSR9.in++;  LSR24.in++ <-- net37ch <-- LSR9.out++; // cost 722
        LSR24.out++ --> net37ch --> LSR15.in++;  LSR24.in++ <-- net37ch <-- LSR15.out++; // cost 1182
        LSR25.out++ --> net37ch --> LSR1.in++;  LSR25.in++ <-- net37ch <-- LSR1.out++; // cost 1363
        LSR26.out++ --> net37ch --> LSR6.in++;  LSR26.in++ <-- net37ch <-- LSR6.out++; // cost 747
        LSR26.out++ --> net37ch --> LSR7.in++;  LSR26.in++ <-- net37ch <-- LSR7.out++; // cost 393
        LSR26.out++ --> net37ch --> LSR18.in++;  LSR26.in++ <-- net37ch <-- LSR18.out++; // cost 514
        LSR26.out++ --> net37ch --> LSR19.in++;  LSR26.in++ <-- net37ch <-- LSR19.out++; // cost 594
        LSR27.out++ --> net37ch --> LSR4.in++;  LSR27.in++ <-- net37ch <-- LSR4.out++; // cost 420
        LSR27.out++ --> net37ch --> LSR8.in++;  LSR27.in++ <-- net37ch <-- LSR8.out++; // cost 668
        LSR28.out++ --> net37ch --> LSR21.in++;  LSR28.in++ <-- net37ch <-- LSR21.out++; // cost 907
        LSR28.out++ --> net37ch --> LSR22.in++;  LSR28.in++ <-- net37ch <-- LSR22.out++; // cost 720
        LSR28.out++ --> net37ch --> LSR25.in++;  LSR28.in++ <-- net37ch <-- LSR25.out++; // cost 638
        LSR29.out++ --> net37ch --> LSR2.in++;  LSR29.in++ <-- net37ch <-- LSR2.out++; // cost 1244
        LSR29.out++ --> net37ch --> LSR17.in++;  LSR29.in++ <-- net37ch <-- LSR17.out++; // cost 471
        LSR30.out++ --> net37ch --> LSR1.in++;  LSR30.in++ <-- net37ch <-- LSR1.out++; // cost 794
        LSR30.out++ --> net37ch --> LSR3.in++;  LSR30.in++ <-- net37ch <-- LSR3.out++; // cost 487
        LSR31.out++ --> net37ch --> LSR9.in++;  LSR31.in++ <-- net37ch <-- LSR9.out++; // cost 777
        LSR31.out++ --> net37ch --> LSR15.in++;  LSR31.in++ <-- net37ch <-- LSR15.out++; // cost 597
        LSR32.out++ --> net37ch --> LSR12.in++;  LSR32.in++ <-- net37ch <-- LSR12.out++; // cost 271
        LSR32.out++ --> net37ch --> LSR26.in++;  LSR32.in++ <-- net37ch <-- LSR26.out++; // cost 600
        LSR33.out++ --> net37ch --> LSR23.in++;  LSR33.in++ <-- net37ch <-- LSR23.out++; // cost 534
        LSR33.out++ --> net37ch --> LSR27.in++;  LSR33.in++ <-- net37ch <-- LSR27.out++; // cost 376
        LSR34.out++ --> net37ch --> LSR4.in++;  LSR34.in++ <-- net37ch <-- LSR4.out++; // cost 775
        LSR34.out++ --> net37ch --> LSR15.in++;  LSR34.in++ <-- net37ch <-- LSR15.out++; // cost 1370
        LSR34.out++ --> net37ch --> LSR16.in++;  LSR34.in++ <-- net37ch <-- LSR16.out++; // cost 383
        LSR35.out++ --> net37ch --> LSR1.in++;  LSR35.in++ <-- net37ch <-- LSR1.out++; // cost 1500
        LSR35.out++ --> net37ch --> LSR3.in++;  LSR35.in++ <-- net37ch <-- LSR3.out++; // cost 551
        LSR35.out++ --> net37ch --> LSR28.in++;  LSR35.in++ <-- net37ch <-- LSR28.out++; // cost 783
        LSR35.out++ --> net37ch --> LSR33.in++;  LSR35.in++ <-- net37ch <-- LSR33.out++; // cost 400
        LSR36.out++ --> net37ch --> LSR19.in++;  LSR36.in++ <-- net37ch <-- LSR19.out++; // cost 507
        LSR36.out++ --> net37ch --> LSR22.in++;  LSR36.in++ <-- net37ch <-- LSR22.out++; // cost 327
        LSR36.out++ --> net37ch --> LSR32.in++;  LSR36.in++ <-- net37ch <-- LSR32.out++; // cost 218

        host1.out++ --> net37ch --> LSR2.in++; host1.in++ <-- net37ch <-- LSR2.out++;
        host2.out++ --> net37ch --> LSR34.in++; host2.in++ <-- net37ch <-- LSR34.out++;

endmodule

network r37 : R37
endnetwork

