import logging
import os
import re
import subprocess
import unidiff
import llm
import argparse

# Initialize logger
_logger = logging.getLogger(__name__)
_logger.setLevel(logging.INFO)

def collect_matching_file_paths(directory, name_pattern, content_pattern=None):
    matching_file_paths = []
    name_regex = re.compile(name_pattern)
    content_regex = re.compile(content_pattern) if content_pattern else None

    for root, _, files in os.walk(directory):
        for file in files:
            if name_regex.match(file):
                file_path = os.path.join(root, file)
                if not content_regex:
                    matching_file_paths.append(file_path)
                else:
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        try:
                            content = f.read()
                            if content_regex.search(content):
                                matching_file_paths.append(file_path)
                        except Exception as e:
                            print(f"Could not read file {file_path}: {e}")

    return matching_file_paths

def read_files(file_list):
    contents = []
    for file_path in file_list:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
            contents.append(f"File `{file_path}`:\n```\n{content}\n```\n\n")
    return "".join(contents)

def apply_command_to_files(file_list, context, command_text, model_name):
    model = llm.get_model(model_name)
    model.key = ''

    n = len(file_list)
    for i, file_path in enumerate(file_list):
        try:
            print(f"Processing file {i + 1}/{n} {file_path}")
            apply_command_to_file(file_path, context, command_text, model)
        except Exception as e:
            print(f"-> Exception: {e}")

def apply_command_to_file(file_path, context, command_text, model, max_chunk_size=8000):
    with open(file_path, 'r', encoding='utf-8') as file:
        file_content = file.read()
        modified_content = apply_command_to_content(file_content, context, command_text, model)
    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(modified_content)
    _logger.debug(f"Modified {file_path} successfully.")

def apply_command_to_content(content, context, command_text, model):
    prompt = "Update a file based on some context."
    if context:
        prompt += f" Here is the context:\n{context}\n"
    prompt += f"Here is what should be done with the file: {command_text}\n"
    prompt += f"Respond with the updated file verbatim without any additional text. Here is the file that should be updated:\n```\n{content}\n```\n"

    _logger.debug(f"Sending prompt to LLM: {prompt}")
    reply = model.prompt(prompt)
    modified_content = reply.text()
    _logger.debug(f"Received result from LLM: {modified_content}")

    if modified_content.count("```") == 2:
        modified_content = modified_content.split("```")[1]

    trailing_whitespace_len = len(content) - len(content.rstrip())
    modified_content = modified_content.rstrip() + content[-trailing_whitespace_len:]
    return modified_content

def generate_command_text(task, file_type):
    file_type_commands = {
        "md": "The following Markdown file belongs to the OMNeT++ project.",
        "rst": "The following reStructuredText file  belongs to the OMNeT++ project. Blocks starting with '..' are comments, do not touch them!",
        "tex": "The following LaTeX file belongs to the OMNeT++ project.",
        "ned": "The following is an OMNeT++ NED file. DO NOT CHANGE ANYTHING IN NON-COMMENT LINES."
    }

    task_commands = {
        "proofread": "Fix any English mistakes in its text. Keep all markup and line breaks intact as much as possible!",
        "improve-language": "Improve the English in the text. Keep all other markup and line breaks intact as much as possible.",
        "eliminate-you-addressing": "At places where the text addresses the user as 'you', change it to neutral, e.g., to passive voice or 'one' as subject. Keep all markup and line breaks intact as much as possible."
    }

    if file_type not in file_type_commands:
        raise ValueError("Unsupported file type.")
    if task not in task_commands:
        raise ValueError("Unsupported task for the given file type.")

    return file_type_commands[file_type] + " " + task_commands[task]

def process_files(paths, file_type, task, model_name):
    file_extension_patterns = {
        "md": r".*.md$",
        "rst": r".*.rst$",
        "tex": r".*.tex$",
        "ned": r".*.ned$"
    }

    if file_type not in file_extension_patterns:
        raise ValueError("Unsupported file type.")

    file_list = []
    for path in paths:
        if os.path.isdir(path):
            file_list.extend(collect_matching_file_paths(path, file_extension_patterns[file_type]))
        elif os.path.isfile(path) and re.match(file_extension_patterns[file_type], path):
            file_list.append(path)

    print("Files to process: " + " ".join(file_list))
    command_text = generate_command_text(task, file_type)
    apply_command_to_files(file_list, "", command_text, model_name)

def main():
    parser = argparse.ArgumentParser(description="Process and improve specific types of files in a given directory.")
    parser.add_argument("paths", type=str, nargs='+', help="The directories or files to process.")
    parser.add_argument("--file-type", type=str, choices=["md", "rst", "tex", "ned"], required=True, help="The type of files to process.")
    parser.add_argument("--task", type=str, choices=["proofread", "improve-language", "eliminate-you-addressing"], required=True, help="The task to perform on the files.")
    parser.add_argument("--model", type=str, default="gpt-3.5-turbo-16k", help="The name of the LLM model to use. Type `llm models` for a list.")

    args = parser.parse_args()

    process_files(args.paths, args.file_type, args.task, args.model)

if __name__ == "__main__":
    main()
