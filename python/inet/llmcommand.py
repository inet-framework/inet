import llm
import os
import re

# Set your OpenAI API key here
# model = llm.get_model("orca-mini-3b-gguf2-q4_0")
# model = llm.get_model("gpt4all-falcon-newbpe-q4_0")
# model = llm.get_model("claude-3-opus")
# model.key = ''
# model = llm.get_model("claude-2")

model = llm.get_model("gpt-4")
# model.key = ''
model.key = ''

import os
import re

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

def apply_command_to_files(file_list, context_file_list, command_text):
    context = []
    for context_file_path in context_file_list:
        # Read the file content
        with open(context_file_path, 'r', encoding='utf-8') as file:
            context.append(file.read())
    context = "\n".join(context)
    for i, file_path in enumerate(file_list):
        print(f"{i}/{len(file_list)} {file_path}:")
        # Read the file content
        with open(file_path, 'r', encoding='utf-8') as file:
            file_content = file.read()

        # Combine the command and file content
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

        # Send the prompt to the LLM model
        print("PROMPT ", prompt)
        modified_content = model.prompt(prompt)
        result = modified_content.text()
        result = result.replace("\n```", "")
        print("RESULT ", result)

        # Save the modified content back to the file
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(result)

        print(f"Modified {file_path} successfully.")

inet_root = "/home/andras/projects/inet"

def test1():
    file_list = [inet_root + '/src/inet/networklayer/ipv4/Ipv4.h']
    context_file_list = [inet_root + '/src/inet/queueing/contract/IPassivePacketSink.h']
    command_text = "Remove all methods that implement the IPassivePacketSink interface."
    apply_command_to_files(file_list, context_file_list, command_text)

def test2():
    file_list = collect_matching_file_paths(inet_root + "/src/inet", ".*\.h", "IPassivePacketSink")
    file_list = file_list + [file[:-2] + '.cc' if file.endswith('.h') else file for file in file_list]
    context_file_list = [inet_root + '/src/inet/queueing/contract/IPassivePacketSink.h']
    command_text = "Remove all methods that implement the IPassivePacketSink interface."
    apply_command_to_files(file_list, context_file_list, command_text)

def test3():
    file_list = [inet_root + '/src/inet/networklayer/ipv4/Ipv4.h']
    context_file_list = [inet_root + '/commit']
    command_text = "Based on the changes in the context remove obsolete include statements, base classes and methods."
    apply_command_to_files(file_list, context_file_list, command_text)

def proofread_ned_comments():
    file_list = collect_matching_file_paths(inet_root + "/src", r".*.ned$", None)
    print(f"{file_list=}")
    context_file_list = []
    command_text = "Here is an OMNeT++ NED file, fix any English mistakes in the comments (lines starting with `//`) in it."
    apply_command_to_files(file_list, context_file_list, command_text)

proofread_ned_comments()
