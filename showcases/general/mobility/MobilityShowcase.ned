package inet.showcases.general.mobility;

import inet.mobility.single.LinearMobility;
import inet.node.inet.StandardHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedMultiVisualizer;
import inet.visualizer.integrated.IntegratedVisualizer;


network MobilityShowcaseA
{
    @display("bgb=400,400");
    submodules:
        host1: StandardHost {
            @display("p=93,167;i=misc/smartphone");
        }
        host2: StandardHost {
            @display("p=293,176;i=misc/smartphone");
        }
        host3: StandardHost {
            @display("p=168,99;i=misc/smartphone");
        }
        host4: StandardHost {
            @display("p=145,293;i=misc/smartphone");
        }
        host5: StandardHost {
            @display("p=285,293;i=misc/smartphone");
        }
        host6: StandardHost {
            @display("p=256,85;i=misc/smartphone");
        }
        host7: StandardHost {
            @display("p=191,206;i=misc/smartphone");
        }
        visualizer: IntegratedVisualizer {
            @display("p=47,39");
        }
}

network MobilityShowcaseB
{
    parameters:
        int numHosts;
        @display("bgb=400,400");
    submodules:
        host[numHosts]: StandardHost {
            @display("i=misc/smartphone");
        }
        visualizer: IntegratedVisualizer {
            @display("p=47,39");
        }
}

network MobilityShowcaseSimplistic1
{
    @display("bgb=400,400");
    submodules:
        mobility1: LinearMobility {
            @display("p=48,128");
        }
        mobility2: LinearMobility {
            @display("p=85,48");
        }
}

network MobilityShowcaseSimplistic2
{
    @display("bgb=400,400");
    submodules:
        mobility1: LinearMobility {
            @display("p=84,74");
        }
        mobility2: LinearMobility {
            @display("p=84,161");
        }
        visualizer: IntegratedVisualizer {
            @display("p=84,234");
        }
}

network MobilityShowcaseE
{
    types:
        module Playground
        {
            @display("bgb=400,400;i=block/network_s");
        }
    submodules:
        host1: StandardHost {
            @display("i=misc/smartphone;p=300,100");
        }
        host2: StandardHost {
            @display("i=misc/smartphone;p=200,100");
        }
        visualizer: IntegratedVisualizer {
            @display("p=47,39");
        }
        playground: Playground {
            @display("p=47,111");
        }
}

//network MobilityShowcaseF
//{
//    @display("bgb=400,400");
//    submodules:
//        host1: StandardHost {
//            @display("i=misc/smartphone");
//        }
//        host2: StandardHost {
//            @display("i=misc/smartphone");
//        }
//        host3: StandardHost {
//            @display("i=misc/smartphone");
//        }
//        host4: StandardHost {
//            @display("i=misc/smartphone");
//        }
//        visualizer: IntegratedVisualizer {
//            @display("p=47,39");
//        }
//}

network MobilityShowcaseG
{
    @display("bgb=400,400");
    submodules:
        mobility1: LinearMobility {
            @display("p=48,128");
        }
        mobility2: LinearMobility {
            @display("p=85,48");
        }
}

network MobilityShowcaseH
{
    @display("bgb=400,400");
    submodules:
        mobility1: LinearMobility {
            @display("p=84,74");
        }
        mobility2: LinearMobility {
            @display("p=84,161");
        }
        visualizer: IntegratedVisualizer {
            @display("p=84,234");
        }
}

network MobilityShowcaseI extends MobilityShowcaseB
{
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=81,158");
        }
}

//network MobilityShowcaseJ
//{
//    @display("bgb=400,400");
//    submodules:
//        visualizer: IntegratedVisualizer {
//            @display("p=56,43");
//        }
//        host1: StandardHost {
//            @display("p=87,265;i=misc/smartphone");
//        }
//        host3: StandardHost {
//            @display("p=281,265;i=misc/smartphone");
//        }
//        host2: StandardHost {
//            @display("p=187,130;i=misc/smartphone");
//        }
//}

network MobilityShowcaseK
{
    @display("bgb=400,400");
    submodules:
        rover: StandardHost {
            @display("p=137,99;i=showcases/marsrover");
        }
        antenna: StandardHost {
            @display("p=118,128;i=misc/antenna");
        }
}

network MobilityShowcaseL
{
    @display("bgb=400,400");
    submodules:
        rover: StandardHost {
            @display("p=312,219;i=showcases/marsrover");
        }
        visualizer: IntegratedVisualizer {
            @display("p=67,299");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=163,298");
        }
        satellite: StandardHost {
            @display("p=43,214;i=misc/satellite2");
        }
}

network MobilityShowcaseM
{
    @display("bgb=800,800");
    string minus = default("-");

    submodules:
        rover: StandardHost {
            @display("p=247.632,395.568;b=100,64,rect,$minus,$minus;i=-");
        }
        visualizer: IntegratedMultiVisualizer {
            @display("p=61.104,57.888");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=173.664,56.28");
        }
        drone: StandardHost {
            @display("p=43,214;i=misc/drone");
        }
        base: StandardHost {
            @display("p=582.096,201;i=device/antennatower");
        }
}

network MobilityShowcaseM1 extends MobilityShowcaseM
{
    @display("bgi=showcases/desert");
}

network MobilityShowcaseM2 extends MobilityShowcaseM
{
    @display("bgi=showcases/desert2,s");
}

network MobilityShowcaseM3 extends MobilityShowcaseM
{
    @display("bgi=showcases/desert3,s");
}

network MobilityShowcaseM3_50 extends MobilityShowcaseM
{
    @display("bgi=showcases/desert3_50,s");
}

network MobilityShowcaseM3_75 extends MobilityShowcaseM
{
    @display("bgi=showcases/desert3_75,s");
}

network MobilityShowcaseN
{
    @display("bgb=800,800");
    submodules:
        rover: StandardHost {
            @display("p=312,219;i=showcases/marsrover");
        }
        visualizer: IntegratedVisualizer {
            @display("p=61.104,57.888");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=173.664,56.28");
        }
        satellite: StandardHost {
            @display("p=43,214;i=misc/satellite2");
        }
}

network MobilityShowcaseC
{
    @display("bgb=800,800;bgi=showcases/desert3,s");
    string minus = default("-");

    submodules:
        rover: StandardHost {
            @display("p=247.632,395.568;b=100,64,rect,$minus,$minus;i=-");
        }
        visualizer: IntegratedMultiVisualizer {
            @display("p=61.104,57.888");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=173.664,56.28");
        }
        drone: StandardHost {
            @display("p=43,214;i=misc/drone");
        }
        base: StandardHost {
            @display("p=582.096,201;i=device/antennatower");
        }
}

// until this