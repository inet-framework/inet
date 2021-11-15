import os
import curses.ascii

COLOR_RED = "\033[1;31m"
COLOR_YELLOW = "\033[1;33m"
COLOR_CYAN = "\033[0;36m"
COLOR_GREEN = "\033[0;32m"
COLOR_RESET = "\033[0;0m"

def get_full_path(resource):
    return os.path.abspath(os.environ['INET_ROOT'] + "/" + resource)

def flatten(list):
    return [item for sublist in list for item in sublist]

def repr(object):
    return f"{object.__class__.__name__}({', '.join([f'{prop}={value}' for prop, value in object.__dict__.items()])})"

def coalesce(*values):
    """Return the first non-None value or None if all values are None"""
    return next((v for v in values if v is not None), None)

def convert_to_seconds(s):
    seconds_per_unit = {"ns": 1E-9, "us": 1E-6, "ms": 1E-3, "s": 1, "m": 60, "h": 3600, "d": 86400, "w": 604800}
    if curses.ascii.isdigit(s[-2]):
        return float(s[:-1]) * seconds_per_unit[s[-1:]]
    else:
        return float(s[:-2]) * seconds_per_unit[s[-2:]]
