# main.py
import ctypes
import os

# Determine the directory the script is running in
script_dir = os.path.dirname(os.path.abspath(__file__))

# Construct the full path to the shared library
lib_path = os.path.join(script_dir, "libutils.dylib")

# Load the shared library
# Use the full path
lib = ctypes.CDLL(lib_path)

# Define the argument and return types of the C++ functions
lib.add.argtypes = [ctypes.c_int, ctypes.c_int]
lib.add.restype = ctypes.c_int

lib.greet.argtypes = [ctypes.c_char_p]
lib.greet.restype = None  # Void function

# Call the C++ functions
result = lib.add(5, 3)
print(f"Result from C++ add function: {result}")

lib.greet(b"World")  # Pass a byte string