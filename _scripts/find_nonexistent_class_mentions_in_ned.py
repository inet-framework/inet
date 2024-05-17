#
# This script reports mentions of apparently non-existent C++ classes from
# NED file comments. In NED files, only backticked phrases are considered.
# The class list is extracted from .h files only; msg files are not looked at,
# so INET should be built before running this script.
#

import os
import re

def remove_comments(text):
    # Remove single-line comments
    text = re.sub(r'//.*', '', text)
    # Remove multi-line comments
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)
    return text

def find_cpp_classes(root_dir):
    class_pattern = re.compile(r'class\s+(?:INET_API\s+)?(\w+)')
    class_names = []

    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.endswith('.h'):
                filepath = os.path.join(dirpath, filename)
                with open(filepath, 'r', encoding='utf-8') as file:
                    content = file.read()
                    content = remove_comments(content)
                    matches = class_pattern.findall(content)
                    class_names.extend(matches)

    return class_names

def find_backticked_phrases_in_ned_files(root_dir):
    # Regex to match // comments with backticked phrases
    comment_pattern = re.compile(r'//.*?`(.*?)`')
    occurrences_dict = {}

    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.endswith('.ned'):
                filepath = os.path.join(dirpath, filename)
                with open(filepath, 'r', encoding='utf-8') as file:
                    content = file.read()
                    matches = comment_pattern.findall(content)
                    if matches:
                        occurrences_dict[filepath] = matches

    return occurrences_dict

def find_nonexistent_class_mentions(class_names, backticked_phrases):
    nonexistent_mentions = {}
    class_set = set(class_names)

    ident_pattern = re.compile(r'^[A-Z][A-Za-z0-9_]*$')

    for filepath, phrases in backticked_phrases.items():
        nonexistent = [phrase for phrase in phrases if ident_pattern.match(phrase) and phrase not in class_set]
        if nonexistent:
            nonexistent_mentions[filepath] = nonexistent

    return nonexistent_mentions

def main():
    dir = '.'
    class_names = find_cpp_classes(dir)
    backticked_phrases = find_backticked_phrases_in_ned_files(dir)
    nonexistent_mentions = find_nonexistent_class_mentions(class_names, backticked_phrases)

    print("Non-existent classes mentioned:")
    for filename, phrases in nonexistent_mentions.items():
        print(f'{filename} ---> {" ".join(phrases)}')

if __name__ == '__main__':
    main()
