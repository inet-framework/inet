import logging
import os
import re

from inet.common import *
from inet.simulation import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

def generate_coverage_report(simulation_project=None, output_dir="coverage", **kwargs):
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    os.environ["LLVM_PROFILE_FILE"] = "coverage-%p.profraw"
    run_simulations(simulation_project=simulation_project, mode="coverage", **kwargs)
    profraw_files = "profraw_files.txt"
    pattern = re.compile(r"coverage-.*\.profraw")
    file_names = []
    with open(profraw_files, "w") as f:
        for root, _, files in os.walk(simulation_project.get_relative_path(".")):
            for file_name in files:
                if pattern.search(file_name):
                    file_names.append(os.path.join(root, file_name))
                    f.write(os.path.join(root, file_name) + "\n")
    merged_profdata_file = "merged.profdata"
    args = ["llvm-profdata", "merge", f"--input-files={profraw_files}", f"-output={merged_profdata_file}"]
    run_command_with_logging(args)
    os.remove(profraw_files)
    for file_name in file_names:
        os.remove(file_name)
    args = ["llvm-cov", "show", "src/libINET_coverage.so", f"-instr-profile={merged_profdata_file}", "-format=html", f"-output-dir={output_dir}"]
    run_command_with_logging(args)
    os.remove(merged_profdata_file)

def open_coverage_report(simulation_project=None, output_dir="coverage", **kwargs):
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    generate_coverage_report(simulation_project=simulation_project, output_dir=output_dir, **kwargs)
    open_file_with_default_editor(f"{output_dir}/index.html")
