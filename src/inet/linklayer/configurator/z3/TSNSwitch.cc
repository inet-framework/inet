#include "inet/linklayer/configurator/z3/FlowFragment.h"
#include "inet/linklayer/configurator/z3/TSNSwitch.h"

namespace inet {

int Cycle::instanceCounter = 0;
int Device::indexCounter = 0;
int Flow::instanceCounter = 0;
int TSNSwitch::indexCounter = 0;
int assertionCount = 0;

void TSNSwitch::addToFragmentList(FlowFragment *flowFrag)
{
    int index = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();

    /*
    System.out.println(std::string("Current node: ") + flowFrag->getNodeName());
    System.out.println(std::string("Next hop: ") + flowFrag->getNextHop());
    System.out.println(std::string("Index of port: ") + index);
    System.out.print("Connects to: ");
    for(std::string connect : this->connectsTo) {
        System.out.print(connect + std::string(", "));
    }

    System.out.println("");
    System.out.println("------------------");

    */

    this->ports.at(index)->addToFragmentList(flowFrag);
}

std::shared_ptr<expr> TSNSwitch::arrivalTime(context& ctx, int auxIndex, FlowFragment *flowFrag)
{
    std::shared_ptr<expr> index = std::make_shared<expr>(ctx.int_val(auxIndex));
    int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();

    return this->ports.at(portIndex)->arrivalTime(ctx, auxIndex, flowFrag);
}

std::shared_ptr<expr> TSNSwitch::departureTime(context& ctx, z3::expr index, FlowFragment *flowFrag)
{
    int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();
    return this->ports.at(portIndex)->departureTime(ctx, index, flowFrag);
}

std::shared_ptr<expr> TSNSwitch::departureTime(context& ctx, int auxIndex, FlowFragment *flowFrag)
{
    expr index = ctx.int_val(auxIndex);
    int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();
    return this->ports.at(portIndex)->departureTime(ctx, index, flowFrag);
}

std::shared_ptr<expr> TSNSwitch::scheduledTime(context& ctx, int auxIndex, FlowFragment *flowFrag)
{
    // z3::expr index = ctx.int_val(auxIndex);
    int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();
    return this->ports.at(portIndex)->scheduledTime(ctx, auxIndex, flowFrag);
}

}
