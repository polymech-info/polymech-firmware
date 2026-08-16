import os
import sys
import glob
import subprocess

# This script finds all .proto files and runs the nanopb generator on them.
# It's a cross-platform way to avoid shell wildcard expansion issues.

def main():
    # --- Firmware (C) Configuration ---
    fw_proto_dest_dir = "src/"
    generator_script = ".pio/libdeps/waveshare/Nanopb/generator/nanopb_generator.py"
    
    # --- Find Proto Files ---
    proto_src_dir = "proto"
    if not os.path.isdir(proto_src_dir):
        print(f"Source directory '{proto_src_dir}' not found. Skipping.")
        sys.exit(0)
    proto_files = glob.glob(os.path.join(proto_src_dir, '*.proto'))
    if not proto_files:
        print(f"No .proto files found in '{proto_src_dir}'")
        sys.exit(0)
    print(f"Found .proto files: {', '.join(proto_files)}")

    # --- Generate Firmware (C) Files ---
    print("\n--- Generating Firmware C files ---")
    if not os.path.exists(generator_script):
        print(f"Error: Nanopb generator not found at '{generator_script}'")
        print("Please ensure the project has been built once (`npm run build`)")
        sys.exit(1)
    
    os.makedirs(fw_proto_dest_dir, exist_ok=True)
    for proto_file in proto_files:
        run_command([
            sys.executable,
            generator_script,
            "-D", fw_proto_dest_dir,
            proto_file
        ], f"C files for {proto_file}")

def run_command(command, step_name):
    """Helper to run a command and handle errors."""
    print(f"Running for: {step_name}")
    try:
        result = subprocess.run(command, check=True, capture_output=True, text=True)
        
        # Print stdout/stderr only if they contain something
        if result.stdout.strip():
            print(result.stdout.strip())
        if result.stderr.strip():
            print(result.stderr.strip())
        print(f"Successfully generated {step_name}")

    except subprocess.CalledProcessError as e:
        print(f"\n--- ERROR processing {step_name} ---")
        print(f"Command failed with exit code {e.returncode}")
        print("Command:", " ".join(e.cmd))
        print("\n--- STDOUT ---")
        print(e.stdout)
        print("\n--- STDERR ---")
        print(e.stderr)
        sys.exit(1)
    except FileNotFoundError as e:
        print(f"\n--- ERROR ---")
        print(f"Command not found: {e.filename}. Is protobufjs installed and in node_modules?")
        sys.exit(1)


if __name__ == "__main__":
    main() 