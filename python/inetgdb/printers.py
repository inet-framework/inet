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

# initially enabled/disabled the inet pretty printers
inet_pp_enabled = True

class IPv4AddressPrinter:
    "Print an IPv4Address"

    def __init__(self, val):
        self.val = val

    @staticmethod
    def addrToString(addr):
        if (addr == 0):
            return "<unspec>"
        return str((addr >> 24)&0xFF) + "." + str((addr >> 16)&0xFF) + "." + str((addr >> 8)&0xFF) + "." + str(addr&0xFF)

    def to_string(self):
        addr = self.val['addr']
        return self.addrToString(addr)


class IPv6AddressPrinter:
    "Print an IPv6Address"

    def __init__(self, val):
        self.val = val

    @staticmethod
    def addrToString(d):
        if ((d[0]|d[1]|d[2]|d[3]) == 0):
            return "<unspec>"

        # convert to 16-bit groups
        groups = [ (d[0]>>16)&0xffff, d[0]&0xffff, (d[1]>>16)&0xffff, d[1]&0xffff, (d[2]>>16)&0xffff, d[2]&0xffff, (d[3]>>16)&0xffff, d[3]&0xffff ]

        # find longest sequence of zeros in groups[]
        start = end = 0
        beg = -1
        for i in range(0, 8):
            if (beg==-1 and groups[i]==0):
                # begin counting
                beg = i
            else:
                if (beg != -1 and groups[i] != 0):
                    # end counting
                    if (i-beg >= 2 and i-beg > end-start):
                        start = beg
                        end = i
                    beg = -1

        # check last zero-seq
        if (beg!=-1 and beg<=6 and 8-beg > end-start):
            start = beg
            end = 8

        if (start==0 and end==8):
            return "::0";  # the unspecified address is a special case

        # print groups, replacing gap with "::"
        os = "";
        for i in range(0, start):
            if i != 0:
                os  += ":"
            os += "%x" % (groups[i])
        if (start != end):
            os += "::"
        for j in range(end, 8):
            if j != end:
                os  += ":"
            os += "%x" % (groups[j])
        return os

    def to_string(self):
        d = self.val['d']
        return self.addrToString(d)


class IPvXAddressPrinter:
    "Print an IPvXAddress"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        isv6 = self.val['isv6']
        d = self.val['d']
        if (isv6):
            return IPv6AddressPrinter.addrToString(d)
        return IPv4AddressPrinter.addrToString(d[0])


class MACAddressPrinter:
    "Print a MACAddress"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        addr = self.val['address']
        return "%02X:%02X:%02X:%02X:%02X:%02X" % ((addr >> 40)&0xFF, (addr >> 32)&0xFF, (addr >> 24)&0xFF, (addr >> 16)&0xFF, (addr >> 8)&0xFF, addr&0xFF)



#########################################################################################

# A "regular expression" printer which conforms to the
# "SubPrettyPrinter" protocol from gdb.printing.
class InetSubPrinter(object):
    def __init__(self, name, function):
        global inet_pp_enabled
        super(InetSubPrinter, self).__init__()
        self.name = name
        self.function = function
        self.enabled = inet_pp_enabled

    def invoke(self, value):
        if not self.enabled:
            return None
        return self.function(value)

# A pretty-printer that conforms to the "PrettyPrinter" protocol from
# gdb.printing.  It can also be used directly as an old-style printer.
class InetPrinter(object):
    def __init__(self, name):
        global inet_pp_enabled
        super(InetPrinter, self).__init__()
        self.name = name
        self.subprinters = []   # used by 'gdb command: info pretty-printer'
        self.lookup = {}
        self.enabled = inet_pp_enabled

    def add(self, name, function):
        printer = InetSubPrinter(name, function)
        self.subprinters.append(printer)
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
#            if val.type.code == gdb.TYPE_CODE_REF or val.type.code == gdb.TYPE_CODE_PTR:
            if val.type.code == gdb.TYPE_CODE_PTR:
                if (long(val.address) == 0):
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
    inet_printer.add('IPv6Address', IPv6AddressPrinter)
    inet_printer.add('IPvXAddress', IPvXAddressPrinter)
    inet_printer.add('MACAddress', MACAddressPrinter)


build_inet_dictionary()

# end
