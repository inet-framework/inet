#!/usr/bin/env python3
"""
OMNET++ Module Analyzer - Command line tool to analyze OMNET++ simple modules

This tool identifies classes that define OMNET++ simple modules and lists their member variables.
"""

import sys
import os
import argparse
from omnetpp.cppindex import CodeBase

def is_omnetpp_simple_module(class_info):
    """
    Determines if a class is an OMNET++ simple module by checking if it inherits
    from 'omnetpp::cSimpleModule' either directly or indirectly, or if it is registered
    with the Define_Module macro.
    """
    # Check if the class is explicitly registered with Define_Module macro
    # by looking at any of the source files for this class
    for source_file in class_info.get_source_files():
        if source_file.endswith('.cc'):
            try:
                with open(source_file, 'r') as f:
                    content = f.read()
                    if f"Define_Module({class_info.name})" in content:
                        return True
            except Exception as e:
                print(f"Warning: Could not read {source_file}: {e}")
    
    # Check direct inheritance
    if "omnetpp::cSimpleModule" in class_info.get_base_class_names():
        return True
    
    # Check indirect inheritance through all base classes
    for base in class_info.get_all_base_classes():
        if "omnetpp::cSimpleModule" in base.get_base_class_names():
            return True
    
    return False

def analyze_module(class_info, verbose=False, output_file=None):
    """
    Analyze an OMNET++ module class and print its member variables.
    
    Args:
        class_info: ClassInfo object representing the module
        verbose: If True, print additional information about the class
        output_file: Optional file handle to write output to
    """
    # Get the source file path
    source_files = class_info.get_source_files()
    file_path = source_files[0] if source_files else "Unknown file path"
    
    # Print file path
    file_line = f"\nFile: {file_path}"
    if output_file:
        output_file.write(file_line + "\n")
    print(file_line)
    
    # Print class name
    class_line = f"Class: {class_info.qualified_name}"
    if output_file:
        output_file.write(class_line + "\n")
    print(class_line)
    
    # Use the class's variable list property
    variables = class_info.variables
    
    if not variables:
        no_vars_line = "  No member variables found"
        if output_file:
            output_file.write(no_vars_line + "\n")
        print(no_vars_line)
        return
    
    vars_header = "  Member Variables:"
    if output_file:
        output_file.write(vars_header + "\n")
    print(vars_header)
    
    for var in variables:
        var_line = f"    {var.type} {var.name}"
        if output_file:
            output_file.write(var_line + "\n")
        print(var_line)
        
        if verbose:
            location = var.get_location()
            if location:
                loc_line = f"      Defined in: {location.file}:{location.line}"
                if output_file:
                    output_file.write(loc_line + "\n")
                print(loc_line)

def main():
    parser = argparse.ArgumentParser(description='Analyze OMNET++ simple modules and their member variables')
    parser.add_argument('source_dirs', nargs='+', help='Source directories or files to analyze')
    parser.add_argument('--output-dir', '-o', default='doxy', help='Output directory for Doxygen files (default: doxy)')
    parser.add_argument('--output-file', '-f', help='Write output to this file instead of stdout')
    parser.add_argument('--doxygen', default='doxygen', help='Path to Doxygen executable (default: doxygen)')
    parser.add_argument('--project-name', default='OMNET++ Project', help='Project name for Doxygen')
    parser.add_argument('--verbose', '-v', action='store_true', help='Print verbose information')
    parser.add_argument('--skip-doxygen', action='store_true', help='Skip running Doxygen (use existing XML)')
    args = parser.parse_args()
    
    codebase = CodeBase()
    
    if not args.skip_doxygen:
        print(f"Running Doxygen on {', '.join(args.source_dirs)}...")
        codebase.run_doxygen(
            source_dir_or_dirs=args.source_dirs,
            project_name=args.project_name,
            output_directory=args.output_dir,
            doxygen=args.doxygen
        )
    
    xml_dir = os.path.join(args.output_dir, 'xml')
    print(f"Reading Doxygen XML output from {xml_dir}...")
    codebase.read(xml_dir)
    codebase.resolve_references()
    
    # Find all OMNET++ simple modules
    simple_modules = [cls for cls in codebase.get_classes() if is_omnetpp_simple_module(cls)]
    
    if not simple_modules:
        print("No OMNET++ simple modules found in the specified source directories.")
        return
    
    summary_line = f"\nFound {len(simple_modules)} OMNET++ simple modules:"
    print(summary_line)
    
    # Open output file if specified
    output_file = None
    if args.output_file:
        try:
            output_file = open(args.output_file, 'w')
            output_file.write(summary_line + "\n")
        except Exception as e:
            print(f"Error opening output file {args.output_file}: {e}")
            return
    
    try:
        for module in simple_modules:
            analyze_module(module, args.verbose, output_file)
    finally:
        # Close output file if opened
        if output_file:
            output_file.close()
            print(f"\nOutput written to {args.output_file}")

if __name__ == "__main__":
    main()
