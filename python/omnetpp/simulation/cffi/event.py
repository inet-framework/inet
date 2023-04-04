import cppyy
import re

from inet.common import *
from inet.simulation.cffi.inet import *
from inet.simulation.cffi.omnetpp import *

cppyy.cppdef("""
#include "omnetpp/cevent.h"
#include <functional>

namespace omnetpp {

class cFunctionalEvent : public cEvent {
private:
    std::function<void()> function;
public:
    cFunctionalEvent(std::function<void()> function) : cEvent(nullptr) {
        this->function = function;
    }
    virtual cEvent *dup() const override { return nullptr; }
    virtual cObject *getTargetObject() const override { return nullptr; }
    virtual void execute() override {
        function();
    }
};

void doit(cEvent *event) {
    event->execute();
}

}
""")

cFunctionalEvent = cppyy.gbl.omnetpp.cFunctionalEvent

class PythonEvent(cEvent):
    def __init__(self):
        super().__init__("")

    def dup(self):
        return cppyy.nullptr

    def getTargetObject(self):
        return cppyy.nullptr

    def execute(self):
        print("PythonEvent execute")

def test_python_event():
    cppyy.gbl.omnetpp.doit(PythonEvent())

def test_function_event():
    def f():
        print("cFunctionalEvent execute")
    pe = cFunctionalEvent(f)
    pe.execute()
