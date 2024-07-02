import os
import re

#
# CAUTION: This script does its best, but after running it, the result should be carefully reviewed and filtered!
#

def process_file(file_path):
    # regex pattern for camelcase identifiers
    pattern = re.compile(r'([a-zA-Z0-9_:\.]*[a-z][A-Z]+[a-zA-Z0-9]*)')

    with open(file_path, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    with open(file_path, 'w', encoding='utf-8') as file:
        for line in lines:
            if line.startswith('//'):
                line = pattern.sub(r'`\1`', line)
                # corrections:
                for word in ["NetLab", "OpenSim", "OMNeT", "QoS", "ToS", "VoIP", "gPTP", "MiXiM", "BibTeX", "MoBAN", "CoCo", "dB", "dBm"]:
                    line = line.replace(f"`{word}`", word)
                for word in ["<tt>", "</tt>", "<code>", "</code>" ]:
                    line = line.replace(word, "`")
                line = re.sub(r"`([a-zA-Z0-9_:\.]+)`\.ned", r'\1.ned', line)
                line = re.sub(r"`([a-zA-Z0-9_:\.]+)`\.cc", r'\1.cc', line)
                line = re.sub(r"=( *)`([a-zA-Z0-9_:\.]+)`", r'=\1\2', line)
                line = re.sub(r"`([a-zA-Z0-9_:\.]+)`( *)=", r'\1\2=', line)
                line = re.sub(r"~`([a-zA-Z0-9_:\.]+)`", r'~\1', line)
                line = re.sub(r"<`([a-zA-Z0-9_:\.]+)`", r'<\1', line)
                line = re.sub(r"</`([a-zA-Z0-9_:\.]+)`", r'</\1', line)
                line = re.sub(r"@`([a-zA-Z0-9_:\.]+)`", r'`@\1`', line)
                line = re.sub(r"`([a-zA-Z0-9_:\.]+)`\*", r'`\1*`', line)
                line = re.sub(r"\"`([a-zA-Z0-9_:\.]+)`\"", r'`\1`', line)
                line = re.sub(r"'`([a-zA-Z0-9_:\.]+)`'", r'`\1`', line)
                line = re.sub(r"`([a-zA-Z0-9_:\.]+)`(\(.*?\))", r'`\1\2`', line)
                line = re.sub(r"`([a-zA-Z0-9_:\.]+)`(\[.*?\])", r'`\1\2`', line)
                line = re.sub(r"``+", r'`', line)
            file.write(line)

def process_folder(folder_path):
    for root, _, files in os.walk(folder_path):
        for file in files:
            if file.endswith('.ned'):
                process_file(os.path.join(root, file))

if __name__ == "__main__":
    process_folder(".")
    print("Processing complete.")
