import sys
import os

# add the pretty printer classes to the system class path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))+"/misc/gdb")

if 'register_inet_printers' in dir():
    print('inet pretty printers already initialized.')
else:
    from inetgdb.printers import register_inet_printers
    register_inet_printers(None)
    print('Pretty printers initialized: inet')
