import logging
import signal
import subprocess

from inet.common.task import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

# TODO: experimental native Python build tool

class BuildTaskResult(TaskResult):
    def __init__(self, possible_results=["DONE", "SKIP", "CANCEL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_RED], **kwargs):
        super().__init__(possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)

class MultipleBuildTaskResults(MultipleTaskResults):
    def __init__(self, possible_results=["DONE", "SKIP", "CANCEL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_RED], **kwargs):
        super().__init__(possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)
        if self.result == "SKIP":
            self.expected = True

class BuildTask(Task):
    def __init__(self, simulation_project=None, task_result_class=BuildTaskResult, **kwargs):
        super().__init__(task_result_class=task_result_class, **kwargs)
        self.simulation_project = simulation_project

    def is_up_to_date(self):
        def get_file_modification_time(file_path):
            full_file_path = self.simulation_project.get_full_path(file_path)
            return os.path.getmtime(full_file_path) if os.path.exists(full_file_path) else None
        def get_file_modification_times(file_paths):
            return list(map(get_file_modification_time, file_paths))
        input_file_modification_times = get_file_modification_times(self.get_input_files())
        output_file_modification_times = get_file_modification_times(self.get_output_files())
        return input_file_modification_times and output_file_modification_times and \
               not list(filter(lambda timestamp: timestamp is None, output_file_modification_times)) and \
               max(input_file_modification_times) < min(output_file_modification_times)

    def run(self, **kwargs):
        if self.is_up_to_date():
            return self.task_result_class(task=self, result="SKIP", expected_result="SKIP", reason="Up-to-date")
        else:
            return super().run(**kwargs)

    def run_protected(self, **kwargs):
        args = self.get_arguments()
        _logger.debug(f"Running subprocess: {args}")
        subprocess_result = subprocess.run(args, cwd=self.simulation_project.get_full_path("."))
        if subprocess_result.returncode == signal.SIGINT.value or subprocess_result.returncode == -signal.SIGINT.value:
            return self.task_result_class(task=self, subprocess_result=subprocess_result, result="CANCEL", reason="Cancel by user")
        elif subprocess_result.returncode == 0:
            return self.task_result_class(task=self, subprocess_result=subprocess_result, result="DONE")
        else:
            return self.task_result_class(task=self, subprocess_result=subprocess_result, result="ERROR", reason=f"Non-zero exit code: {subprocess_result.returncode}")

class MsgCompileTask(BuildTask):
    def __init__(self, simulation_project=None, file_path=None, name="MSG compile task", mode="release", **kwargs):
        super().__init__(simulation_project=simulation_project, name=name, **kwargs)
        self.file_path = file_path
        self.mode = mode

    def get_action_string(self, **kwargs):
        return "Generating"

    def get_parameters_string(self, **kwargs):
        return self.file_path

    def get_input_files(self):
        output_folder = f"out/clang-{self.mode}"
        object_path = re.sub("\\.msg", "_m.cc", self.file_path)
        dependency_file_path = re.sub("\\.msg", "_m.h.d", self.file_path)
        full_file_path = self.simulation_project.get_full_path(os.path.join(output_folder, dependency_file_path))
        if os.path.exists(full_file_path):
            dependency = read_dependency_file(full_file_path)
            # KLUDGE: src folder hacked in and out
            file_paths = dependency[re.sub("src/", "", object_path)]
            return list(map(lambda file_path: self.simulation_project.get_full_path(os.path.join("src", file_path)), file_paths))
        else:
            return [self.file_path]

    def get_output_files(self):
        cpp_file_path = re.sub("\\.msg", "_m.cc", self.file_path)
        header_file_path = re.sub("\\.msg", "_m.h", self.file_path)
        return [f"{cpp_file_path}", f"{header_file_path}"]

    def get_arguments(self):
        executable = "opp_msgc"
        output_folder = f"out/clang-{self.mode}"
        header_file_path = re.sub("\\.msg", "_m.h", self.file_path)
        import_paths = list(map(lambda msg_folder: self.simulation_project.get_full_path(msg_folder), self.simulation_project.msg_folders))
        return [executable,
                "--msg6",
                "-s",
                "_m.cc",
                "-MD",
                "-MP",
                "-MF",
                f"../{output_folder}/{header_file_path}.d",
                *list(map(lambda import_path: "-I" + import_path, import_paths)),
                "-Iinet/transportlayer/tcp_lwip/lwip/include",
                "-Iinet/transportlayer/tcp_lwip/lwip/include/ipv4",
                "-Iinet/transportlayer/tcp_lwip/lwip/include/ipv6",
                "-PINET_API",
                self.file_path]

class CppCompileTask(BuildTask):
    def __init__(self, simulation_project=None, file_path=None, name="C++ compile task", mode="release", **kwargs):
        super().__init__(simulation_project=simulation_project, name=name, **kwargs)
        self.file_path = file_path
        self.mode = mode

    def get_action_string(self, **kwargs):
        return "Compiling"

    def get_parameters_string(self, **kwargs):
        return self.file_path

    def get_input_files(self):
        output_folder = f"out/clang-{self.mode}"
        object_path = re.sub("\\.cc", ".o", self.file_path)
        dependency_file_path = re.sub("\\.cc", ".o.d", self.file_path)
        full_file_path = self.simulation_project.get_full_path(os.path.join(output_folder, dependency_file_path))
        if os.path.exists(full_file_path):
            dependency = read_dependency_file(full_file_path)
            file_paths = dependency[os.path.join(output_folder, object_path)]
            return list(map(lambda file_path: self.simulation_project.get_full_path(file_path), file_paths))
        else:
            return [self.file_path]

    def get_output_files(self):
        output_folder = f"out/clang-{self.mode}"
        object_path = re.sub("\\.cc", ".o", self.file_path)
        return [f"{output_folder}/{object_path}"]

    def get_arguments(self):
        executable = "clang++"
        output_folder = f"out/clang-{self.mode}"
        output_file = self.get_output_files()[0]
        return [executable,
                "-c",
                "-O3",
                "-ffp-contract=off",
                "-march=native",
                "-mtune=native",
                "-DNDEBUG=1",
                "-MMD",
                "-MP",
                "-MF",
                f"{output_file}.d",
                "-fPIC",
                "-Wno-deprecated-register",
                "-Wno-unused-function",
                "-fno-omit-frame-pointer",
                "-DHAVE_SWAPCONTEXT",
                "-DWITH_NETBUILDER",
                "-DOMNETPPLIBS_IMPORT",
                "-I/home/levy/workspace/omnetpp/include",
                *list(map(lambda include_path: "-I" + include_path, self.simulation_project.get_effective_include_folders())),
                *list(map(lambda cpp_define: "-D" + cpp_define, self.simulation_project.cpp_defines)),
                "-o",
                f"{output_file}",
                self.file_path]

    def run_protected(self, **kwargs):
        output_folder = f"out/clang-{self.mode}"
        output_file = self.get_output_files()[0]
        directory = os.path.dirname(self.simulation_project.get_full_path(output_file))
        if not os.path.exists(directory):
            try:
                os.makedirs(directory)
            except:
                pass
        return super().run_protected(**kwargs)

class LinkTask(BuildTask):
    def __init__(self, simulation_project=None, name="Link task", type="dynamic library", mode="release", compile_tasks=[], **kwargs):
        super().__init__(simulation_project=simulation_project, name=name, **kwargs)
        self.type = type
        self.mode = mode
        self.compile_tasks = compile_tasks

    def get_action_string(self, **kwargs):
        return "Linking"

    def get_output_prefix(self):
        return "" if self.type == "executable" else "lib"

    def get_output_suffix(self):
        return "_dbg" if self.mode == "debug" else ""

    def get_output_extension(self):
        return "" if self.type == "executable" else (".so" if self.type == "dynamic library" else ".a")

    def get_parameters_string(self, **kwargs):
        return self.get_output_prefix() + self.simulation_project.dynamic_libraries[0] + self.get_output_suffix() + self.get_output_extension()

    def get_input_files(self):
        return flatten(map(lambda compile_task: compile_task.get_output_files(), self.compile_tasks))

    def get_output_files(self):
        output_folder = f"out/clang-{self.mode}"
        return [os.path.join(output_folder, self.get_output_prefix() + self.simulation_project.dynamic_libraries[0] + self.get_output_suffix() + self.get_output_extension())]

    def get_arguments(self):
        output_folder = f"out/clang-{self.mode}"
        input_files = self.get_input_files()
        if self.type == "executable":
            executable = "clang++"
            return [executable,
                    "-fuse-ld=lld",
                    "-Wl,-rpath,/home/levy/workspace/omnetpp/lib",
                    "-Wl,-rpath,/home/levy/workspace/omnetpp/tools/linux.x86_64/lib",
                    "-Wl,-rpath,.",
                    "-Wl,--export-dynamic",
                    "-L/home/levy/workspace/omnetpp/lib",
                    *list(map(lambda used_project: "-L" + get_simulation_project(used_project, None).get_library_folder_full_path(), self.simulation_project.used_projects)),
                    "-o",
                    self.get_output_files()[0],
                    *input_files,
                    "-Wl,--no-as-needed",
                    "-Wl,--whole-archive",
                    "-Wl,--no-whole-archive",
                    "-loppmain",
                    "-Wl,-u,_cmdenv_lib",
                    "-Wl,--no-as-needed",
                    "-loppcmdenv",
                    "-loppenvir",
                    "-Wl,-u,_qtenv_lib",
                    "-Wl,--no-as-needed",
                    "-Wl,-rpath=/usr/lib/x86_64-linux-gnu",
                    "-loppqtenv",
                    "-loppenvir",
                    "-lopplayout",
                    "-loppsim",
                    *list(map(lambda used_project: "-l" + get_simulation_project(used_project, None).dynamic_libraries[0], self.simulation_project.used_projects)),
                    *list(map(lambda external_library: "-l" + external_library, self.simulation_project.external_libraries)),
                    "-lstdc++"]
        elif self.type == "dynamic library":
            executable = "clang++"
            return [executable,
                    "-shared",
                    "-fPIC",
                    "-o",
                    self.get_output_files()[0],
                    *input_files,
                    "-Wl,--no-as-needed",
                    "-Wl,--whole-archive",
                    "-Wl,--no-whole-archive",
                    *list(map(lambda used_project: "-l" + get_simulation_project(used_project, None).dynamic_libraries[0], self.simulation_project.used_projects)),
                    *list(map(lambda external_library: "-l" + external_library, self.simulation_project.external_libraries)),
                    "-loppenvir",
                    "-loppsim",
                    "-lstdc++",
                    "-fuse-ld=lld",
                    "-Wl,-rpath,/home/levy/workspace/omnetpp/lib",
                    "-Wl,-rpath,/home/levy/workspace/omnetpp/tools/linux.x86_64/lib",
                    "-Wl,-rpath,.",
                    "-Wl,--export-dynamic",
                    "-L/home/levy/workspace/omnetpp/lib",
                    *list(map(lambda used_project: "-L" + get_simulation_project(used_project, None).get_library_folder_full_path(), self.simulation_project.used_projects))]
        else:
            executable = "ar"
            return [executable,
                    "cr",
                    self.get_output_files()[0],
                    *input_files]
