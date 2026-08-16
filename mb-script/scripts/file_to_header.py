#!/usr/bin/env python3
"""
Script to convert a single file to a C++ header file
"""

import os
import sys

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
    progmem_string = f'const char {var_name}[] PROGMEM = R"rawliteral({content})rawliteral";'
    
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
    
    output_dir = os.path.dirname(output_file)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(header_content)
    
    print(f"Created header file: {output_file}")

def main():
    if len(sys.argv) < 3:
        print("Usage: python file_to_header.py <input_file> <output_file> [var_name]")
        print("Example: python file_to_header.py swagger.yaml src/web/swagger_content.h SWAGGER_CONTENT")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    # Generate variable name from filename if not provided
    if len(sys.argv) > 3:
        var_name = sys.argv[3]
    else:
        var_name = os.path.splitext(os.path.basename(input_file))[0].upper() + "_CONTENT"
    
    create_header_file(input_file, output_file, var_name)

if __name__ == "__main__":
    main() 