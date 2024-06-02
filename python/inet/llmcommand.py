import logging
import os
import re
import subprocess
import unidiff
import llm
import argparse
import tiktoken

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

def get_llm_context_window(model):
    size = model.default_max_tokens
    if size:  # it is not always filled in?
        return size
    model_name = model.model_id
    context_window_sizes = {
        "gpt-3.5-turbo": 4096,
        "gpt-3.5-turbo-16k": 16384,
        "gpt-4": 8192,
        "gpt-4-32k": 32768,
        "gpt-4o": 128*1024,
    }
    if model_name not in context_window_sizes:
        raise Exception(f"Context window size for llm '{model_name}' is not known, please add it to the table")
    return context_window_sizes[model_name]

def check_token_count(prompt, model):
    encoder = tiktoken.encoding_for_model(model.model_id)
    num_tokens = len(encoder.encode(prompt))
    max_tokens = get_llm_context_window(model)
    _logger.info(f"Number of tokens in the prompt: {num_tokens}")
    if max_tokens and num_tokens > max_tokens:
        print(f"WARNING: Prompt of {num_tokens} tokens exceeds the model's context window size of {max_tokens} tokens")

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
        "eliminate-you-addressing": "At places where the text addresses the user as 'you', change it to neutral, e.g., to passive voice or 'one' as subject. Keep all markup and line breaks intact as much as possible.",
        # "neddoc": "Write a new neddoc comment for the module in the NED file. NED comments use // marks instead of /*..*/."
        "neddoc": """
        You are working in the context of OMNeT++ and the INET Framework.
        Your task is to write a single sentence capturing the most important aspect of the following simple module.
        Ignore the operational details of the module and focus on the aspects that help the user understand what this module is good for.
          """
    }

    if file_type not in file_type_commands:
        raise ValueError(f'Unsupported file type "{file_type}"')
    if task not in task_commands:
        raise ValueError(f'Unsupported task "{task}"')
    if "ned" in task and file_type != "ned":
        raise ValueError(f'Task "{task}" is only supported for the "ned" file type')

    return file_type_commands[file_type] + " " + task_commands[task]

def get_recommended_model(task):
    recommended_models = {
        "proofread": "gpt-3.5-turbo-16k",
        "improve-language": "gpt-3.5-turbo-16k",
        "eliminate-you-addressing": "gpt-4o",
        "neddoc": "gpt-4o"
    }
    if task not in recommended_models:
        raise ValueError(f'No info on which model is recommended for "{task}", specify one explicitly via --model or update the tool')
    return recommended_models[task]

def find_additional_context_files(file_path, file_type, task):
    context_files = []
    if task == "neddoc":
        # fname_without_ext = os.path.splitext(os.path.basename(file_path))[0]
        # h_fname = fname_without_ext + ".h"
        # cc_fname = fname_without_ext + ".cc"
        # for root, _, files in os.walk(os.path.dirname(file_path) or "."):
        #     if h_fname in files:
        #         context_files.append(os.path.join(root, h_fname))
        #     if cc_fname in files:
        #         context_files.append(os.path.join(root, cc_fname))

        fname_without_ext = os.path.splitext(os.path.basename(file_path))[0]
        grep_command = f"""rg --heading -g '*.cc' -g '*.h' -g '*.ini' -g '*.ned' -g '!{os.path.basename(file_path)}' -C 10 '{fname_without_ext}' $(git rev-parse --show-toplevel) > {file_path}.ctx"""
        os.system(grep_command)
        context_files.append(file_path + ".ctx")

    return context_files

def create_prompt(content, context, task, file_type):
    command_text = generate_command_text(task, file_type)
    prompt = f"Update a file delimited by triple quotes. {command_text}\n\n"
    if context:
        prompt += f"Here is the context:\n{context}\n\n"
    prompt += f'Here is the file that should be updated:\n\n```\n{content}\n```\n\n'
    prompt += f"Respond with the updated file verbatim without any additional commentary.\n"
    return prompt

def invoke_llm(prompt, model):
    check_token_count(prompt, model)

    _logger.debug(f"Sending prompt to LLM: {prompt}")
    reply = model.prompt(prompt)
    reply_text = reply.text()
    _logger.debug(f"Received result from LLM: {reply_text}")
    return reply_text

def split_content(file_content, file_type):
    if file_type == "tex":
        return split_latex_by_sections(file_content)
    else:
        return [file_content]

def split_latex_by_sections(latex_source):
    section_pattern = re.compile(r'(\\section\{.*?\})', re.DOTALL)
    # Split to list containing both the section headers and section contents
    parts = section_pattern.split(latex_source)
    if len(parts) < 2:
        return [latex_source]

    # Combine the initial content with the first section header and content
    initial_content = parts[0]
    sections = []
    sections.append(initial_content + parts[1] + parts[2])

    # Combine the rest of the section headers with their content
    for i in range(3, len(parts), 2):
        sections.append(parts[i] + parts[i+1])
    return sections

def extract(reply_text, original_content):
    content = reply_text
    if content.count("```") >= 2:
        content = re.sub(r"^.*```.*?\n(.*\n)```.*$", r"\1", content, 1, re.DOTALL)

    trailing_whitespace_len = len(original_content) - len(original_content.rstrip())
    original_trailing_whitespace = original_content[-trailing_whitespace_len:]
    content = content.rstrip() + original_trailing_whitespace
    return content

def apply_command_to_file(file_path, context_files, file_type, task, model, save_prompt=False):
    with open(file_path, 'r', encoding='utf-8') as file:
        file_content = file.read()

    context_files = context_files or []
    context_files += find_additional_context_files(file_path, file_type, task)
    print("   context files: " + " ".join(context_files))
    context = read_files(context_files)

    modified_content = ""
    parts = split_content(file_content, file_type)
    for i, part in enumerate(parts):
        if len(parts) > 1:
            print(f"   part  {i + 1}/{len(parts)}")
        prompt = create_prompt(part, context, task, file_type)

        if save_prompt:
            with open(file_path+".prompt"+str(i), 'w', encoding='utf-8') as file:
                file.write(prompt)

        reply_text = invoke_llm(prompt, model)
        modified_content += extract(reply_text, part)

    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(modified_content)
    _logger.debug(f"Modified {file_path} successfully.")

def apply_command_to_files(file_list, context_files, file_type, task, model, save_prompt=False):
    n = len(file_list)
    for i, file_path in enumerate(file_list):
        try:
            print(f"Processing file {i + 1}/{n} {file_path}")
            apply_command_to_file(file_path, context_files, file_type, task, model, save_prompt)
        except Exception as e:
            print(f"-> Exception: {e}")

def resolve_file_list(paths, file_type):
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
    return file_list

def process_files(paths, context_files, file_type, task, model_name, save_prompt=False):
    if not model_name:
        model_name = get_recommended_model(task)
    model = llm.get_model(model_name)
    model.key = ''
    file_list = resolve_file_list(paths, file_type)
    print("Files to process: " + " ".join(file_list))
    print("Using LLM: " + model_name)
    apply_command_to_files(file_list, context_files, file_type, task, model, save_prompt=save_prompt)

def main():
    parser = argparse.ArgumentParser(description="Process and improve specific types of files in a given directory or files.")
    parser.add_argument("paths", type=str, nargs='+', help="The directories or files to process.")
    parser.add_argument("--file-type", type=str, choices=["md", "rst", "tex", "ned"], required=True, help="The type of files to process.")
    parser.add_argument("--task", type=str, choices=["proofread", "improve-language", "eliminate-you-addressing", "neddoc"], required=True, help="The task to perform on the files.")
    parser.add_argument("--model", type=str, default=None, help="The name of the LLM model to use.")
    parser.add_argument("--context", type=str, nargs='*', help="The context files to be used.")
    parser.add_argument("--save-prompt", action='store_true', help="Save the LLM prompt for each input file as <filename>.prompt.")

    args = parser.parse_args()
    process_files(args.paths, args.context, args.file_type, args.task, args.model, save_prompt=args.save_prompt)

if __name__ == "__main__":
    main()
