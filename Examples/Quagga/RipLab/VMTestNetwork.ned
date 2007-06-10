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
    PC03: StandardHost;
      parameters:
        namid = 9;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=300,450;i=device/pc2";
    R1: QuaggaRouter;
      parameters:
        routingDaemon = "Ripd",
        namid = 2;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=250,150;i=device/router";
    R4: QuaggaRouter;
      parameters:
        routingDaemon = "Ripd",
        namid = 6;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=50,350;i=device/router";
    R5: QuaggaRouter;
      parameters:
        routingDaemon = "Ripd",
        namid = 8;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=300,350;i=device/router";
    PC01: StandardHost;
      parameters:
        namid = 1;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=100,100;i=device/pc2";
    SRV2: StandardHost;
      parameters:
        namid = 5;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=220,300;i=device/pc2";
    SRV1: StandardHost;
      parameters:
        namid = 4;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=135,300;i=device/pc2";
    PC02: StandardHost;
      parameters:
        namid = 7;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=100,450;i=device/pc2";
    R2: QuaggaRouter;
      parameters:
        routingDaemon = "Ripd",
        namid = 3;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=100,250;i=device/router";
    hub1: EtherHub;
      gatesizes:
        in[3],
        out[3];
      display: "p=200,230;i=device/switch";
    hub2: EtherHub;
      gatesizes:
        in[2],
        out[2];
      display: "p=125,400;i=device/switch";
    hub3: EtherHub;
      gatesizes:
        in[2],
        out[2];
      display: "p=150,150;i=device/switch";
    hub4: EtherHub;
      gatesizes:
        in[2],
        out[2];
      display: "p=325,400;i=device/switch";
  connections nocheck:
    hub1.out[0] --> delay 0.0001s --> R2.ethIn[1];
    hub1.in[0] <-- delay 0.0001s <-- R2.ethOut[1];
    hub1.out[1] --> delay 0.0001s --> SRV1.ethIn[0];
    hub1.in[1] <-- delay 0.0001s <-- SRV1.ethOut[0];
    hub1.out[2] --> delay 0.0001s --> SRV2.ethIn[0];
    hub1.in[2] <-- delay 0.0001s <-- SRV2.ethOut[0];
    hub2.out[0] --> delay 0.0001s --> R4.ethIn[2];
    hub2.in[0] <-- delay 0.0001s <-- R4.ethOut[2];
    hub2.out[1] --> delay 0.0001s --> PC02.ethIn[0];
    hub2.in[1] <-- delay 0.0001s <-- PC02.ethOut[0];
    hub3.out[0] --> delay 0.0001s --> PC01.ethIn[0];
    hub3.in[0] <-- delay 0.0001s <-- PC01.ethOut[0];
    hub3.out[1] --> delay 0.0001s --> R1.ethIn[0];
    hub3.in[1] <-- delay 0.0001s <-- R1.ethOut[0];
    R4.ethOut[1] --> delay 0.0001s --> R5.ethIn[0];
    R4.ethIn[1] <-- delay 0.0001s <-- R5.ethOut[0];
    hub4.out[0] --> delay 0.0001s --> R5.ethIn[1];
    hub4.in[0] <-- delay 0.0001s <-- R5.ethOut[1];
    hub4.out[1] --> delay 0.0001s --> PC03.ethIn[0];
    hub4.in[1] <-- delay 0.0001s <-- PC03.ethOut[0];
    R2.ethOut[2] --> delay 0.0001s --> R4.ethIn[0];
    R2.ethIn[2] <-- delay 0.0001s <-- R4.ethOut[0];
    R1.ethOut[1] --> delay 0.0001s --> R2.ethIn[0];
    R1.ethIn[1] <-- delay 0.0001s <-- R2.ethOut[0];
endmodule
network vmtestNetwork : VMTestNetwork
endnetwork
