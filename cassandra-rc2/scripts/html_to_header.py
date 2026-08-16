#!/usr/bin/env python3
import os
import sys
import re

def escape_char(char):
    """Escape special characters for C++ strings"""
    if char == '\n':
        return '\\n'
    elif char == '\r':
        return '\\r'
    elif char == '"':
        return '\\"'
    elif char == '\\':
        return '\\\\'
    return char

def file_to_progmem_string(file_path, var_name):
    """Convert a file to a C++ PROGMEM string declaration"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Escape special characters
    escaped_content = ''.join(escape_char(c) for c in content)
    
    # Create the C++ code
    progmem_string = f'const char {var_name}[] PROGMEM = R"rawliteral({escaped_content})rawliteral";'
    
    return progmem_string

def create_header_file(input_file, output_file, var_name):
    """Create a C++ header file with a PROGMEM string variable"""
    progmem_string = file_to_progmem_string(input_file, var_name)
    
    file_name = os.path.basename(output_file).upper().replace('.', '_')
    guard_name = f"{file_name}_H"
    
    header_content = f"""#ifndef {guard_name}
#define {guard_name}

#include <pgmspace.h>

{progmem_string}

#endif // {guard_name}
"""
    
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(header_content)
    
    print(f"Created header file: {output_file}")

def process_file(input_file, output_file, var_name):
    """Process a single file and create a header file"""
    if not os.path.exists(os.path.dirname(output_file)):
        os.makedirs(os.path.dirname(output_file))
    
    create_header_file(input_file, output_file, var_name)

def process_directory(input_dir, output_dir):
    """Process all files in a directory and create header files"""
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    for filename in os.listdir(input_dir):
        input_file = os.path.join(input_dir, filename)
        
        if os.path.isfile(input_file):
            # Generate variable name from filename
            var_name = re.sub(r'[^a-zA-Z0-9_]', '_', os.path.splitext(filename)[0]).upper()
            var_name = f"{var_name}_CONTENT"
            
            # Generate output filename
            output_filename = f"{os.path.splitext(filename)[0]}_content.h"
            output_file = os.path.join(output_dir, output_filename)
            
            create_header_file(input_file, output_file, var_name)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python html_to_header.py <input_path> <output_path> [var_name]")
        sys.exit(1)
    
    input_path = sys.argv[1]
    output_path = sys.argv[2]
    var_name = sys.argv[3] if len(sys.argv) > 3 else None
    
    if os.path.isfile(input_path):
        if var_name is None:
            # Generate variable name from filename
            var_name = re.sub(r'[^a-zA-Z0-9_]', '_', os.path.splitext(os.path.basename(input_path))[0]).upper()
            var_name = f"{var_name}_CONTENT"
        process_file(input_path, output_path, var_name)
    else:
        process_directory(input_path, output_path) 