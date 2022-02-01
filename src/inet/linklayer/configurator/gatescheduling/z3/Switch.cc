//
// Copyright (C) 2021 by original authors
//
// This file is copied from the following project with the explicit permission
// from the authors: https://github.com/ACassimiro/TSNsched
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/gatescheduling/z3/FlowFragment.h"
#include "inet/linklayer/configurator/gatescheduling/z3/Switch.h"

namespace inet {

int Cycle::instanceCounter = 0;
int Device::indexCounter = 0;
int Flow::instanceCounter = 0;
int Switch::indexCounter = 0;
int assertionCount = 0;

void Switch::addToFragmentList(FlowFragment *flowFrag)
{
    int index = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();
    this->ports.at(index)->addToFragmentList(flowFrag);
}

std::shared_ptr<expr> Switch::arrivalTime(context& ctx, int auxIndex, FlowFragment *flowFrag)
{
    std::shared_ptr<expr> index = std::make_shared<expr>(ctx.int_val(auxIndex));
    int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();

    return ports.at(portIndex)->arrivalTime(ctx, auxIndex, flowFrag);
}

std::shared_ptr<expr> Switch::departureTime(context& ctx, z3::expr index, FlowFragment *flowFrag)
{
    int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();
    return ports.at(portIndex)->departureTime(ctx, index, flowFrag);
}

std::shared_ptr<expr> Switch::departureTime(context& ctx, int auxIndex, FlowFragment *flowFrag)
{
    expr index = ctx.int_val(auxIndex);
    int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();
    return ports.at(portIndex)->departureTime(ctx, index, flowFrag);
}

std::shared_ptr<expr> Switch::scheduledTime(context& ctx, int auxIndex, FlowFragment *flowFrag)
{
    int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();
    return ports.at(portIndex)->scheduledTime(ctx, auxIndex, flowFrag);
}

}
