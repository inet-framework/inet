#!/usr/bin/env python3

import os
from cppindex import CodeBase

def main():
    # Get the base directory of INET
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Parse the to-be-processed.txt file to get class names and paths
    class_list = []
    with open(os.path.join(script_dir, 'to-be-processed.txt'), 'r', encoding='utf-8') as f:
        for line in f:
            if line.strip():
                parts = line.strip().split()
                if len(parts) >= 2:
                    class_name = parts[0]
                    file_path = parts[1]
                    class_list.append((class_name, file_path))
    
    # Create source directories list based on the file paths
    source_dirs = set()
    for _, file_path in class_list:
        source_dir = os.path.join(script_dir, os.path.dirname(file_path))
        source_dirs.add(source_dir)
    
    # Initialize the CodeBase and analyze the source code
    print("Analyzing codebase with Doxygen (this may take a while)...")
    codebase = CodeBase()
    codebase.analyze(list(source_dirs), project_name="INET")
    
    # Process each class and print its state variables
    print("\nClass State Variables:")
    print("======================")
    
    for class_name, file_path in class_list:
        print(f"\n{class_name} [{file_path}]:")
        
        # Find the class in the codebase
        matching_classes = codebase.get_classes_by_name(class_name)
        
        if not matching_classes:
            print(f"  Class not found in the analyzed codebase")
            continue
        
        # If multiple classes with the same name, find the one with matching file path
        found_class = None
        for cls in matching_classes:
            location = cls.get_location()
            if location and file_path in location.file:
                found_class = cls
                break
        
        if not found_class and matching_classes:
            # If no exact match found but classes with this name exist, use the first one
            found_class = matching_classes[0]
        
        if found_class:
            # Print the state variables (non-static, access type != 'private' variables)
            variables = [var for var in found_class.variables if not var.is_static]
            
            if not variables:
                print("  No state variables found")
            else:
                for var in variables:
                    access_marker = ""
                    if var.access == 'private':
                        access_marker = "- "
                    elif var.access == 'protected':
                        access_marker = "# "
                    elif var.access == 'public':
                        access_marker = "+ "
                    
                    type_str = var.type.replace("\n", " ").strip()
                    print(f"  {access_marker}{var.name}: {type_str}")
        else:
            print("  Class not found in the analyzed codebase")

if __name__ == "__main__":
    main()
