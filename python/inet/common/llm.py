import llm
import logging
import os
import re
import subprocess
import unidiff

from inet.project import *

_logger = logging.getLogger(__name__)

# model = llm.get_model("orca-mini-3b-gguf2-q4_0")
# model = llm.get_model("gpt4all-falcon-newbpe-q4_0")
# model = llm.get_model("claude-3-opus")
# model = llm.get_model("claude-2")

model = llm.get_model("gpt-3.5-turbo")
model.key = ''

def collect_matching_file_paths(directory, name_pattern, content_pattern):
    """
    Collect all file paths in a directory and its subdirectories that match the given regular expressions for file name, extension, and content.

    Args:
        directory (str): The root directory to start the search from.
        name_pattern (str): The regular expression pattern to match file names against.
        content_pattern (str): The regular expression pattern to match file contents against.

    Returns:
        list: A list of file paths that match the regular expressions.
    """
    matching_file_paths = []
    name_regex = re.compile(name_pattern)
    content_regex = re.compile(content_pattern)

    for root, _, files in os.walk(directory):
        for file in files:
            if name_regex.match(file):
                file_path = os.path.join(root, file)
                with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                    if content_regex.search(content):
                        matching_file_paths.append(file_path)
    return matching_file_paths

def read_files(file_list):
    contents = []
    for file_path in file_list:
        with open(file_path, 'r', encoding='utf-8') as file:
            contents.append(file.read())
    return "\n".join(contents)

def apply_command_to_files(file_list, context, command_text):
    for file_path in file_list:
        _logger.info(f"Processing file {file_path}")
        with open(file_path, 'r', encoding='utf-8') as file:
            file_content = file.read()
        prompt = f"""
Update a file based on some context. Here is the context:
```
{context}
```
Here is what should be done with the file: {command_text}
Respond with the updated file verbatim without any additional text. Here is the file that should be updated:
```
{file_content}
```
"""
        _logger.debug("Sending prompt to LLM", prompt)
        modified_content = model.prompt(prompt)
        result = modified_content.text()
        _logger.debug("Received result from LLM", result)
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(result)
        _logger.debug(f"Modified {file_path} successfully.")

def filter_diff(input_filename, output_filename, keep_lines):
    keep_lines_regex = re.compile(keep_lines)
    with open(input_filename, 'r') as diff_file:
        file_index = 0
        patch = unidiff.PatchSet(diff_file)
        for patched_file in list(patch):
            keep_file = False
            hunk_index = 0
            for hunk in list(patched_file):
                keep = False
                for line in hunk:
                    if keep_lines_regex.match(str(line)):
                        keep = True
                if not keep:
                    del patched_file[hunk_index]
                else:
                    hunk_index = hunk_index + 1
                    keep_file = True
            if not keep_file:
                del patch[file_index]
            else:
                file_index = file_index + 1
        with open(output_filename, 'w') as output_file:
            output_file.write(str(patch))

def filter_unstaged_changes(keep_lines, file_list):
    result = subprocess.run(["git", "diff", "HEAD", "-U1", "--output=input.diff"])
    subprocess.run(["git", "restore", "."])
    filter_diff("input.diff", "output.diff", keep_lines)
    subprocess.run(["git", "apply", "output.diff"])
    os.remove("input.diff")
    os.remove("output.diff")

def proofread_ned_comments(simulation_project=None, folder="."):
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    path = simulation_project.get_relative_path(folder)
    file_list = collect_matching_file_paths(path, r".*.ned$", r".*")
    command_text = "Here is an OMNeT++ NED file, fix any English mistakes in the comments (lines starting with `//`) in it."
    apply_command_to_files(file_list, [], command_text)
    filter_unstaged_changes("^[+-]//", file_list)

def add_backticks_in_ned_comments(simulation_project=None, folder="."):
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    path = simulation_project.get_relative_path(folder)
    file_list = collect_matching_file_paths(path, r".*.ned$", r".*")
    command_text = "Here is an OMNeT++ NED file, modify the comment lines (starting with '//') by adding backticks around upper-case identifiers."
    apply_command_to_files(file_list, [], command_text)
    filter_unstaged_changes("^[+-]//", file_list)
