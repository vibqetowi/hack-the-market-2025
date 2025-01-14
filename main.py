import ctypes
import os

# Load the shared library
lib = ctypes.CDLL("./libutils.so")

# Define the MarketMaker opaque pointer type
class MarketMaker(ctypes.Structure):
    _fields_ = []  # No fields are needed, it's an opaque pointer

# Set argument and return types for the C functions
lib.createMarketMaker.restype = ctypes.POINTER(MarketMaker)
lib.destroyMarketMaker.argtypes = [ctypes.POINTER(MarketMaker)]
lib.MarketMaker_calculateBid.argtypes = [ctypes.POINTER(MarketMaker), ctypes.c_double, ctypes.c_double]
lib.MarketMaker_calculateBid.restype = ctypes.c_double
lib.MarketMaker_calculateAsk.argtypes = [ctypes.POINTER(MarketMaker), ctypes.c_double, ctypes.c_double]
lib.MarketMaker_calculateAsk.restype = ctypes.c_double
lib.MarketMaker_updatePosition.argtypes = [ctypes.POINTER(MarketMaker), ctypes.c_double, ctypes.c_bool]
lib.MarketMaker_getPosition.argtypes = [ctypes.POINTER(MarketMaker)]
lib.MarketMaker_getPosition.restype = ctypes.c_double
lib.MarketMaker_getVolatility.argtypes = [ctypes.POINTER(MarketMaker)]
lib.MarketMaker_getVolatility.restype = ctypes.c_double

# Create a MarketMaker instance
mm = lib.createMarketMaker()

# Example usage
price = 100.0
volume = 10.0

bid = lib.MarketMaker_calculateBid(mm, price, volume)
ask = lib.MarketMaker_calculateAsk(mm, price, volume)
print(f"Bid: {bid}, Ask: {ask}")

lib.MarketMaker_updatePosition(mm, 5.0, True)  # Buy 5 units
position = lib.MarketMaker_getPosition(mm)
print(f"Position: {position}")

volatility = lib.MarketMaker_getVolatility(mm)
print(f"Volatility: {volatility}")

# Clean up: Destroy the MarketMaker instance when done
lib.destroyMarketMaker(mm)