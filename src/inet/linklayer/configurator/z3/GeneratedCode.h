#ifndef __INET_Z3_GENERATEDCODE_H
#define __INET_Z3_GENERATEDCODE_H

package schedule_generator;
import java.util.*;
import java.io.*;


public class GeneratedCode {
	public void runTestCase(){

		/*
		* GENERATING DEVICES
		*/
		Device dev0 = new Device(1000, 0, 1000, 1625);
		Device dev1 = new Device(1000, 0, 1000, 1625);
		Device dev2 = new Device(1000, 0, 1000, 1625);
		Device dev3 = new Device(1000, 0, 1000, 1625);
		Device dev4 = new Device(1000, 0, 1000, 1625);
		Device dev5 = new Device(1000, 0, 1000, 1625);
		Device dev6 = new Device(1000, 0, 1000, 1625);
		Device dev7 = new Device(1000, 0, 1000, 1625);
		Device dev8 = new Device(1000, 0, 1000, 1625);
		Device dev9 = new Device(1000, 0, 1000, 1625);
		Device dev10 = new Device(1000, 0, 1000, 1625);
		Device dev11 = new Device(1000, 0, 1000, 1625);
		Device dev12 = new Device(1000, 0, 1000, 1625);
		Device dev13 = new Device(1000, 0, 1000, 1625);
		Device dev14 = new Device(1000, 0, 1000, 1625);
		Device dev15 = new Device(1000, 0, 1000, 1625);
		Device dev16 = new Device(1000, 0, 1000, 1625);
		Device dev17 = new Device(1000, 0, 1000, 1625);
		Device dev18 = new Device(1000, 0, 1000, 1625);
		Device dev19 = new Device(1000, 0, 1000, 1625);
		Device dev20 = new Device(1000, 0, 1000, 1625);
		Device dev21 = new Device(1000, 0, 1000, 1625);
		Device dev22 = new Device(1000, 0, 1000, 1625);
		Device dev23 = new Device(1000, 0, 1000, 1625);
		Device dev24 = new Device(1000, 0, 1000, 1625);
		Device dev25 = new Device(1000, 0, 1000, 1625);
		Device dev26 = new Device(1000, 0, 1000, 1625);
		Device dev27 = new Device(1000, 0, 1000, 1625);
		Device dev28 = new Device(1000, 0, 1000, 1625);
		Device dev29 = new Device(1000, 0, 1000, 1625);
		Device dev30 = new Device(1000, 0, 1000, 1625);
		Device dev31 = new Device(1000, 0, 1000, 1625);
		Device dev32 = new Device(1000, 0, 1000, 1625);
		Device dev33 = new Device(1000, 0, 1000, 1625);
		Device dev34 = new Device(1000, 0, 1000, 1625);
		Device dev35 = new Device(1000, 0, 1000, 1625);
		Device dev36 = new Device(1000, 0, 1000, 1625);
		Device dev37 = new Device(1000, 0, 1000, 1625);
		Device dev38 = new Device(1000, 0, 1000, 1625);
		Device dev39 = new Device(1000, 0, 1000, 1625);
		Device dev40 = new Device(1000, 0, 1000, 1625);
		Device dev41 = new Device(1000, 0, 1000, 1625);
		Device dev42 = new Device(1000, 0, 1000, 1625);
		Device dev43 = new Device(1000, 0, 1000, 1625);
		Device dev44 = new Device(1000, 0, 1000, 1625);
		Device dev45 = new Device(1000, 0, 1000, 1625);
		Device dev46 = new Device(1000, 0, 1000, 1625);
		Device dev47 = new Device(1000, 0, 1000, 1625);
		Device dev48 = new Device(1000, 0, 1000, 1625);
		Device dev49 = new Device(1000, 0, 1000, 1625);


		/*
		* GENERATING SWITCHES
		*/
		// 1Gbit/s = 125Bytes/microsec. Packet size on packets should be in bytes
		TSNSwitch switch0 = new TSNSwitch("switch0",100, 1, 125, 1, 400, 3000);
		TSNSwitch switch1 = new TSNSwitch("switch1",100, 1, 125, 1, 400, 3000);
		TSNSwitch switch2 = new TSNSwitch("switch2",100, 1, 125, 1, 400, 3000);
		TSNSwitch switch3 = new TSNSwitch("switch3",100, 1, 125, 1, 400, 3000);
		TSNSwitch switch4 = new TSNSwitch("switch4",100, 1, 125, 1, 400, 3000);
		TSNSwitch switch5 = new TSNSwitch("switch5",100, 1, 125, 1, 400, 3000);
		TSNSwitch switch6 = new TSNSwitch("switch6",100, 1, 125, 1, 400, 3000);
		TSNSwitch switch7 = new TSNSwitch("switch7",100, 1, 125, 1, 400, 3000);
		TSNSwitch switch8 = new TSNSwitch("switch8",100, 1, 125, 1, 400, 3000);
		TSNSwitch switch9 = new TSNSwitch("switch9",100, 1, 125, 1, 400, 3000);


		/*
		* GENERATING PORTS
		*/
		Cycle cycle0 = new Cycle(50);
		switch0.createPort(switch1, cycle0);
		Cycle cycle1 = new Cycle(50);
		switch1.createPort(switch0, cycle1);
		Cycle cycle2 = new Cycle(50);
		switch0.createPort(switch2, cycle2);
		Cycle cycle3 = new Cycle(50);
		switch2.createPort(switch0, cycle3);
		Cycle cycle4 = new Cycle(50);
		switch0.createPort(switch3, cycle4);
		Cycle cycle5 = new Cycle(50);
		switch3.createPort(switch0, cycle5);
		Cycle cycle6 = new Cycle(50);
		switch0.createPort(switch4, cycle6);
		Cycle cycle7 = new Cycle(50);
		switch4.createPort(switch0, cycle7);
		Cycle cycle8 = new Cycle(50);
		switch0.createPort(switch5, cycle8);
		Cycle cycle9 = new Cycle(50);
		switch5.createPort(switch0, cycle9);
		Cycle cycle10 = new Cycle(50);
		switch0.createPort(switch6, cycle10);
		Cycle cycle11 = new Cycle(50);
		switch6.createPort(switch0, cycle11);
		Cycle cycle12 = new Cycle(50);
		switch0.createPort(switch7, cycle12);
		Cycle cycle13 = new Cycle(50);
		switch7.createPort(switch0, cycle13);
		Cycle cycle14 = new Cycle(50);
		switch0.createPort(switch8, cycle14);
		Cycle cycle15 = new Cycle(50);
		switch8.createPort(switch0, cycle15);
		Cycle cycle16 = new Cycle(50);
		switch0.createPort(switch9, cycle16);
		Cycle cycle17 = new Cycle(50);
		switch9.createPort(switch0, cycle17);
		Cycle cycle18 = new Cycle(50);
		switch1.createPort(switch2, cycle18);
		Cycle cycle19 = new Cycle(50);
		switch2.createPort(switch1, cycle19);
		Cycle cycle20 = new Cycle(50);
		switch1.createPort(switch3, cycle20);
		Cycle cycle21 = new Cycle(50);
		switch3.createPort(switch1, cycle21);
		Cycle cycle22 = new Cycle(50);
		switch1.createPort(switch4, cycle22);
		Cycle cycle23 = new Cycle(50);
		switch4.createPort(switch1, cycle23);
		Cycle cycle24 = new Cycle(50);
		switch1.createPort(switch5, cycle24);
		Cycle cycle25 = new Cycle(50);
		switch5.createPort(switch1, cycle25);
		Cycle cycle26 = new Cycle(50);
		switch1.createPort(switch6, cycle26);
		Cycle cycle27 = new Cycle(50);
		switch6.createPort(switch1, cycle27);
		Cycle cycle28 = new Cycle(50);
		switch1.createPort(switch7, cycle28);
		Cycle cycle29 = new Cycle(50);
		switch7.createPort(switch1, cycle29);
		Cycle cycle30 = new Cycle(50);
		switch1.createPort(switch8, cycle30);
		Cycle cycle31 = new Cycle(50);
		switch8.createPort(switch1, cycle31);
		Cycle cycle32 = new Cycle(50);
		switch1.createPort(switch9, cycle32);
		Cycle cycle33 = new Cycle(50);
		switch9.createPort(switch1, cycle33);
		Cycle cycle34 = new Cycle(50);
		switch2.createPort(switch3, cycle34);
		Cycle cycle35 = new Cycle(50);
		switch3.createPort(switch2, cycle35);
		Cycle cycle36 = new Cycle(50);
		switch2.createPort(switch4, cycle36);
		Cycle cycle37 = new Cycle(50);
		switch4.createPort(switch2, cycle37);
		Cycle cycle38 = new Cycle(50);
		switch2.createPort(switch5, cycle38);
		Cycle cycle39 = new Cycle(50);
		switch5.createPort(switch2, cycle39);
		Cycle cycle40 = new Cycle(50);
		switch2.createPort(switch6, cycle40);
		Cycle cycle41 = new Cycle(50);
		switch6.createPort(switch2, cycle41);
		Cycle cycle42 = new Cycle(50);
		switch2.createPort(switch7, cycle42);
		Cycle cycle43 = new Cycle(50);
		switch7.createPort(switch2, cycle43);
		Cycle cycle44 = new Cycle(50);
		switch2.createPort(switch8, cycle44);
		Cycle cycle45 = new Cycle(50);
		switch8.createPort(switch2, cycle45);
		Cycle cycle46 = new Cycle(50);
		switch2.createPort(switch9, cycle46);
		Cycle cycle47 = new Cycle(50);
		switch9.createPort(switch2, cycle47);
		Cycle cycle48 = new Cycle(50);
		switch3.createPort(switch4, cycle48);
		Cycle cycle49 = new Cycle(50);
		switch4.createPort(switch3, cycle49);
		Cycle cycle50 = new Cycle(50);
		switch3.createPort(switch5, cycle50);
		Cycle cycle51 = new Cycle(50);
		switch5.createPort(switch3, cycle51);
		Cycle cycle52 = new Cycle(50);
		switch3.createPort(switch6, cycle52);
		Cycle cycle53 = new Cycle(50);
		switch6.createPort(switch3, cycle53);
		Cycle cycle54 = new Cycle(50);
		switch3.createPort(switch7, cycle54);
		Cycle cycle55 = new Cycle(50);
		switch7.createPort(switch3, cycle55);
		Cycle cycle56 = new Cycle(50);
		switch3.createPort(switch8, cycle56);
		Cycle cycle57 = new Cycle(50);
		switch8.createPort(switch3, cycle57);
		Cycle cycle58 = new Cycle(50);
		switch3.createPort(switch9, cycle58);
		Cycle cycle59 = new Cycle(50);
		switch9.createPort(switch3, cycle59);
		Cycle cycle60 = new Cycle(50);
		switch4.createPort(switch5, cycle60);
		Cycle cycle61 = new Cycle(50);
		switch5.createPort(switch4, cycle61);
		Cycle cycle62 = new Cycle(50);
		switch4.createPort(switch6, cycle62);
		Cycle cycle63 = new Cycle(50);
		switch6.createPort(switch4, cycle63);
		Cycle cycle64 = new Cycle(50);
		switch4.createPort(switch7, cycle64);
		Cycle cycle65 = new Cycle(50);
		switch7.createPort(switch4, cycle65);
		Cycle cycle66 = new Cycle(50);
		switch4.createPort(switch8, cycle66);
		Cycle cycle67 = new Cycle(50);
		switch8.createPort(switch4, cycle67);
		Cycle cycle68 = new Cycle(50);
		switch4.createPort(switch9, cycle68);
		Cycle cycle69 = new Cycle(50);
		switch9.createPort(switch4, cycle69);
		Cycle cycle70 = new Cycle(50);
		switch5.createPort(switch6, cycle70);
		Cycle cycle71 = new Cycle(50);
		switch6.createPort(switch5, cycle71);
		Cycle cycle72 = new Cycle(50);
		switch5.createPort(switch7, cycle72);
		Cycle cycle73 = new Cycle(50);
		switch7.createPort(switch5, cycle73);
		Cycle cycle74 = new Cycle(50);
		switch5.createPort(switch8, cycle74);
		Cycle cycle75 = new Cycle(50);
		switch8.createPort(switch5, cycle75);
		Cycle cycle76 = new Cycle(50);
		switch5.createPort(switch9, cycle76);
		Cycle cycle77 = new Cycle(50);
		switch9.createPort(switch5, cycle77);
		Cycle cycle78 = new Cycle(50);
		switch6.createPort(switch7, cycle78);
		Cycle cycle79 = new Cycle(50);
		switch7.createPort(switch6, cycle79);
		Cycle cycle80 = new Cycle(50);
		switch6.createPort(switch8, cycle80);
		Cycle cycle81 = new Cycle(50);
		switch8.createPort(switch6, cycle81);
		Cycle cycle82 = new Cycle(50);
		switch6.createPort(switch9, cycle82);
		Cycle cycle83 = new Cycle(50);
		switch9.createPort(switch6, cycle83);
		Cycle cycle84 = new Cycle(50);
		switch7.createPort(switch8, cycle84);
		Cycle cycle85 = new Cycle(50);
		switch8.createPort(switch7, cycle85);
		Cycle cycle86 = new Cycle(50);
		switch7.createPort(switch9, cycle86);
		Cycle cycle87 = new Cycle(50);
		switch9.createPort(switch7, cycle87);
		Cycle cycle88 = new Cycle(50);
		switch8.createPort(switch9, cycle88);
		Cycle cycle89 = new Cycle(50);
		switch9.createPort(switch8, cycle89);

		/*
		* LINKING SWITCHES TO DEVICES
		*/
		Cycle cycle90 = new Cycle(50);
		switch0.createPort(dev0, cycle90);
		Cycle cycle91 = new Cycle(50);
		switch0.createPort(dev1, cycle91);
		Cycle cycle92 = new Cycle(50);
		switch0.createPort(dev2, cycle92);
		Cycle cycle93 = new Cycle(50);
		switch0.createPort(dev3, cycle93);
		Cycle cycle94 = new Cycle(50);
		switch0.createPort(dev4, cycle94);
		Cycle cycle95 = new Cycle(50);
		switch1.createPort(dev5, cycle95);
		Cycle cycle96 = new Cycle(50);
		switch1.createPort(dev6, cycle96);
		Cycle cycle97 = new Cycle(50);
		switch1.createPort(dev7, cycle97);
		Cycle cycle98 = new Cycle(50);
		switch1.createPort(dev8, cycle98);
		Cycle cycle99 = new Cycle(50);
		switch1.createPort(dev9, cycle99);
		Cycle cycle100 = new Cycle(50);
		switch2.createPort(dev10, cycle100);
		Cycle cycle101 = new Cycle(50);
		switch2.createPort(dev11, cycle101);
		Cycle cycle102 = new Cycle(50);
		switch2.createPort(dev12, cycle102);
		Cycle cycle103 = new Cycle(50);
		switch2.createPort(dev13, cycle103);
		Cycle cycle104 = new Cycle(50);
		switch2.createPort(dev14, cycle104);
		Cycle cycle105 = new Cycle(50);
		switch3.createPort(dev15, cycle105);
		Cycle cycle106 = new Cycle(50);
		switch3.createPort(dev16, cycle106);
		Cycle cycle107 = new Cycle(50);
		switch3.createPort(dev17, cycle107);
		Cycle cycle108 = new Cycle(50);
		switch3.createPort(dev18, cycle108);
		Cycle cycle109 = new Cycle(50);
		switch3.createPort(dev19, cycle109);
		Cycle cycle110 = new Cycle(50);
		switch4.createPort(dev20, cycle110);
		Cycle cycle111 = new Cycle(50);
		switch4.createPort(dev21, cycle111);
		Cycle cycle112 = new Cycle(50);
		switch4.createPort(dev22, cycle112);
		Cycle cycle113 = new Cycle(50);
		switch4.createPort(dev23, cycle113);
		Cycle cycle114 = new Cycle(50);
		switch4.createPort(dev24, cycle114);
		Cycle cycle115 = new Cycle(50);
		switch5.createPort(dev25, cycle115);
		Cycle cycle116 = new Cycle(50);
		switch5.createPort(dev26, cycle116);
		Cycle cycle117 = new Cycle(50);
		switch5.createPort(dev27, cycle117);
		Cycle cycle118 = new Cycle(50);
		switch5.createPort(dev28, cycle118);
		Cycle cycle119 = new Cycle(50);
		switch5.createPort(dev29, cycle119);
		Cycle cycle120 = new Cycle(50);
		switch6.createPort(dev30, cycle120);
		Cycle cycle121 = new Cycle(50);
		switch6.createPort(dev31, cycle121);
		Cycle cycle122 = new Cycle(50);
		switch6.createPort(dev32, cycle122);
		Cycle cycle123 = new Cycle(50);
		switch6.createPort(dev33, cycle123);
		Cycle cycle124 = new Cycle(50);
		switch6.createPort(dev34, cycle124);
		Cycle cycle125 = new Cycle(50);
		switch7.createPort(dev35, cycle125);
		Cycle cycle126 = new Cycle(50);
		switch7.createPort(dev36, cycle126);
		Cycle cycle127 = new Cycle(50);
		switch7.createPort(dev37, cycle127);
		Cycle cycle128 = new Cycle(50);
		switch7.createPort(dev38, cycle128);
		Cycle cycle129 = new Cycle(50);
		switch7.createPort(dev39, cycle129);
		Cycle cycle130 = new Cycle(50);
		switch8.createPort(dev40, cycle130);
		Cycle cycle131 = new Cycle(50);
		switch8.createPort(dev41, cycle131);
		Cycle cycle132 = new Cycle(50);
		switch8.createPort(dev42, cycle132);
		Cycle cycle133 = new Cycle(50);
		switch8.createPort(dev43, cycle133);
		Cycle cycle134 = new Cycle(50);
		switch8.createPort(dev44, cycle134);
		Cycle cycle135 = new Cycle(50);
		switch9.createPort(dev45, cycle135);
		Cycle cycle136 = new Cycle(50);
		switch9.createPort(dev46, cycle136);
		Cycle cycle137 = new Cycle(50);
		switch9.createPort(dev47, cycle137);
		Cycle cycle138 = new Cycle(50);
		switch9.createPort(dev48, cycle138);
		Cycle cycle139 = new Cycle(50);
		switch9.createPort(dev49, cycle139);


		/*
		* GENERATING FLOWS
		*/
		LinkedList<PathNode> nodeList;

		Flow flow0 = new Flow(Flow.PUBLISH_SUBSCRIBE);
		PathTree pathTree0 = new PathTree();
		PathNode pathNode0;
		pathNode0 = pathTree0.addRoot(dev4);
		pathNode0 = pathNode0.addChild(switch0);
		nodeList = new LinkedList<PathNode>();
		nodeList.add(pathNode0);
		nodeList.add(nodeList.removeFirst().addChild(switch4));
		nodeList.add(nodeList.removeFirst().addChild(switch3));
		nodeList.getFirst().addChild(dev15);
		nodeList.getFirst().addChild(dev16);
		nodeList.getFirst().addChild(dev17);
		nodeList.getFirst().addChild(dev18);
		nodeList.getFirst().addChild(dev19);
		nodeList.removeFirst();
		flow0.setPathTree(pathTree0);

		Flow flow1 = new Flow(Flow.PUBLISH_SUBSCRIBE);
		PathTree pathTree1 = new PathTree();
		PathNode pathNode1;
		pathNode1 = pathTree1.addRoot(dev21);
		pathNode1 = pathNode1.addChild(switch4);
		nodeList = new LinkedList<PathNode>();
		nodeList.add(pathNode1);
		nodeList.add(nodeList.removeFirst().addChild(switch7));
		nodeList.add(nodeList.removeFirst().addChild(switch6));
		nodeList.getFirst().addChild(dev30);
		nodeList.getFirst().addChild(dev31);
		nodeList.getFirst().addChild(dev32);
		nodeList.getFirst().addChild(dev33);
		nodeList.getFirst().addChild(dev34);
		nodeList.removeFirst();
		flow1.setPathTree(pathTree1);

		Flow flow2 = new Flow(Flow.PUBLISH_SUBSCRIBE);
		PathTree pathTree2 = new PathTree();
		PathNode pathNode2;
		pathNode2 = pathTree2.addRoot(dev39);
		pathNode2 = pathNode2.addChild(switch7);
		nodeList = new LinkedList<PathNode>();
		nodeList.add(pathNode2);
		nodeList.add(nodeList.removeFirst().addChild(switch1));
		nodeList.add(nodeList.removeFirst().addChild(switch2));
		nodeList.getFirst().addChild(dev10);
		nodeList.getFirst().addChild(dev11);
		nodeList.getFirst().addChild(dev12);
		nodeList.getFirst().addChild(dev13);
		nodeList.getFirst().addChild(dev14);
		nodeList.removeFirst();
		flow2.setPathTree(pathTree2);


		/*
		* GENERATING THE NETWORK
		*/
		Network net = new Network();
		net.addSwitch(switch0);
		net.addSwitch(switch1);
		net.addSwitch(switch2);
		net.addSwitch(switch3);
		net.addSwitch(switch4);
		net.addSwitch(switch5);
		net.addSwitch(switch6);
		net.addSwitch(switch7);
		net.addSwitch(switch8);
		net.addSwitch(switch9);
		net.addFlow(flow0);
		net.addFlow(flow1);
		net.addFlow(flow2);


		ScheduleGenerator scheduleGenerator = new ScheduleGenerator();
		long startTime = System.nanoTime();
		scheduleGenerator.generateSchedule(net);
		long endTime   = System.nanoTime();
		long totalTime = endTime - startTime;
		int numOfFramesScheduled = 0;


		/*
		* OUTPUT DATA
		*/
		float overallAverageJitter = 0;
		float overallAverageLatency = 0;
		System.out.println("");
		System.out.println("");
		int auxCount = 0;
		ArrayList<PathNode> auxNodes;
		ArrayList<FlowFragment> auxFlowFragments;

		Cycle auxCycle;
		TSNSwitch auxSwt;
		int flagContinueLoop = 1;

		for(Switch swt : net.getSwitches()) {
			flagContinueLoop = 1;
			auxSwt = (TSNSwitch) swt;
			for(Port port : auxSwt.getPorts()) {
				if(port.getCycle().getSlotsUsed().size() != 0) {
					flagContinueLoop = 0;
					break;
				}
			}

			if(flagContinueLoop == 1) {
				continue;
			}
			System.out.println("\n\n>>>> INFORMATION OF SWITCH: " + auxSwt.getName() + " <<<<");
			System.out.println("    Port list - ");
				for(Port port : auxSwt.getPorts()) {
					if(port.getCycle().getSlotsUsed().size() == 0) {
						continue;
					}
					System.out.println("        => Port name:       " + port.getName());
					System.out.println("        Connects to:     " + port.getConnectsTo());

					System.out.println("        Cycle start:    " + port.getCycle().getCycleStart());
					System.out.println("        Cycle duration: " + port.getCycle().getCycleDuration());
					System.out.print("        Fragments:       ");
					for(FlowFragment ffrag : port.getFlowFragments()) {
						System.out.print(ffrag.getName() + ", ");
					}
					System.out.println();

					auxCycle = port.getCycle();
					System.out.println("        Slots per prt:   " +  auxCycle.getNumOfSlots());
				for(int i = 0; i < auxCycle.getSlotsUsed().size(); i++) {
					System.out.println("        Priority number: " + auxCycle.getSlotsUsed().get(i));
					for(int j = 0; j < auxCycle.getNumOfSlots(); j++) {						System.out.println("          Index " + j + " Slot start:      " + auxCycle.getSlotStart(auxCycle.getSlotsUsed().get(i), j));
						System.out.println("          Index " + j + " Slot duration:   " + auxCycle.getSlotDuration(auxCycle.getSlotsUsed().get(i), j));
					}					System.out.println("        ------------------------");
				}
			}
		}

		System.out.println("");

		float sumOfAvgLatencies = 0;
		float sumOfLatencies;
		int flowCounter = 0;
		for(Flow flw : net.getFlows()){


			System.out.println("\n\n>>>> INFORMATION OF FLOW" + flowCounter++ + " <<<<\n");

			System.out.println("    Total number of packets scheduled: " + flw.getTotalNumOfPackets());
			numOfFramesScheduled = numOfFramesScheduled + flw.getTotalNumOfPackets();
			System.out.println("    Path tree of the flow:");
			for(PathNode node : flw.getPathTree().getLeaves()) {
				auxNodes = flw.getNodesFromRootToNode((Device) node.getNode());
				auxFlowFragments = flw.getFlowFromRootToNode((Device) node.getNode());
				System.out.print("        Path to " + ((Device) node.getNode()).getName() + ": ");
				auxCount = 0;
				for(PathNode auxNode : auxNodes) {
					if(auxNode.getNode() instanceof Device) {
						System.out.print(((Device) auxNode.getNode()).getName() + ", ");
					} else if (auxNode.getNode() instanceof TSNSwitch) {
						System.out.print(((TSNSwitch) auxNode.getNode()).getName() +	"(" + auxFlowFragments.get(auxCount).getName() + "), ");
						auxCount++;
					}
				}
				System.out.println("");
			}


			System.out.println();
			System.out.println();
			for(PathNode node : flw.getPathTree().getLeaves()) {
				Device dev = (Device) node.getNode();

				sumOfLatencies = 0;
				System.out.println("    Packets heading to " + dev.getName() + ":");

				for(int i = 0; i < flw.getNumOfPacketsSent(); i++) {
					System.out.println("       Flow firstDepartureTime of packet " + i + ": " + flw.getDepartureTime(dev, 0, i));
					System.out.println("       Flow lastScheduledTime of packet " + i + ":  " + flw.getScheduledTime(dev, flw.getFlowFromRootToNode(dev).size() - 1, i));
					sumOfLatencies += flw.getScheduledTime(dev, flw.getFlowFromRootToNode(dev).size() - 1, i) - flw.getDepartureTime(dev, 0, i);
				}

				sumOfAvgLatencies += sumOfLatencies/flw.getNumOfPacketsSent();
				System.out.println("       Calculated average Latency: " + (sumOfLatencies/flw.getNumOfPacketsSent()));
				System.out.println("       Method average Latency: " + flw.getAverageLatencyToDevice(dev));
				System.out.println("       Method average Jitter: " + flw.getAverageJitterToDevice(dev));
				System.out.println("");

			}
			System.out.println("    Calculated average latency of all devices: " + sumOfAvgLatencies/flw.getPathTree().getLeaves().size());
			sumOfAvgLatencies = 0;
		}




		System.out.println("Execution time: " + ((float) totalTime)/1000000000 + " seconds\n ");
		System.out.println("Flow 0 average latency: " + flow0.getAverageLatency());
		System.out.println("Flow 0 average jitter: " + flow0.getAverageJitter());
		overallAverageLatency += flow0.getAverageLatency();
		overallAverageJitter += flow0.getAverageJitter();
		System.out.println("Flow 1 average latency: " + flow1.getAverageLatency());
		System.out.println("Flow 1 average jitter: " + flow1.getAverageJitter());
		overallAverageLatency += flow1.getAverageLatency();
		overallAverageJitter += flow1.getAverageJitter();
		System.out.println("Flow 2 average latency: " + flow2.getAverageLatency());
		System.out.println("Flow 2 average jitter: " + flow2.getAverageJitter());
		overallAverageLatency += flow2.getAverageLatency();
		overallAverageJitter += flow2.getAverageJitter();
		overallAverageLatency = overallAverageLatency/3;
		overallAverageJitter = overallAverageJitter/3;

		System.out.println("\nNumber of nodes in the network: 3 ");
		System.out.println("Number of flows in the network: 3 ");
		System.out.println("Number of subscribers in the network: 15 ");
		System.out.println("Total number of scheduled packets: " +  numOfFramesScheduled);
		System.out.println("Overall average latency: " + overallAverageLatency);
		System.out.println("Overall average jitter: " + overallAverageJitter);
	}
}

#endif

