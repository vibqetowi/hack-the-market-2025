# main.py
import ctypes
import os
from AmplifyQuantTrading import Data, Exchange, MarketMaker, HedgeFund as hf

# Load the shared library directly, assuming it's in the same directory
lib = ctypes.CDLL("./libutils.so") 

# Define the argument and return types of the C++ functions
lib.add.argtypes = [ctypes.c_int, ctypes.c_int]
lib.add.restype = ctypes.c_int

lib.greet.argtypes = [ctypes.c_char_p]
lib.greet.restype = None  # Void function

# Call the C++ functions
result = lib.add(5, 3)
print(f"Result from C++ add function: {result}")

lib.greet(b"Blin")  # Pass a byte string