import curses.ascii
import glob
import hashlib
import io
import IPython
import logging
import os
import pickle
import re
import signal
import subprocess
import sys
import threading
import time

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

COLOR_GRAY = "\033[38;20m"
COLOR_RED = "\033[1;31m"
COLOR_YELLOW = "\033[1;33m"
COLOR_CYAN = "\033[0;36m"
COLOR_GREEN = "\033[0;32m"
COLOR_MAGENTA = "\033[1;35m"
COLOR_RESET = "\033[0;0m"

STDOUT_LEVEL = 25  # between INFO (20) and WARNING (30)
STDERR_LEVEL = 35  # between WARNING (30) and ERROR (40)

def enable_autoreload():
    ipython = IPython.get_ipython()
    ipython.magic("load_ext autoreload")
    ipython.magic("autoreload 2")

_logging_initialized = False

def initialize_logging(log_level, external_command_log_level):
    global _logging_initialized
    formatter = ColoredLoggingFormatter()
    handler = logging.StreamHandler()
    handler.setFormatter(formatter)
    logger = logging.getLogger()
    logger.setLevel(log_level)
    logger.handlers = []
    logger.addHandler(handler)
    logging.getLogger("make").setLevel(external_command_log_level)
    logging.getLogger("opp_featuretool").setLevel(external_command_log_level)
    logging.getLogger("opp_makemake").setLevel(external_command_log_level)
    logging.getLogger("opp_run").setLevel(external_command_log_level)
    logging.getLogger("opp_run_dbg").setLevel(external_command_log_level)
    logging.getLogger("opp_run_release").setLevel(external_command_log_level)
    logging.getLogger("opp_run_sanitize").setLevel(external_command_log_level)
    logging.getLogger("opp_test").setLevel(external_command_log_level)
    logging.addLevelName(STDOUT_LEVEL, "STDOUT")
    logging.addLevelName(STDERR_LEVEL, "STDERR")
    def stdout(self, message, *args, **kwargs):
        if self.isEnabledFor(STDOUT_LEVEL):
            self._log(STDOUT_LEVEL, message, args, **kwargs)
    def stderr(self, message, *args, **kwargs):
        if self.isEnabledFor(STDERR_LEVEL):
            self._log(STDERR_LEVEL, message, args, **kwargs)
    logging.Logger.stdout = stdout
    logging.Logger.stderr = stderr
    _logging_initialized = True

def ensure_logging_initialized(log_level, external_command_log_level):
    if not _logging_initialized:
        initialize_logging(log_level, external_command_log_level)
        return True
    else:
        return False

def get_logging_formatter():
    logger = logging.getLogger()
    return logger.handlers[0].formatter

def get_omnetpp_relative_path(path):
    return os.path.abspath(os.path.join(os.environ["__omnetpp_root_dir"], path)) if "__omnetpp_root_dir" in os.environ else None

def get_inet_relative_path(path):
    return os.path.join(os.environ["INET_ROOT"], path)

def get_workspace_path(path):
    return os.path.join(os.path.realpath(get_omnetpp_relative_path("..")), path)

def flatten(list):
    return [item for sublist in list for item in sublist]

def repr(object, properties=None):
    return f"{object.__class__.__name__}({', '.join([f'{prop}={value}' for prop, value in object.__dict__.items() if properties is None or prop in properties])})"

def coalesce(*values):
    """Return the first non-None value or None if all values are None"""
    return next((v for v in values if v is not None), None)

def convert_to_seconds(s):
    seconds_per_unit = {"ps": 1E-12, "ns": 1E-9, "us": 1E-6, "ms": 1E-3, "s": 1, "second": 1, "m": 60, "min": 60, "h": 3600, "hour": 3600, "d": 86400, "day": 86400, "w": 604800, "week": 604800}
    match = re.match(r"(-?[0-9]*\.?[0-9]*) *([a-zA-Z]+)", s)
    return float(match.group(1)) * seconds_per_unit[match.group(2)]

def write_object(file_name, object):
    with open(file_name, "wb") as file:
        pickle.dump(object, file)

def read_object(file_name):
    with open(file_name, "rb") as file:
        return pickle.load(file)

def hex_or_none(array):
    if array is None:
        return None
    else:
        return array.hex()

file_hashes = {}

def get_file_hash(file_path):
    global file_hashes
    modification_time = os.path.getmtime(file_path)
    if file_path in file_hashes:
        (stored_modification_time, stored_result) = file_hashes[file_path]
        if stored_modification_time == modification_time:
            return stored_result
    hasher = hashlib.sha256()
    hasher.update(open(file_path, "rb").read())
    result = hasher.digest()
    file_hashes[file_path] = (modification_time, result)
    return result

dependency_files = {}

def read_dependency_file(file_path):
    global dependency_files
    modification_time = os.path.getmtime(file_path)
    if file_path in dependency_files:
        (stored_modification_time, stored_result) = dependency_files[file_path]
        if stored_modification_time == modification_time:
            return stored_result
    result = {}
    file = open(file_path, "r", encoding="utf-8")
    text = file.read()
    text = text.replace("\\\n", "")
    text = text.replace("//", "/")
    for line in text.splitlines():
        match = re.match(r"(.+): (.+)", line)
        if match:
            targets = [e for e in match.group(1).strip().split(" ") if e != ""]
            for target in targets:
                result[target] = [e for e in match.group(2).strip().split(" ") if e != ""]
    file.close()
    dependency_files[file_path] = (modification_time, result)
    return result

def matches_filter(value, positive_filter, negative_filter, full_match):
    return ((re.fullmatch(positive_filter, value) if full_match else re.search(positive_filter, value)) is not None if positive_filter is not None else True) and \
           ((re.fullmatch(negative_filter, value) if full_match else re.search(negative_filter, value)) is None if negative_filter is not None else True)

num_runs_fast_regex = re.compile(r"(?m).*^\s*(include\s+.*\.ini|repeat\s*=\s*[0-9]+|.*\$\{.*\})")

def get_num_runs_fast(ini_path):
    file = open(ini_path, "r", encoding="utf-8")
    text = file.read()
    file.close()
    return None if num_runs_fast_regex.search(text) else 1

class KeyboardInterruptHandler:
    def __init__(self):
        self.enabled = True
        self.old_handler = None
        self.received_signal = None

    def handle_disabled_keyboard_interrupt(self, sig, frame):
        self.received_signal = (sig, frame)
        _logger.debug("SIGINT received, delaying KeyboardInterrupt.")

    def handle_pending_keyboard_interrupt(self):
        if self.received_signal:
            self.old_handler(*self.received_signal)
            self.received_signal = None

    def disable(self):
        if self.enabled:
            self.enabled = False
            self.old_handler = signal.signal(signal.SIGINT, self.handle_disabled_keyboard_interrupt)
            self.received_signal = None

    def enable(self):
        if not self.enabled:
            self.enabled = True
            signal.signal(signal.SIGINT, self.old_handler)
            self.handle_pending_keyboard_interrupt()

class EnabledKeyboardInterrupts:
    def __init__(self, handler):
        self.handler = handler

    def __enter__(self):
        if self.handler:
            try:
                self.handler.enable()
            except:
                if self.__exit__(*sys.exc_info()):
                    pass
                else:
                    raise

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.handler:
            self.handler.disable()

class DisabledKeyboardInterrupts:
    def __init__(self, handler):
        self.handler = handler

    def __enter__(self):
        if self.handler:
            try:
                self.handler.disable()
            except:
                if self.__exit__(*sys.exc_info()):
                    pass
                else:
                    raise

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.handler:
            self.handler.enable()

def test_keyboard_interrupt_handler(a, b):
    handler = KeyboardInterruptHandler()
    with DisabledKeyboardInterrupts(handler):
        print("Disabled start")
        time.sleep(a)
        try:
            with EnabledKeyboardInterrupts(handler):
                print("Enabled start")
                time.sleep(b)
                print("Enabled end")
        except KeyboardInterrupt:
            print("Interrupted")
        print("Disabled end")

class ColoredLoggingFormatter(logging.Formatter):
    print_thread_name = False
    print_function_name = False

    COLORS = {
        logging.DEBUG: COLOR_GREEN,
        logging.INFO: COLOR_GREEN,
        logging.WARNING: COLOR_YELLOW,
        logging.ERROR: COLOR_RED,
        logging.CRITICAL: COLOR_RED,
        STDOUT_LEVEL: COLOR_GREEN,
        STDERR_LEVEL: COLOR_YELLOW,
    }

    def format(self, record):
        format = self.COLORS.get(record.levelno) + "%(levelname)s " + \
                 (COLOR_MAGENTA + "%(threadName)s " if self.print_thread_name else "") + \
                 COLOR_CYAN + "%(name)s " + \
                 (COLOR_MAGENTA + "%(funcName)s " if self.print_function_name else "") + \
                 COLOR_RESET + "%(message)s"
        formatter = logging.Formatter(format)
        return formatter.format(record)

def with_extended_thread_name(name, body):
    current_thread = threading.current_thread()
    old_name = current_thread.name
    try:
        current_thread.name = old_name + "/" + name
        body()
    finally:
        current_thread.name = old_name

def with_logger_level(logger, level, body):
    old_level = logger.getEffectiveLevel()
    try:
        logger.setLevel(level)
        body()
    finally:
        logger.setLevel(old_level)

class LoggerLevel(object):
    def __init__(self, logger, level):
        self.logger = logger
        self.level = level

    def __enter__(self):
        self.old_level = self.logger.getEffectiveLevel()
        self.logger.setLevel(self.level)

    def __exit__(self, type, value, traceback):
        self.logger.setLevel(self.old_level)

class DebugLevel(LoggerLevel):
    def __init__(self, logger):
        super().__init__(self, logger, logging.DEBUG)

def run_command_with_logging(args, error_message=None, nice=10, **kwargs):
    logger = logging.getLogger(os.path.basename(args[0]))
    def log_stream(stream, logger, lines):
        for line in iter(stream.readline, ""):
            logger(line.rstrip("\n"))
            lines.append(line)
        stream.close()
    stdout_lines = []
    stderr_lines = []
    logger.info(f"Running external command: {' '.join(args)}")
    process = subprocess.Popen(["nice", "-n", str(nice), *args], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, **kwargs)
    stdout_thread = threading.Thread(target=log_stream, args=(process.stdout, logger.stdout, stdout_lines))
    stderr_thread = threading.Thread(target=log_stream, args=(process.stderr, logger.stderr, stderr_lines))
    stdout_thread.start()
    stderr_thread.start()
    try:
        stdout_thread.join()
        stderr_thread.join()
        process.wait()
    except KeyboardInterrupt:
        process.kill()
        raise
    if process.returncode == -signal.SIGINT:
        raise KeyboardInterrupt()
    if error_message and process.returncode != 0:
        raise Exception(error_message)
    return subprocess.CompletedProcess(args, process.returncode, "".join(stdout_lines), ''.join(stderr_lines))

def collect_existing_ned_types():
    types = set()
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.ned"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                text = f.read()
                for type in re.findall("^simple (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^module (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^network (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^moduleinterface (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^channel (\\w+)", text, re.M):
                    types.add(type)
    return types

def collect_referenced_ned_types():
    types = set()
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.ini"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                for type in re.findall("typename = \"(\\w+?)\"", f.read()):
                    types.add(type)
    for ned_file_path in glob.glob(get_inet_relative_path("**/*.ned"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ned_file_path, "r") as f:
                for type in re.findall("~(\\w+)", f.read()):
                    types.add(type)
    for rst_file_path in glob.glob(get_inet_relative_path("**/*.rst"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(rst_file_path, "r") as f:
                for type in re.findall(":ned:`(\\w+?)`", f.read()):
                    types.add(type)
    return types

def collect_ned_type_reference_file_paths(type):
    references = []
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.ini"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                if re.search(f"typename = \"{type}\"", f.read()):
                    references.append(ini_file_path)
    for rst_file_path in glob.glob(get_inet_relative_path("**/*.rst"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(rst_file_path, "r") as f:
                if re.search(f":ned:`{type}`", f.read()):
                    references.append(rst_file_path)
    return references

def collect_existing_msg_types():
    types = set()
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.msg"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                text = f.read()
                for type in re.findall("^class (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^packet (\\w+)", text, re.M):
                    types.add(type)
    return types

def collect_existing_cpp_types():
    types = set()
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.h"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                text = f.read()
                for type in re.findall("^class INET_API (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^enum (\\w+)", text, re.M):
                    types.add(type)
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.cc"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                text = f.read()
                for type in re.findall("Register_Packet_Dropper_Function\\((\\w+),", text, re.M):
                    types.add(type)
                for type in re.findall("Register_Packet_Comparator_Function\\((\\w+),", text, re.M):
                    types.add(type)
    return types

def collect_referenced_non_existing_ned_types():
    referenced_ned_types = collect_referenced_ned_types()
    existing_ned_types = collect_existing_ned_types()
    existing_msg_types = collect_existing_msg_types()
    existing_cpp_types = collect_existing_cpp_types()
    return referenced_ned_types.difference(existing_ned_types).difference(existing_msg_types).difference(existing_cpp_types)
