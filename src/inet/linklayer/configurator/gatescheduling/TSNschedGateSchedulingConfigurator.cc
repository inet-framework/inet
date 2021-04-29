//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/linklayer/configurator/gatescheduling/TSNschedGateSchedulingConfigurator.h"

#include <fstream>

namespace inet {

Define_Module(TSNschedGateSchedulingConfigurator);

std::string TSNschedGateSchedulingConfigurator::getJavaName(std::string name) const
{
    std::replace(name.begin(), name.end(), '.', '_');
    std::replace(name.begin(), name.end(), '[', '_');
    std::replace(name.begin(), name.end(), ']', '_');
    return name;
}

void TSNschedGateSchedulingConfigurator::writeJavaCode(const Input& input, std::string fileName) const
{
    std::string className = fileName.substr(0, fileName.find('.'));
    std::ofstream javaFile;
    javaFile.open(fileName.c_str());
    javaFile <<
"import java.util.*;\n"
"import java.io.*;\n"
"import schedule_generator.*;\n"
"\n"
"public class " << className << " {\n"
"   public static void main(String[] args) {\n";

    for (auto device : input.devices) {
        auto application = *std::find_if(input.applications.begin(), input.applications.end(), [&] (Input::Application *application) { return application->device == device; });
        if (application == nullptr)
            javaFile <<
"       Device " << getJavaName(device->module->getFullName()) << " = new Device(0, 0, 0, 0);\n";
        else
            javaFile <<
"       Device " << getJavaName(device->module->getFullName()) << " = new Device("
                 << (application->packetInterval * 1000000).dbl() << "f, 0, " << (application->maxLatency * 1000000).dbl()
                 << "f, " << b(application->packetLength + B(12)).get() << ");\n";
    }

    for (auto cycle : input.cycles) {
        javaFile <<
"       Cycle " << getJavaName(cycle->name) << " = new Cycle(" << (gateCycleDuration * 1000000).dbl() << ", 0, " << (gateCycleDuration * 1000000).dbl() << ");\n";
    }

    for (auto switch_ : input.switches) {
        // KLUDGE TODO use per port parameters
        double datarate = 1E+9 / 1000000;
        double propagationTime = 50E-9 * 1000000;
        // TODO KLUDGE this is a wild guess
        double guardBand = 0;
        for (auto flow : input.flows) {
            double v = b(flow->startApplication->packetLength).get() / datarate;
            if (guardBand < v)
                guardBand = v;
        }
        javaFile << // TODO parameters
"       TSNSwitch " << getJavaName(switch_->module->getFullName()) << " = new TSNSwitch(\"" << switch_->module->getFullName() << "\", "
                    << 1500 << ", " << propagationTime << "f, " << datarate << ", " << guardBand << "f, 0, " << (gateCycleDuration * 1000000).dbl() << ");\n";
    }

    for (auto switch_ : input.switches) {
        for (auto port : switch_->ports) {
            javaFile <<
"       " << getJavaName(switch_->module->getFullName()) << ".createPort(" << getJavaName(port->endNode->module->getFullName()) << ", "
          << getJavaName(port->cycle->name) << ");\n";
        }
    }

    for (auto flow : input.flows) {
        javaFile <<
"       Flow " << getJavaName(flow->name) << " = new Flow(Flow.UNICAST);\n"
"       " << getJavaName(flow->name) << ".setFixedPriority(true);\n"
"       " << getJavaName(flow->name) << ".setPriorityValue(" << flow->startApplication->priority << ");\n"
"       " << getJavaName(flow->name) << ".setStartDevice(" << getJavaName(flow->startApplication->device->module->getFullName()) << ");\n"
        "       " << getJavaName(flow->name) << ".setEndDevice(" << getJavaName(flow->endDevice->module->getFullName()) << ");\n";
        for (int j = 0; j < flow->pathFragments.size(); j++) {
            auto pathFragment = flow->pathFragments[j];
            for (int k = 0; k < pathFragment->networkNodes.size(); k++) {
                auto networkNode = pathFragment->networkNodes[k];
                if (dynamic_cast<Input::Switch *>(networkNode))
                    javaFile <<
"       " << getJavaName(flow->name) << ".addToPath(" << getJavaName(networkNode->module->getFullName()) << ");\n";
            }
        }
    }

    javaFile <<
"       Network network = new Network();\n";
    for (auto switch_ : input.switches) {
        javaFile <<
"       network.addSwitch(" << getJavaName(switch_->module->getFullName()) << ");\n";
    }
    for (auto flow : input.flows) {
        javaFile <<
"       network.addFlow(" << getJavaName(flow->name) << ");\n";
    }

    javaFile <<
"       ScheduleGenerator scheduleGenerator = new ScheduleGenerator();\n"
"       scheduleGenerator.generateSchedule(network);\n";

    javaFile <<
"   }\n"
"}\n";
    javaFile.close();
}

void TSNschedGateSchedulingConfigurator::compileJavaCode(std::string fileName) const
{
    std::string classpath = "${TSNSCHED_HOME}/libs/com.microsoft.z3.jar:${TSNSCHED_HOME}/libs/ScheduleGeneratorHyperCycle.jar:${TSNSCHED_HOME}/libs/ScheduleGeneratorMicroCycle.jar";
    std::string command = std::string("javac -classpath ") + classpath + " " + fileName;
    std::system(command.c_str());
}

void TSNschedGateSchedulingConfigurator::executeJavaCode(std::string fileName) const
{
    std::string classpath = ".:${TSNSCHED_HOME}/libs/com.microsoft.z3.jar:${TSNSCHED_HOME}/libs/ScheduleGeneratorHyperCycle.jar:${TSNSCHED_HOME}/libs/ScheduleGeneratorMicroCycle.jar";
    std::string command = std::string("java -classpath ") + classpath + " " + fileName;
    std::system(command.c_str());
}

TSNschedGateSchedulingConfigurator::Output *TSNschedGateSchedulingConfigurator::readGateScheduling(std::string directoryName) const
{
    auto output = new Output();
    return output;
}

TSNschedGateSchedulingConfigurator::Output *TSNschedGateSchedulingConfigurator::computeGateScheduling(const Input& input) const
{
    opp_mkdir("XMLExporterFiles", 0755);
    std::string className = getSimulation()->getSystemModule()->getComponentType()->getName();
    std::string classFileName = className + ".class";
    std::string sourceFileName = className + ".java";
    writeJavaCode(input, sourceFileName);
    compileJavaCode(sourceFileName);
    executeJavaCode(className);
    unlink(sourceFileName.c_str());
    unlink(classFileName.c_str());
    return readGateScheduling("XMLExporterFiles");
}

} // namespace inet

