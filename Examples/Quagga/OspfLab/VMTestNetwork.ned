import
  "Router",
  "QuaggaRouter",
  "StandardHost",
  "FailureManager",
  "ScenarioManager",
  "NetworkInfo",
  "EtherHub";

module VMTestNetwork
  submodules:
    failureManager: FailureManager;
      display: "p=150,50;i=block/cogwheel_s";
    scenarioManager: ScenarioManager;
      display: "p=250,50;i=block/cogwheel_s";
    networkInfo: NetworkInfo;
      display: "p=350,50;i=block/cogwheel_s";
    RRC1: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 24;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=350,450;i=device/router";
    PC03: StandardHost;
      parameters:
        namid = 21;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=450,150;i=device/pc2";
    SC: StandardHost;
      parameters:
        namid = 16;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=950,450;i=device/pc2";
    RRB1: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 17;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=400,250;i=device/router";
    RB1: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 1;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=650,400;i=device/router";
    RB4: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 4;
      gatesizes:
        ethIn[4],
        ethOut[4];
      display: "p=570,500;i=device/router";
    RRF2: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 46;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=650,620;i=device/router";
    RC4: StandardHost;
      parameters:
        namid = 15;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=850,450;i=device/pc2";
    PC01: StandardHost;
      parameters:
        namid = 9;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=800,150;i=device/pc2";
    rout11: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 48;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=680,710;i=device/router";
    rout06: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 29;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=200,400;i=device/router";
    RRF1: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 45;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=630,560;i=device/router";
    PC10: StandardHost;
      parameters:
        namid = 44;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=650,780;i=device/pc2";
    PC02: StandardHost;
      parameters:
        namid = 11;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=500,100;i=device/pc2";
    PC09: StandardHost;
      parameters:
        namid = 42;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=400,780;i=device/pc2";
    rout07: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 34;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=325,625;i=device/router";
    SRB: StandardHost;
      parameters:
        namid = 19;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=300,300;i=device/pc2";
    rout10: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 43;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=600,750;i=device/router";
    RRD1: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 31;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=475,500;i=device/router";
    PC07: StandardHost;
      parameters:
        namid = 35;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=225,600;i=device/pc2";
    RRE2: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 39;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=525,720;i=device/router";
    rout05: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 27;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=150,300;i=device/router";
    PC12: StandardHost;
      parameters:
        namid = 51;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=850,700;i=device/pc2";
    rout12: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 50;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=780,660;i=device/router";
    SRC: StandardHost;
      parameters:
        namid = 26;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=400,350;i=device/pc2";
    PC11: StandardHost;
      parameters:
        namid = 49;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=750,740;i=device/pc2";
    rout08: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 36;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=350,700;i=device/router";
    PC05: StandardHost;
      parameters:
        namid = 28;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=100,350;i=device/pc2";
    RB2: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 2;
      gatesizes:
        ethIn[4],
        ethOut[4];
      display: "p=550,380;i=device/router";
    rout01: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 8;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=700,170;i=device/router";
    rout09: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 41;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=450,750;i=device/router";
    SRA: StandardHost;
      parameters:
        namid = 7;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=500,265;i=device/pc2";
    rout04: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 22;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=150,150;i=device/router";
    PC08: StandardHost;
      parameters:
        namid = 37;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=300,750;i=device/pc2";
    RRB2: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 18;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=250,200;i=device/router";
    RRC2: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 25;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=250,350;i=device/router";
    SRF: StandardHost;
      parameters:
        namid = 47;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=720,580;i=device/pc2";
    PC04: StandardHost;
      parameters:
        namid = 23;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=50,150;i=device/pc2";
    SRE: StandardHost;
      parameters:
        namid = 40;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=450,650;i=device/pc2";
    RC1: StandardHost;
      parameters:
        namid = 12;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=700,300;i=device/pc2";
    RRA1: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 5;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=600,330;i=device/router";
    SRD: StandardHost;
      parameters:
        namid = 33;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=325,550;i=device/pc2";
    RC2: StandardHost;
      parameters:
        namid = 13;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=800,300;i=device/pc2";
    RB3: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 3;
      gatesizes:
        ethIn[4],
        ethOut[4];
      display: "p=500,420;i=device/router";
    rout03: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 20;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=350,100;i=device/router";
    PC06: StandardHost;
      parameters:
        namid = 30;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=150,450;i=device/pc2";
    RRD2: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 32;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=425,580;i=device/router";
    RC3: StandardHost;
      parameters:
        namid = 14;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=750,450;i=device/pc2";
    RRE1: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 38;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=500,620;i=device/router";
    rout02: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 10;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=550,150;i=device/router";
    RRA2: QuaggaRouter;
      parameters:
        routingDaemon = "Ospfd",
        namid = 6;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=600,200;i=device/router";
    hub1: EtherHub;
      gatesizes:
        in[1],
        out[1];
      display: "p=900,500;i=device/switch";
    hub2: EtherHub;
      gatesizes:
        in[3],
        out[3];
      display: "p=325,200;i=device/switch";
    hub3: EtherHub;
      gatesizes:
        in[1],
        out[1];
      display: "p=800,500;i=device/switch";
    hub4: EtherHub;
      gatesizes:
        in[3],
        out[3];
      display: "p=600,265;i=device/switch";
    hub5: EtherHub;
      gatesizes:
        in[1],
        out[1];
      display: "p=750,250;i=device/switch";
    hub6: EtherHub;
      gatesizes:
        in[3],
        out[3];
      display: "p=330,380;i=device/switch";
    hub7: EtherHub;
      gatesizes:
        in[6],
        out[6];
      display: "p=850,400;i=device/switch";
    hub8: EtherHub;
      gatesizes:
        in[1],
        out[1];
      display: "p=850,250;i=device/switch";
    hub9: EtherHub;
      gatesizes:
        in[3],
        out[3];
      display: "p=375,500;i=device/switch";
    hub10: EtherHub;
      gatesizes:
        in[3],
        out[3];
      display: "i=device/switch";
    hub11: EtherHub;
      gatesizes:
        in[3],
        out[3];
      display: "p=690,510;i=device/switch";
  connections nocheck:
    RRD2.ethOut[1] --> delay 0.0001s --> rout07.ethIn[0];
    RRD2.ethIn[1] <-- delay 0.0001s <-- rout07.ethOut[0];
    hub1.out[0] --> delay 0.0001s --> RC4.ethIn[1];
    hub1.in[0] <-- delay 0.0001s <-- RC4.ethOut[1];
    hub2.out[0] --> delay 0.0001s --> RRB1.ethIn[1];
    hub2.in[0] <-- delay 0.0001s <-- RRB1.ethOut[1];
    hub2.out[1] --> delay 0.0001s --> RRB2.ethIn[0];
    hub2.in[1] <-- delay 0.0001s <-- RRB2.ethOut[0];
    hub2.out[2] --> delay 0.0001s --> SRB.ethIn[0];
    hub2.in[2] <-- delay 0.0001s <-- SRB.ethOut[0];
    rout01.ethOut[1] --> delay 0.0001s --> PC01.ethIn[0];
    rout01.ethIn[1] <-- delay 0.0001s <-- PC01.ethOut[0];
    RB1.ethOut[0] --> delay 0.0001s --> RB2.ethIn[0];
    RB1.ethIn[0] <-- delay 0.0001s <-- RB2.ethOut[0];
    rout08.ethOut[1] --> delay 0.0001s --> PC08.ethIn[0];
    rout08.ethIn[1] <-- delay 0.0001s <-- PC08.ethOut[0];
    hub3.out[0] --> delay 0.0001s --> RC3.ethIn[1];
    hub3.in[0] <-- delay 0.0001s <-- RC3.ethOut[1];
    hub4.out[0] --> delay 0.0001s --> RRA1.ethIn[1];
    hub4.in[0] <-- delay 0.0001s <-- RRA1.ethOut[1];
    hub4.out[1] --> delay 0.0001s --> RRA2.ethIn[0];
    hub4.in[1] <-- delay 0.0001s <-- RRA2.ethOut[0];
    hub4.out[2] --> delay 0.0001s --> SRA.ethIn[0];
    hub4.in[2] <-- delay 0.0001s <-- SRA.ethOut[0];
    RRE2.ethOut[2] --> delay 0.0001s --> rout10.ethIn[0];
    RRE2.ethIn[2] <-- delay 0.0001s <-- rout10.ethOut[0];
    rout02.ethOut[1] --> delay 0.0001s --> PC02.ethIn[0];
    rout02.ethIn[1] <-- delay 0.0001s <-- PC02.ethOut[0];
    RRA2.ethOut[2] --> delay 0.0001s --> rout02.ethIn[0];
    RRA2.ethIn[2] <-- delay 0.0001s <-- rout02.ethOut[0];
    rout10.ethOut[1] --> delay 0.0001s --> PC10.ethIn[0];
    rout10.ethIn[1] <-- delay 0.0001s <-- PC10.ethOut[0];
    RB2.ethOut[3] --> delay 0.0001s --> RRB1.ethIn[0];
    RB2.ethIn[3] <-- delay 0.0001s <-- RRB1.ethOut[0];
    hub5.out[0] --> delay 0.0001s --> RC1.ethIn[1];
    hub5.in[0] <-- delay 0.0001s <-- RC1.ethOut[1];
    RB1.ethOut[1] --> delay 0.0001s --> RB4.ethIn[1];
    RB1.ethIn[1] <-- delay 0.0001s <-- RB4.ethOut[1];
    RRE2.ethOut[1] --> delay 0.0001s --> rout09.ethIn[0];
    RRE2.ethIn[1] <-- delay 0.0001s <-- rout09.ethOut[0];
    hub6.out[0] --> delay 0.0001s --> RRC1.ethIn[1];
    hub6.in[0] <-- delay 0.0001s <-- RRC1.ethOut[1];
    hub6.out[1] --> delay 0.0001s --> RRC2.ethIn[0];
    hub6.in[1] <-- delay 0.0001s <-- RRC2.ethOut[0];
    hub6.out[2] --> delay 0.0001s --> SRC.ethIn[0];
    hub6.in[2] <-- delay 0.0001s <-- SRC.ethOut[0];
    RRC2.ethOut[2] --> delay 0.0001s --> rout06.ethIn[0];
    RRC2.ethIn[2] <-- delay 0.0001s <-- rout06.ethOut[0];
    RRF2.ethOut[1] --> delay 0.0001s --> rout11.ethIn[0];
    RRF2.ethIn[1] <-- delay 0.0001s <-- rout11.ethOut[0];
    RRB2.ethOut[2] --> delay 0.0001s --> rout04.ethIn[0];
    RRB2.ethIn[2] <-- delay 0.0001s <-- rout04.ethOut[0];
    RB3.ethOut[3] --> delay 0.0001s --> RRD1.ethIn[0];
    RB3.ethIn[3] <-- delay 0.0001s <-- RRD1.ethOut[0];
    RRA2.ethOut[1] --> delay 0.0001s --> rout01.ethIn[0];
    RRA2.ethIn[1] <-- delay 0.0001s <-- rout01.ethOut[0];
    rout12.ethOut[1] --> delay 0.0001s --> PC12.ethIn[0];
    rout12.ethIn[1] <-- delay 0.0001s <-- PC12.ethOut[0];
    rout11.ethOut[1] --> delay 0.0001s --> PC11.ethIn[0];
    rout11.ethIn[1] <-- delay 0.0001s <-- PC11.ethOut[0];
    hub7.out[0] --> delay 0.0001s --> RB1.ethIn[2];
    hub7.in[0] <-- delay 0.0001s <-- RB1.ethOut[2];
    hub7.out[1] --> delay 0.0001s --> RC1.ethIn[0];
    hub7.in[1] <-- delay 0.0001s <-- RC1.ethOut[0];
    hub7.out[2] --> delay 0.0001s --> RC2.ethIn[0];
    hub7.in[2] <-- delay 0.0001s <-- RC2.ethOut[0];
    hub7.out[3] --> delay 0.0001s --> RC3.ethIn[0];
    hub7.in[3] <-- delay 0.0001s <-- RC3.ethOut[0];
    hub7.out[4] --> delay 0.0001s --> RC4.ethIn[0];
    hub7.in[4] <-- delay 0.0001s <-- RC4.ethOut[0];
    hub7.out[5] --> delay 0.0001s --> SC.ethIn[0];
    hub7.in[5] <-- delay 0.0001s <-- SC.ethOut[0];
    hub8.out[0] --> delay 0.0001s --> RC2.ethIn[1];
    hub8.in[0] <-- delay 0.0001s <-- RC2.ethOut[1];
    RRD2.ethOut[2] --> delay 0.0001s --> rout08.ethIn[0];
    RRD2.ethIn[2] <-- delay 0.0001s <-- rout08.ethOut[0];
    RB2.ethOut[1] --> delay 0.0001s --> RB3.ethIn[0];
    RB2.ethIn[1] <-- delay 0.0001s <-- RB3.ethOut[0];
    RRB2.ethOut[1] --> delay 0.0001s --> rout03.ethIn[0];
    RRB2.ethIn[1] <-- delay 0.0001s <-- rout03.ethOut[0];
    hub9.out[0] --> delay 0.0001s --> RRD1.ethIn[1];
    hub9.in[0] <-- delay 0.0001s <-- RRD1.ethOut[1];
    hub9.out[1] --> delay 0.0001s --> RRD2.ethIn[0];
    hub9.in[1] <-- delay 0.0001s <-- RRD2.ethOut[0];
    hub9.out[2] --> delay 0.0001s --> SRD.ethIn[0];
    hub9.in[2] <-- delay 0.0001s <-- SRD.ethOut[0];
    RRF2.ethOut[2] --> delay 0.0001s --> rout12.ethIn[0];
    RRF2.ethIn[2] <-- delay 0.0001s <-- rout12.ethOut[0];
    hub10.out[0] --> delay 0.0001s --> RRE1.ethIn[1];
    hub10.in[0] <-- delay 0.0001s <-- RRE1.ethOut[1];
    hub10.out[1] --> delay 0.0001s --> RRE2.ethIn[0];
    hub10.in[1] <-- delay 0.0001s <-- RRE2.ethOut[0];
    hub10.out[2] --> delay 0.0001s --> SRE.ethIn[0];
    hub10.in[2] <-- delay 0.0001s <-- SRE.ethOut[0];
    rout06.ethOut[1] --> delay 0.0001s --> PC06.ethIn[0];
    rout06.ethIn[1] <-- delay 0.0001s <-- PC06.ethOut[0];
    RB3.ethOut[2] --> delay 0.0001s --> RRC1.ethIn[0];
    RB3.ethIn[2] <-- delay 0.0001s <-- RRC1.ethOut[0];
    rout05.ethOut[1] --> delay 0.0001s --> PC05.ethIn[0];
    rout05.ethIn[1] <-- delay 0.0001s <-- PC05.ethOut[0];
    RB2.ethOut[2] --> delay 0.0001s --> RRA1.ethIn[0];
    RB2.ethIn[2] <-- delay 0.0001s <-- RRA1.ethOut[0];
    hub11.out[0] --> delay 0.0001s --> RRF1.ethIn[1];
    hub11.in[0] <-- delay 0.0001s <-- RRF1.ethOut[1];
    hub11.out[1] --> delay 0.0001s --> RRF2.ethIn[0];
    hub11.in[1] <-- delay 0.0001s <-- RRF2.ethOut[0];
    hub11.out[2] --> delay 0.0001s --> SRF.ethIn[0];
    hub11.in[2] <-- delay 0.0001s <-- SRF.ethOut[0];
    rout07.ethOut[1] --> delay 0.0001s --> PC07.ethIn[0];
    rout07.ethIn[1] <-- delay 0.0001s <-- PC07.ethOut[0];
    RB4.ethOut[2] --> delay 0.0001s --> RRE1.ethIn[0];
    RB4.ethIn[2] <-- delay 0.0001s <-- RRE1.ethOut[0];
    rout04.ethOut[1] --> delay 0.0001s --> PC04.ethIn[0];
    rout04.ethIn[1] <-- delay 0.0001s <-- PC04.ethOut[0];
    rout03.ethOut[1] --> delay 0.0001s --> PC03.ethIn[0];
    rout03.ethIn[1] <-- delay 0.0001s <-- PC03.ethOut[0];
    RRC2.ethOut[1] --> delay 0.0001s --> rout05.ethIn[0];
    RRC2.ethIn[1] <-- delay 0.0001s <-- rout05.ethOut[0];
    rout09.ethOut[1] --> delay 0.0001s --> PC09.ethIn[0];
    rout09.ethIn[1] <-- delay 0.0001s <-- PC09.ethOut[0];
    RB4.ethOut[3] --> delay 0.0001s --> RRF1.ethIn[0];
    RB4.ethIn[3] <-- delay 0.0001s <-- RRF1.ethOut[0];
    RB3.ethOut[1] --> delay 0.0001s --> RB4.ethIn[0];
    RB3.ethIn[1] <-- delay 0.0001s <-- RB4.ethOut[0];
endmodule
network vmtestNetwork : VMTestNetwork
endnetwork
