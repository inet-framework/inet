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

def collect_matching_file_paths(directory, name_regex, content_pattern=None):
    matching_file_paths = []
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
        "ned": "The following is an OMNeT++ NED file. DO NOT CHANGE ANYTHING IN NON-COMMENT LINES.",
        "py":  "The following Python source file belongs to the OMNeT++ project.",
    }
    what = "docstrings" if file_type == "py" else "text"
    task_commands = {
        "proofread": f"You are an editor of technical documentation. You are tireless and diligent. Fix any English mistakes in the {what}. Keep all markup, line breaks and indentation intact as much as possible!",
        "improve-language": f"You are an editor of technical documentation. You are tireless and diligent. Improve the English in the {what}. Keep all other markup and line breaks intact as much as possible.",
        "eliminate-you-addressing": f"You are an editor of technical documentation. You are tireless and diligent. At places where {what} addresses the user as 'you', change it to neutral, e.g., to passive voice or 'one' as subject. Keep all markup and line breaks intact as much as possible.",
        "neddoc": "You are a technical writer. You are tireless and diligent. Write a new neddoc comment for the module in the NED file. NED comments use // marks instead of /*..*/.",
        # "neddoc": """
        # You are a technical writer.  You are tireless and diligent. You are working in the context of OMNeT++ and the INET Framework.
        # Your task is to write a single sentence capturing the most important aspect of the following simple module.
        # Ignore the operational details of the module and focus on the aspects that help the user understand what this module is good for.
        #   """
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
        None: "gpt-3.5-turbo-16k",
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
        grep_command = f"""rg --heading -g '*.cc' -g '*.h' -g '*.ini' -g '*.ned' -g '!{os.path.basename(file_path)}' -C 20 '{fname_without_ext}' $(git rev-parse --show-toplevel) | head -n 3000 > {file_path}.ctx"""
        os.system(grep_command)
        context_files.append(file_path + ".ctx")

    return context_files

def create_prompt(content, context, task, prompt, file_type):
    command_text = prompt if prompt else generate_command_text(task, file_type)
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

def split_content(file_content, file_type, max_chars):
    if file_type == "tex":
        return split_latex_by_sections(file_content, max_chars)
    elif file_type == "rst":
        return split_rst_by_headings(file_content, max_chars)
    elif file_type == "py":
        return split_python_by_defs(file_content, max_chars)
    else:
        return [file_content]

def split_latex_by_sections(latex_source, max_chars):
    # Regex to match section headers
    pattern = r'(\\section\{.*?\})'
    return split_by_regex(latex_source, pattern, max_chars)

def split_rst_by_headings(rst_source, max_chars):
    # Regular expression to match RST headings (title and underline)
    pattern = r'(^.*\n(={3,}|-{3,}|`{3,}|:{3,}|\+{3,}|\*{3,}|\#{3,}|\^{3,}|"{3,}|~{3,})$)'
    return split_by_regex(rst_source, pattern, max_chars)

def split_python_by_defs(python_source, max_chars):
    # Regular expression to match Python toplevel functions and classes
    pattern = r'(^(def|class) )'
    return split_by_regex(python_source, pattern, max_chars)

def split_by_regex(text, regex_pattern, max_chars):
    # Split to chunks
    regex = re.compile(regex_pattern, re.MULTILINE)
    matches = list(regex.finditer(text))
    split_indices = [0] + [match.start() for match in matches] + [len(text)]
    chunks = [text[split_indices[i]:split_indices[i+1]] for i in range(1, len(split_indices)-1)]

    # Merge smaller chunks
    parts = []
    current_part = ""
    for chunk in chunks:
        if len(current_part) + len(chunk) > max_chars:
            parts.append(current_part)
            current_part = ""
        current_part += chunk
    parts.append(current_part)

    return parts

def extract(reply_text, original_content):
    content = reply_text
    if content.count("```") >= 2:
        content = re.sub(r"^.*```.*?\n(.*\n)```.*$", r"\1", content, 1, re.DOTALL)

    trailing_whitespace_len = len(original_content) - len(original_content.rstrip())
    original_trailing_whitespace = original_content[-trailing_whitespace_len:]
    content = content.rstrip() + original_trailing_whitespace
    return content

def apply_command_to_file(file_path, context_files, file_type, task, prompt, model, chunk_size=None, save_prompt=False):
    with open(file_path, 'r', encoding='utf-8') as file:
        file_content = file.read()

    context_files = context_files or []
    context_files += find_additional_context_files(file_path, file_type, task)
    if context_files:
        print("   context files: " + " ".join(context_files))
    context = read_files(context_files)

    if chunk_size is None:
        chunk_size = get_llm_context_window(model)*2  - len(context) # assume average token length of 2 chars
    modified_content = ""
    parts = split_content(file_content, file_type, chunk_size)
    for i, part in enumerate(parts):
        if len(parts) > 1:
            print(f"   part {i + 1}/{len(parts)}")
        prompt = create_prompt(part, context, task, prompt, file_type)

        if save_prompt:
            with open(file_path+".prompt"+str(i), 'w', encoding='utf-8') as file:
                file.write(prompt)

        reply_text = invoke_llm(prompt, model)
        modified_content += extract(reply_text, part)

    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(modified_content)
    _logger.debug(f"Modified {file_path} successfully.")

def apply_command_to_files(file_list, context_files, file_type, task, prompt, model, chunk_size=None, save_prompt=False):
    n = len(file_list)
    for i, file_path in enumerate(file_list):
        try:
            print(f"Processing file {i + 1}/{n} {file_path}")
            apply_command_to_file(file_path, context_files, file_type, task, prompt, model, chunk_size=chunk_size, save_prompt=save_prompt)
        except Exception as e:
            print(f"-> Exception: {e}")

def resolve_file_list(paths, file_type, file_ext=None):
    file_extension_patterns = {
        "md":  r".*.md$",
        "rst": r".*.rst$",
        "tex": r".*.tex$",
        "ned": r".*.ned$",
        "py":  r".*.py$"
    }

    if file_ext:
        filename_regex = re.compile(rf".*.{file_ext}$")
    else:
        if file_type not in file_extension_patterns:
            raise ValueError("Unsupported file type.")
        filename_regex = re.compile(file_extension_patterns[file_type])

    file_list = []
    for path in paths:
        if os.path.isdir(path):
            file_list.extend(collect_matching_file_paths(path, filename_regex))
        elif os.path.isfile(path) and filename_regex.match(path):
            file_list.append(path)
    return sorted(file_list)

def process_files(paths, context_files, file_type, file_ext, task, prompt, model_name, chunk_size=None, save_prompt=False):
    if not model_name:
        model_name = get_recommended_model(task)
    model = llm.get_model(model_name)
    model.key = ''
    file_list = resolve_file_list(paths, file_type, file_ext)
    print("Files to process: " + " ".join(file_list))
    print("Using LLM: " + model_name)
    del model_name, file_ext, paths
    apply_command_to_files(**locals())

def main():
    parser = argparse.ArgumentParser(description="Process and improve specific types of files in a given directory or files.")
    parser.add_argument("paths", type=str, nargs='+', help="The directories or files to process.")
    parser.add_argument("--file-type", type=str, choices=["md", "rst", "tex", "ned", "py"], help="The type of files to process.")
    parser.add_argument("--file-ext", type=str, help="The extension of files to process. Takes precedence over --file-type.")
    parser.add_argument("--task", type=str, choices=["proofread", "improve-language", "eliminate-you-addressing", "neddoc"], help="The task to perform on the files.")
    parser.add_argument("--prompt", type=str, help="The LLM prompt to use. Generic instructions and the content of context files will be appended. Takes precedence over --task.")
    parser.add_argument("--model", type=str, dest="model_name", default=None, help="The name of the LLM model to use.")
    parser.add_argument("--context", type=str, nargs='*', dest="context_files", help="The context files to be used.")
    parser.add_argument("--chunk-size", type=int, default=None, help="The maximum number of characters to be sent to the LLM model at once.")
    parser.add_argument("--save-prompt", action='store_true', help="Save the LLM prompt for each input file as <filename>.prompt.")

    args = parser.parse_args()
    process_files(**vars(args))

if __name__ == "__main__":
    main()
