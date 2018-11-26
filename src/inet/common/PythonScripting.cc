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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "PythonScripting.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(INET, m) {
    py::class_<inet::InterfaceEntry>(m, "InterfaceEntry")
            .def("join_multicast_group", [](inet::InterfaceEntry &self, const char *address) {
                self.getProtocolData<inet::Ipv4InterfaceData>()->joinMulticastGroup(inet::Ipv4Address(address));
            });


    py::class_<cModule>(m, "cModule")
            .def("get_network_interface", [](cModule &self, const char *name) {
                return inet::L3AddressResolver().findInterfaceTableOf(&self)->getInterfaceByName(name);
            });
    py::class_<cSimulation>(m, "cSimulation")
            .def_static("get_active_simulation", &cSimulation::getActiveSimulation, py::return_value_policy::reference)
            .def("get_module_by_path", &cSimulation::getModuleByPath, py::return_value_policy::reference)
            .def("get_network_node", &cSimulation::getModuleByPath, py::return_value_policy::reference);

}

extern "C" {
void createModule(const char *parentModulePath, const char *submoduleName, const char *moduleTypeName, bool vector) {
    cModuleType *moduleType = cModuleType::get(moduleTypeName);
    if (moduleType == nullptr)
        throw cRuntimeError("module type '%s' is not found", moduleType);
    cModule *parentModule = getSimulation()->getSystemModule()->getModuleByPath(parentModulePath);
    if (parentModule == nullptr)
        throw cRuntimeError("parent module '%s' is not found", parentModulePath);
    cModule *module = nullptr;
    if (vector) {
        cModule *submodule = parentModule->getSubmodule(submoduleName, 0);
        int submoduleIndex = submodule == nullptr ? 0 : submodule->getVectorSize();
        module = moduleType->create(submoduleName, parentModule, submoduleIndex + 1, submoduleIndex);
    }
    else {
        module = moduleType->create(submoduleName, parentModule);
    }
    module->finalizeParameters();
    module->buildInside();
    module->callInitialize();
}
}

namespace inet {

Define_Module(PythonScripting);

void PythonScripting::initialize(int stage)
{
    if (stage == INITSTAGE_LAST) {
        Py_Initialize();
        l = PyDict_New();

        m = PyImport_AddModule("__main__");
        d = PyModule_GetDict(m);

        PyRun_String("from cffi import FFI", Py_file_input, d, d);
        PyRun_String("ffi = FFI()\n"
                    "ffi.cdef('void createModule(const char *, const char *, const char *, bool);')\n"
                "C = ffi.dlopen(None)", Py_file_input, d, d);

        PyRun_String("FES = list()\n"
                     "def at(time, event):\n"
                     "    print('at ' + str(time), flush=True)\n"
                     "    global FES\n"
                     "    FES.append((float(time), event))\n"
                     "    FES.sort(key=lambda tup: tup[0])\n"
                     "    print('at done',flush=True)\n", Py_file_input, d, d);
        PyRun_String("def first_time():\n"
                     "    print('in ft', flush=True)\n"
                     "    global FES\n"
                     "    ft = FES[0][0] if len(FES) else None\n"
                     "    print(ft, flush=True)\n"
                     "    return ft\n", Py_file_input, d, d);
        PyRun_String("def exec_first():\n"
                     "    print('exec first', flush=True)\n"
                     "    global FES\n"
                     "    first = FES[0]\n"
                     "    FES = FES[1:]\n"
                     "    print('before exec', flush=True)\n"
                     "    first[1]()\n"
                     "    print('after exec', flush=True)\n"
                     "    print('exec done')\n", Py_file_input, d, d);
        PyRun_String("def create_module(parent, submodule, type, vector):\n"
                     "    C.createModule(parent.encode('utf8'), submodule.encode('utf8'), type.encode('utf8'), vector)\n", Py_file_input, d, d);

        PyRun_String("from INET import *", Py_file_input, d, d);
        PyRun_String("simulation = cSimulation.get_active_simulation()", Py_file_input, d, d);

        PyRun_String(par("script").stringValue(), Py_file_input, d, d);

        msg = new cMessage("next python event");

        PyObject *firstTime = PyRun_String("first_time()", Py_eval_input, d, d);
        if (PyFloat_Check(firstTime)) {
            double t = PyFloat_AsDouble(firstTime);
            std::cout << "T: " << t << std::endl;
            if (t >= 0)
                scheduleAt(t, msg);
        }
    }
}

void PythonScripting::handleMessage(cMessage *msg)
{
    PyRun_String("exec_first()", Py_eval_input, d, d);

    PyObject *firstTime = PyRun_String("first_time()", Py_eval_input, d, d);

    if (PyFloat_Check(firstTime)) {
        double t = PyFloat_AsDouble(firstTime);
        std::cout << "T: " << t << std::endl;
        if (t >= 0)
            scheduleAt(t, msg);
    }
}


void PythonScripting::finish()
{
    Py_Finalize();
    delete msg;
}

} //namespace
