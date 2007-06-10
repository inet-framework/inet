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
    R3: QuaggaRouter;
      parameters:
        routingDaemon = "Bgpd",
        namid = 3;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=250,175;i=device/router";
    R1: QuaggaRouter;
      parameters:
        routingDaemon = "Bgpd",
        namid = 1;
      gatesizes:
        ethIn[3],
        ethOut[3];
      display: "p=50,100;i=device/router";
    R6: QuaggaRouter;
      parameters:
        routingDaemon = "Bgpd",
        namid = 6;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=250,250;i=device/router";
    R4: QuaggaRouter;
      parameters:
        routingDaemon = "Bgpd",
        namid = 4;
      gatesizes:
        ethIn[2],
        ethOut[2];
      display: "p=50,250;i=device/router";
    R5: QuaggaRouter;
      parameters:
        routingDaemon = "Bgpd",
        namid = 5;
      gatesizes:
        ethIn[1],
        ethOut[1];
      display: "p=150,250;i=device/router";
    R2: QuaggaRouter;
      parameters:
        routingDaemon = "Bgpd",
        namid = 2;
      gatesizes:
        ethIn[4],
        ethOut[4];
      display: "p=150,175;i=device/router";
  connections nocheck:
    R2.ethOut[3] --> delay 0.0001s --> R5.ethIn[0];
    R2.ethIn[3] <-- delay 0.0001s <-- R5.ethOut[0];
    R3.ethOut[2] --> delay 0.0001s --> R6.ethIn[0];
    R3.ethIn[2] <-- delay 0.0001s <-- R6.ethOut[0];
    R2.ethOut[1] --> delay 0.0001s --> R3.ethIn[1];
    R2.ethIn[1] <-- delay 0.0001s <-- R3.ethOut[1];
    R1.ethOut[2] --> delay 0.0001s --> R4.ethIn[0];
    R1.ethIn[2] <-- delay 0.0001s <-- R4.ethOut[0];
    R1.ethOut[1] --> delay 0.0001s --> R3.ethIn[0];
    R1.ethIn[1] <-- delay 0.0001s <-- R3.ethOut[0];
    R1.ethOut[0] --> delay 0.0001s --> R2.ethIn[0];
    R1.ethIn[0] <-- delay 0.0001s <-- R2.ethOut[0];
    R2.ethOut[2] --> delay 0.0001s --> R4.ethIn[1];
    R2.ethIn[2] <-- delay 0.0001s <-- R4.ethOut[1];
endmodule
network vmtestNetwork : VMTestNetwork
endnetwork
