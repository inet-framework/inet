import dataset
import importlib
import os
import subprocess
import sys

import omnetpp.scave

import inet.documentation
import inet.fingerprinttest
import inet.simulation

from inet.documentation import *
from inet.fingerprinttest import *
from inet.simulation import *

from IPython import get_ipython
ipython = get_ipython()
ipython.magic("load_ext autoreload")
ipython.magic("autoreload 2")
