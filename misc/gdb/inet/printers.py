#
# Pretty-printers for INET classes.
#
# Copyright (C) 2012 OpenSim Ltd.
#
# This file is distributed WITHOUT ANY WARRANTY. See the file
# `license' for details on this and other legal matters.
#
# @author: Zoltan Bojthe
#

import gdb
import pprint

# Try to use the new-style pretty-printing if available.
_use_gdb_pp = True
try:
    import gdb.printing
except ImportError:
    _use_gdb_pp = False


class IPv4AddressPrinter:
    "Print an IPv4Address"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        addr = self.val['addr']
        if (addr == 0):
            return "<unspec>"
        return str((addr >> 24)&0xFF) + "." + str((addr >> 16)&0xFF) + "." + str((addr >> 8)&0xFF) + "." + str(addr&0xFF)


#########################################################################################

# A "regular expression" printer which conforms to the
# "SubPrettyPrinter" protocol from gdb.printing.
class InetSubPrinter(object):
    def __init__(self, name, function):
        super(InetSubPrinter, self).__init__()
        self.name = name
        self.function = function
        self.enabled = True

    def invoke(self, value):
        if not self.enabled:
            return None
        return self.function(value)

# A pretty-printer that conforms to the "PrettyPrinter" protocol from
# gdb.printing.  It can also be used directly as an old-style printer.
class InetPrinter(object):
    def __init__(self, name):
        super(InetPrinter, self).__init__()
        self.name = name
        self.lookup = {}
        self.enabled = True

    def add(self, name, function):
        printer = InetSubPrinter(name, function)
        self.lookup[name] = printer

    @staticmethod
    def get_basic_type(type):
        # If it points to a reference, get the reference.
        if type.code == gdb.TYPE_CODE_REF or type.code == gdb.TYPE_CODE_PTR:
            type = type.target()

        # Get the unqualified type, stripped of typedefs.
        type = type.unqualified().strip_typedefs()

        return type.tag

    def __call__(self, val):
        typename = self.get_basic_type(val.type)
        #print "BASIC TYPE OF '%s' type IS '%s'" % (val.type, typename)
        if not typename:
            return None

        #print "lookup printer for '%s' type" % (typename)
        if typename in self.lookup:
            if val.type.code == gdb.TYPE_CODE_REF or val.type.code == gdb.TYPE_CODE_PTR:
                if (long(val) == 0):
                    return None
                val = val.dereference()
            return self.lookup[typename].invoke(val)

        # Cannot find a pretty printer.  Return None.
        return None


inet_printer = None


def register_inet_printers(obj):
    "Register OMNeT++ pretty-printers with objfile Obj."

    global _use_gdb_pp
    global inet_printer

    if _use_gdb_pp:
        gdb.printing.register_pretty_printer(obj, inet_printer)
    else:
        if obj is None:
            obj = gdb
        obj.pretty_printers.append(inet_printer)


def build_inet_dictionary():
    global inet_printer

    inet_printer = InetPrinter("inet")

    inet_printer.add('IPv4Address', IPv4AddressPrinter)


build_inet_dictionary()

# end
