# this imports the modifiedplot.py file from the inet/python folder, based on relative path (it doesn't need to be in the python path)
# this is a workaround when using omnetpp 6.0 that doesn't yet contain the python path feature
# delete this file when the python path feature part of a release
import importlib.util
import sys

def load_module(file_name, module_name):
    spec = importlib.util.spec_from_file_location(module_name, file_name)
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)
    return module

file_name = '../../../python/modifiedplot.py'
module_name = 'modifiedplot'

load_module(file_name, module_name)