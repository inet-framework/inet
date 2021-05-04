//
// Copyright (C) 2021 OpenSim Ltd. and the original authors
//
// This file is partly copied from the following project with the explicit
// permission from the authors: https://github.com/ACassimiro/TSNsched
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/linklayer/configurator/gatescheduling/z3/Z3GateSchedulingConfigurator.h"

namespace inet {

Define_Module(Z3GateSchedulingConfigurator);

Z3GateSchedulingConfigurator::Output *Z3GateSchedulingConfigurator::computeGateScheduling(const Input& input) const
{
    auto output = new Output();
    std::map<cModule *, cObject *> devices;
    std::map<Input::Flow *, Flow *> flows;
    std::map<cModule *, Cycle *> interfaceCycles;
    Network *network = new Network();
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        auto networkNode = node->module;
        if (isBridgeNode(node)) {
            double cycleDurationUpperBound = (gateCycleDuration * 1000000).dbl();
            Switch *tsnSwitch = new Switch(networkNode->getFullName(), 0, cycleDurationUpperBound);
            devices[node->module] = tsnSwitch;
            network->addSwitch(tsnSwitch);
        }
        else {
            Device *device = new Device(0, 0, 0, 0);
            device->setName(networkNode->getFullName());
            devices[node->module] = device;
        }
    }
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        if (isBridgeNode(node)) {
            for (int j = 0; j < node->getNumOutLinks(); j++) {
                auto link = node->getLinkOut(j);
                Node *localNode = (Node *)link->getLocalNode();
                Node *remoteNode = (Node *)link->getRemoteNode();
                Cycle *cycle = new Cycle((gateCycleDuration * 1000000).dbl(), 0, (gateCycleDuration * 1000000).dbl());
                interfaceCycles[((Link *)link)->sourceInterfaceInfo->networkInterface] = cycle;
                // TODO datarate, propagation time
                double datarate = 1E+9 / 1000000;
                double propagationTime = 50E-9 * 1000000;
                double guardBand = 0;
                for (auto& flow : input.flows) {
                    double v = b(flow->startApplication->packetLength).get() / datarate;
                    if (guardBand < v)
                        guardBand = v;
                }
                check_and_cast<Switch *>(devices[localNode->module])->createPort(devices[remoteNode->module], cycle, 1500 * 8, propagationTime, datarate, guardBand);
            }
        }
    }
    for (auto f : input.flows) {
        Flow *flow = new Flow(Flow::UNICAST);
        flow->setFixedPriority(true);
        flow->setPriorityValue(f->startApplication->priority);
        auto startDevice = check_and_cast<Device *>(devices[f->startApplication->device->module]);
        startDevice->setPacketPeriodicity((f->startApplication->packetInterval * 1000000).dbl());
        startDevice->setPacketSize(b(f->startApplication->packetLength + B(12)).get());
        startDevice->setHardConstraintTime((f->startApplication->maxLatency * 1000000).dbl());
        flow->setStartDevice(startDevice);
        for (auto pathFragment : f->pathFragments) {
            for (auto networkNode : pathFragment->networkNodes) {
                if (dynamic_cast<Input::Switch *>(networkNode))
                    flow->addToPath(check_and_cast<Switch *>(devices[networkNode->module]));
            }
        }
        flow->setEndDevice(check_and_cast<Device *>(devices[f->endDevice->module]));
        network->addFlow(flow);
        flows[f] = flow;
    }
    generateSchedule(network);
    for (auto switch_ : input.switches) {
        for (auto port : switch_->ports) {
            Cycle *cycle = interfaceCycles[port->module];
            auto& schedules = output->gateSchedules[port];
            for (int priority = 0; priority < port->numPriorities; priority++) {
                auto schedule = new Output::Schedule();
                schedule->port = port;
                schedule->priority = priority;
                schedule->cycleStart = us(cycle->getCycleStart()).get();
                schedule->cycleDuration = us(cycle->getCycleDuration()).get();
                for (int i = 0; i < cycle->getSlotsUsed().size(); i++) {
                    if (priority == cycle->getSlotsUsed().at(i)) {
                        // find all slot durations for priority
                        // second loop, because there could be more slots than priorities
                        for (int j = 0; j < cycle->getSlotDuration().at(i).size(); j++) {
                            auto slotStart = us(cycle->getSlotStart(priority, j));
                            auto slotDuration = us(cycle->getSlotDuration(priority, j));
                            // slot with length 0 are not used
                            if (slotDuration == s(0))
                                continue;
                            schedule->slotStarts.push_back(s(slotStart).get());
                            schedule->slotDurations.push_back(s(slotDuration).get());
                        }
                    }
                }
                schedules.push_back(schedule);
            }
        }
    }
    for (auto& it : flows) {
        auto application = it.first->startApplication;
        // TODO
        bps datarate = Gbps(1.0);
        auto startTime = (us(it.second->getFlowFirstSendingTime()) - s((application->packetLength + B(12)) / datarate)).get();
        // KLUDGE TODO workaround a numerical accuracy problem that this number comes out negative sometimes
        if (startTime < 0)
            startTime = 0;
        output->applicationStartTimes[application] = startTime;
    }
    return output;
}

void Z3GateSchedulingConfigurator::configureNetwork(Network *net, context& ctx, solver& solver) const
{
    for(Flow *flw : net->getFlows()) {
           flw->modifyIfUsingCustomVal();
        flw->convertUnicastFlow();
        flw->setUpPeriods(flw->getPathTree()->getRoot());
    }

    for(Switch *swt : net->getSwitches()) {
        swt->setUpCycleSize(solver, ctx);
    }


    // On all network flows: Data given by the user will be converted to z3 values
    for(Flow *flw : net->getFlows()) {
        flw->toZ3(ctx);
        flw->setNumberOfPacketsSent(flw->getPathTree()->getRoot());
    }

    // On all network switches: Data given by the user will be converted to z3 values
    for(Switch *swt : net->getSwitches()) {
        swt->toZ3(ctx, solver);
    }

    // Sets up the hard constraint for each individual flow in the network
    net->setJitterUpperBoundRangeZ3(ctx, 25);
    net->secureHC(solver, ctx);
}

void Z3GateSchedulingConfigurator::generateSchedule(Network *net) const
{
//    EV_DEBUG << "Z3 Major Version: " << version.getMajor() << std::endl;
//    EV_DEBUG << "Z3 Full Version: " << version.getString() << std::endl;
//    EV_DEBUG << "Z3 Full Version std::string: " << version.getFullVersion() << std::endl;
    config cfg;
    cfg.set("model", "true");
    context *ctx = new context(cfg);
    solver solver(*ctx);     //Creating the solver to generate unknown values based on the given context

    this->configureNetwork(net, *ctx, solver);
    // net.loadNetwork(ctx, solver);


    // A switch is picked in order to evaluate the unknown values
    Switch *switch1 = nullptr;
    switch1 = (Switch *) net->getSwitches().at(0);
    // The duration of the cycle is given as a question to z3, so all the
    // constraints will have to be evaluated in order to z3 to know this cycle
    // duration
    std::shared_ptr<expr> switch1CycDuration = switch1->getCycle(0)->getCycleDurationZ3();


    /* find model for the constraints above */
//           model model = nullptr;
//           LocalTime time = LocalTime.now();

    EV_INFO << "Rules set. Checking solver." << std::endl;
    EV_INFO << solver << std::endl;
//           EV_DEBUG << std::string("Current time of the day: ") << time << std::endl;

    if (sat == solver.check())
    {
        model model = solver.get_model();
        EV_INFO << model << std::endl;
        expr v = model.eval(*switch1CycDuration, false);
//               if (v != nullptr)
//               {
            EV_INFO << "Model generated." << std::endl;

//                   try {
//                       PrintWriter out = new PrintWriter("log.txt");

                EV_DEBUG << "SCHEDULER LOG:\n\n" << std::endl;

                EV_DEBUG << "SWITCH LIST:" << std::endl;

                // For every switch in the network, store its information in the log
                for(Switch *auxSwt : net->getSwitches()) {
                    EV_DEBUG << std::string("  Switch name: ") << auxSwt->getName();
//                        EV_DEBUG << std::string("    Max packet size: ") << auxSwt->getMaxPacketSize();
//                        EV_DEBUG << std::string("    Port speed: ") << auxSwt->getPortSpeed();
//                        EV_DEBUG << std::string("    Time to Travel: ") << auxSwt->getTimeToTravel();
//                        EV_DEBUG << std::string("    Transmission time: ") << auxSwt->getTransmissionTime();
//                           EV_DEBUG << "    Cycle information -" << std::endl;
//                           EV_DEBUG << std::string("        First cycle start: ") + model.eval(((TSNSwitch *)auxSwt).getCycleStart(), false));
//                           EV_DEBUG << std::string("        Cycle duration: ") + model.eval(((TSNSwitch *)auxSwt).getCycleDuration(), false));
                    EV_DEBUG << "" << std::endl;
                    /*
                    for (Port port : ((TSNSwitch *)auxSwt).getPorts()) {
                        EV_DEBUG << std::string("        Port name (Virtual Index): ") + port->getName());
                        EV_DEBUG << std::string("        First cycle start: ") + model.eval(port->getCycle()->getFirstCycleStartZ3(), false));
                        EV_DEBUG << std::string("        Cycle duration: ") + model.eval(port->getCycle()->getCycleDurationZ3(), false));
                        EV_DEBUG << "" << std::endl;
                    }
                    */


                    // [EXTRACTING OUTPUT]: Obtaining the z3 output of the switch properties,
                    // converting it from string to double and storing in the objects

                    for (Port *port : ((Switch *)auxSwt)->getPorts()) {
                        port
                            ->getCycle()
                            ->setCycleStart(
                                this->stringToFloat(model.eval(*port->getCycle()->getFirstCycleStartZ3(), false).to_string())
                            );
                    }

                    // cycleDuration
                    for (Port *port : ((Switch *)auxSwt)->getPorts()) {
                        port
                            ->getCycle()
                            ->setCycleDuration(
                                this->stringToFloat(model.eval(*port->getCycle()->getCycleDurationZ3(), false).to_string())
                            );
                    }

                }

                EV_DEBUG << "" << std::endl;

                EV_DEBUG << "FLOW LIST:" << std::endl;
                //For every flow in the network, store its information in the log
                for(Flow *f : net->getFlows()) {
                    EV_DEBUG << std::string("  Flow name: ") + f->getName() << std::endl;
                    // EV_DEBUG << std::string("    Flow priority: ") + model.eval(f->getFlowPriority(), false) << std::endl;
                    EV_DEBUG << std::string("    Start dev. first t1: ") << model.eval(*f->getStartDevice()->getFirstT1TimeZ3(), false).to_string() << std::endl;
                    EV_DEBUG << std::string("    Start dev. HC: ") << model.eval(*f->getStartDevice()->getHardConstraintTimeZ3(), false).to_string() << std::endl;
                    EV_DEBUG << std::string("    Start dev. packet periodicity: ") << model.eval(*f->getStartDevice()->getPacketPeriodicityZ3(), false).to_string() << std::endl;

                    f->setFlowFirstSendingTime(stringToFloat(model.eval(*f->getStartDevice()->getFirstT1TimeZ3(), false).to_string()));

                    // IF FLOW IS UNICAST
                    /*
                    Observation: The flow is broken in smaller flow fragments.
                    In order to know the departure, arrival, scheduled times
                    and other properties of the flow the switch that the flow fragment
                    belongs to must be retrieved. The flow is then used on the switch
                    to find the port to its destination. The port and the flow fragment
                    can now be used to retrieve information about the flow.

                    The way in which unicast and publish subscribe flows are
                    structured here are different. So this process is done differently
                    in each case.
                    */
                    if(f->getType() == Flow::UNICAST) {
                        // TODO: Throw error. UNICAST data structure are not allowed at this point
                        // Everything should had been converted into the multicast model.
                    } else if(f->getType() == Flow::PUBLISH_SUBSCRIBE) { //IF FLOW IS PUB-SUB

                        /*
                         * In case of a publish subscribe flow, it is easier to
                         * traverse through the path three than iterate over the
                         * nodes as it could be done with the unicast flow.
                         */

                        PathTree *pathTree;
                        PathNode *pathNode;

                        pathTree = f->getPathTree();
                        pathNode = pathTree->getRoot();

                        EV_DEBUG << "    Flow type: Multicast" << std::endl;
                        std::vector<PathNode *> *auxNodes;
                        std::vector<FlowFragment *> *auxFlowFragments;
                        int auxCount = 0;

                        EV_DEBUG << "    List of leaves: ";
                        for(PathNode *node : *f->getPathTree()->getLeaves()) {
                            EV_DEBUG << ((Device *) node->getNode())->getName() + std::string(", ");
                        }
                        EV_DEBUG << "" << std::endl;
                        for(PathNode *node : *f->getPathTree()->getLeaves()) {
                            auxNodes = f->getNodesFromRootToNode((Device *) node->getNode());
                            auxFlowFragments = f->getFlowFromRootToNode((Device *) node->getNode());

                            EV_DEBUG << std::string("    Path to ") + ((Device *) node->getNode())->getName() + std::string(": ");
                            auxCount = 0;
                            for(PathNode *auxNode : *auxNodes) {
                                if(dynamic_cast<Device *>(auxNode->getNode())) {
                                    EV_DEBUG << ((Device *) auxNode->getNode())->getName() + std::string(", ");
                                } else if (dynamic_cast<Switch *>(auxNode->getNode())) {
                                    EV_DEBUG <<
                                        ((Switch *) auxNode->getNode())->getName() +
                                        std::string("(") +
                                        auxFlowFragments->at(auxCount)->getName() +
                                        "), ";
                                    auxCount++;
                                }

                            }
                            EV_DEBUG << "" << std::endl;
                        }
                        EV_DEBUG << "" << std::endl;

                        //Start the data storing and log printing process from the root
                        this->writePathTree(pathNode, model, *ctx);
                    }

                    EV_DEBUG << "" << std::endl;

                }
//
//                   } catch (FileNotFoundException e) {
//                       e.printStackTrace();
//                   }

//               } else
//               {
//                   EV_DEBUG << "Failed to evaluate" << std::endl;
//               }
    } else
    {
        EV_WARN << solver.unsat_core() << std::endl;
        throw cRuntimeError("The specified constraints might not be satisfiable.");
    }
}

double Z3GateSchedulingConfigurator::stringToFloat(std::string str) const
{
    int index = str.find('/');
    if (index != std::string::npos) {
        str = str.substr(3, str.length() - 1);
        index = str.find(' ');
        double val1 = atof(str.substr(0, index).c_str());
        double val2 = atof(str.substr(index + 1).c_str());
        return val1 / val2;
    }
    else
        return atof(str.c_str());
}

void Z3GateSchedulingConfigurator::writePathTree(PathNode *pathNode, model& model, context& ctx) const
{
    Switch *swt;
    std::shared_ptr<expr> indexZ3 = nullptr;

    if(dynamic_cast<Device *>(pathNode->getNode()) && (pathNode->getParent() != nullptr)) {
        EV_DEBUG << "    [END OF BRANCH]" << std::endl;

    }


    /*
     * Once given a node, an iteration through its children will begin. For
     * each switch children, there will be a flow fragment, and to each device
     * children, there will be an end of branch.
     *
     * The logic for storing and printing the data on the publish subscribe
     * flows is similar but easier than the unicast flows. The pathNode object
     * stores references to both flow fragment and switch, so no search is needed.
     */
    for(PathNode *child : pathNode->getChildren()) {
        if(dynamic_cast<Switch *>(child->getNode())) {

            for(FlowFragment *ffrag : child->getFlowFragments()) {
                EV_DEBUG << std::string("    Fragment name: ") << ffrag->getName() << std::endl;
                EV_DEBUG << std::string("        Fragment node: ") << ffrag->getNodeName() << std::endl;
                EV_DEBUG << std::string("        Fragment next hop: ") << ffrag->getNextHop() << std::endl;
                EV_DEBUG << std::string("        Fragment priority: ") << model.eval(*ffrag->getFragmentPriorityZ3(), false) << std::endl;
                for(int index = 0; index < ((Switch *) child->getNode())->getPortOf(ffrag->getNextHop())->getCycle()->getNumOfSlots(); index++) {
                    indexZ3 = std::make_shared<expr>(ctx.int_val(index));
                    EV_DEBUG << std::string("        Fragment slot start ") << index << std::string(": ")
                            << this->stringToFloat(
                                    model.eval(*((Switch *) child->getNode())
                                           ->getPortOf(ffrag->getNextHop())
                                           ->getCycle()
                                           ->slotStartZ3(ctx, *ffrag->getFragmentPriorityZ3(), *indexZ3)
                                           , false).to_string()) << std::endl;
                    EV_DEBUG << std::string("        Fragment slot duration ") << index << std::string(" : ")
                             << this->stringToFloat(
                                 model.eval(*((Switch *) child->getNode())
                                            ->getPortOf(ffrag->getNextHop())
                                            ->getCycle()
                                            ->slotDurationZ3(ctx, *ffrag->getFragmentPriorityZ3(), *indexZ3)
                                            , false).to_string()) << std::endl;

                }

                EV_DEBUG << "        Fragment times-" << std::endl;
                ffrag->getParent()->addToTotalNumOfPackets(ffrag->getNumOfPacketsSent());

                for(int i = 0; i < ffrag->getParent()->getNumOfPacketsSent(); i++) {
                        EV_DEBUG << std::string("On " + ffrag->getName() + std::string(" - ")) <<
                         ((Switch *)child->getNode())->departureTime(ctx, i, ffrag)->to_string() << std::string(" - ") <<
                           ((Switch *)child->getNode())->scheduledTime(ctx, i, ffrag)->to_string() << std::endl;
                    // EV_DEBUG << ((TSNSwitch *)child->getNode())->departureTime(ctx, i, ffrag).to_string() << std::endl;
                    // EV_DEBUG << ((TSNSwitch *)child->getNode())->arrivalTime(ctx, i, ffrag).to_string() << std::endl;
                    // EV_DEBUG << ((TSNSwitch *)child->getNode())->scheduledTime(ctx, i, ffrag).to_string() << std::endl;

                    if(i < ffrag->getParent()->getNumOfPacketsSent()) {
                        EV_DEBUG << std::string("          (") << std::to_string(i) << std::string(") Fragment departure time: ") << this->stringToFloat(model.eval(*((Switch *) child->getNode())->departureTime(ctx, i, ffrag), false).to_string()) << std::endl;
                        EV_DEBUG << std::string("          (") << std::to_string(i) << std::string(") Fragment arrival time: ") << this->stringToFloat(model.eval(*((Switch *) child->getNode())->arrivalTime(ctx, i, ffrag), false).to_string()) << std::endl;
                        EV_DEBUG << std::string("          (") << std::to_string(i) << std::string(") Fragment scheduled time: ") << this->stringToFloat(model.eval(*((Switch *) child->getNode())->scheduledTime(ctx, i, ffrag), false).to_string()) << std::endl;
                        EV_DEBUG << "          ----------------------------" << std::endl;
                    }

                    ffrag->setFragmentPriority(
                        atoi(
                            model.eval(*ffrag->getFragmentPriorityZ3(), false).to_string().c_str()
                        )
                    );

                    ffrag->addDepartureTime(
                        this->stringToFloat(
                            model.eval(*((Switch *) child->getNode())->departureTime(ctx, i, ffrag) , false).to_string()
                        )
                    );
                    ffrag->addArrivalTime(
                        this->stringToFloat(
                            model.eval(*((Switch *) child->getNode())->arrivalTime(ctx, i, ffrag) , false).to_string()
                        )
                    );
                    ffrag->addScheduledTime(
                        this->stringToFloat(
                            model.eval(*((Switch *) child->getNode())->scheduledTime(ctx, i, ffrag) , false).to_string()
                        )
                    );

                }

                swt = (Switch *) child->getNode();

                for (Port *port : ((Switch *) swt)->getPorts()) {

                    auto flowFragments = port->getFlowFragments();
                    if(std::find(flowFragments.begin(), flowFragments.end(), ffrag) == flowFragments.end()) {
                        continue;
                    }

                    std::vector<double> listOfStart;
                    std::vector<double> listOfDuration;


                    for(int index = 0; index < ((Switch *) child->getNode())->getPortOf(ffrag->getNextHop())->getCycle()->getNumOfSlots(); index++) {
                        indexZ3 = std::make_shared<expr>(ctx.int_val(index));

                        listOfStart.push_back(
                            this->stringToFloat(model.eval(
                                *((Switch *) child->getNode())
                                ->getPortOf(ffrag->getNextHop())
                                ->getCycle()->slotStartZ3(ctx, *ffrag->getFragmentPriorityZ3(), *indexZ3) , false).to_string())
                        );
                        listOfDuration.push_back(
                            this->stringToFloat(model.eval(
                                *((Switch *) child->getNode())
                                ->getPortOf(ffrag->getNextHop())
                                ->getCycle()->slotDurationZ3(ctx, *ffrag->getFragmentPriorityZ3(), *indexZ3) , false).to_string())
                        );
                    }

                    port->getCycle()->addSlotUsed(
                        (int) this->stringToFloat(model.eval(*ffrag->getFragmentPriorityZ3(), false).to_string()),
                        listOfStart,
                        listOfDuration
                    );
                }

            }

            this->writePathTree(child, model, ctx);
        }
    }

}

} // namespace inet

