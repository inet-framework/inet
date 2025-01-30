# invoke from LLDB with: command script import <INET_ROOT>/python/lldb/inet/formatter.py

import lldb
from decimal import *

def call_str(value):
    str_expr = value.EvaluateExpression("str()")
    error = str_expr.GetError()
    if error.Success():
        summary = str_expr.GetSummary()
        if summary is not None:
            return summary.strip('"')
    return "<error calling str(): {}>".format(error.GetCString())

def clocktime_SummaryProvider(value, internal_dict):
    return call_str(value) + " s"

def unit_SummaryProvider(value, internal_dict):
    return call_str(value)

def __lldb_init_module(debugger, internal_dict):
    print("Initializing LLDB type formatters for INET")
    debugger.HandleCommand("type summary add -w inet -v -F " + __name__ + ".clocktime_SummaryProvider inet::clocktime_t")
    debugger.HandleCommand("type summary add -w inet -v -F " + __name__ + ".clocktime_SummaryProvider inet::ClockTime")
    debugger.HandleCommand("type summary add -x -w inet -v -F " + __name__ + ".unit_SummaryProvider '^inet::units::values::.*$'")
    debugger.HandleCommand("type category enable inet")

