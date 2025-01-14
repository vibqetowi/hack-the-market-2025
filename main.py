import ctypes
import numpy as np
import pandas as pd
from pathlib import Path

# Load the shared library
lib_path = Path(__file__).parent / "utils.so"  # Use .dll on Windows
lib = ctypes.CDLL(str(lib_path))

# Define C++ function interfaces
lib.createMarketMaker.restype = ctypes.c_void_p
lib.createMarketMaker.argtypes = []

lib.deleteMarketMaker.argtypes = [ctypes.c_void_p]

lib.calculateSpread.argtypes = [
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_double,
    ctypes.c_double,
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double)
]

lib.updatePosition.argtypes = [
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_double,
    ctypes.c_double,
    ctypes.c_bool
]

lib.getPosition.restype = ctypes.c_double
lib.getPosition.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

lib.getAvgPrice.restype = ctypes.c_double
lib.getAvgPrice.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

class MarketMaker:
    def __init__(self):
        self._mm = lib.createMarketMaker()
        self.trades = []
        
    def __del__(self):
        if hasattr(self, '_mm'):
            lib.deleteMarketMaker(self._mm)
            
    def quote_price(self, ticker: str, price: float, volume: float) -> dict:
        bid = ctypes.c_double()
        offer = ctypes.c_double()
        vol = ctypes.c_double()
        
        lib.calculateSpread(
            self._mm,
            ticker.encode('utf-8'),
            price,
            volume,
            ctypes.byref(bid),
            ctypes.byref(offer),
            ctypes.byref(vol)
        )
        
        return {
            'ticker': ticker,
            'bid': bid.value,
            'offer': offer.value,
            'ref_price': price,
            'volume': volume,
            'volatility': vol.value
        }
        
    def execute_trade(self, quote: dict, is_buy: bool):
        ticker = quote['ticker']
        price = quote['bid'] if is_buy else quote['offer']
        volume = quote['volume']
        
        lib.updatePosition(
            self._mm,
            ticker.encode('utf-8'),
            volume,
            price,
            is_buy
        )
        
        self.trades.append({
            'ticker': ticker,
            'price': price,
            'volume': volume,
            'side': 'buy' if is_buy else 'sell',
            'position': self.get_position(ticker),
            'avg_price': self.get_avg_price(ticker),
            'volatility': quote['volatility'],
            'spread_pct': (quote['offer'] - quote['bid']) / quote['ref_price']
        })
        
    def get_position(self, ticker: str) -> float:
        return lib.getPosition(self._mm, ticker.encode('utf-8'))
        
    def get_avg_price(self, ticker: str) -> float:
        return lib.getAvgPrice(self._mm, ticker.encode('utf-8'))
        
    def get_trades(self) -> pd.DataFrame:
        return pd.DataFrame(self.trades)
    
    def get_market_stats(self, ticker: str) -> dict:
        trades_df = self.get_trades()
        ticker_trades = trades_df[trades_df['ticker'] == ticker]
        
        return {
            'avg_spread': ticker_trades['spread_pct'].mean(),
            'avg_vol': ticker_trades['volatility'].mean(),
            'position': self.get_position(ticker),
            'avg_price': self.get_avg_price(ticker)
        }

def main():
    mm = MarketMaker()
    
    # Example usage with price series
    prices = [150.0, 151.2, 149.8, 152.3, 151.5]
    for price in prices:
        quote = mm.quote_price('AAPL', price, 100)
        print(f"Quote: {quote}")
        
        # Simulate random trading
        if np.random.random() > 0.5:
            mm.execute_trade(quote, np.random.random() > 0.5)
    
    print("\nTrading Statistics:")
    print(mm.get_market_stats('AAPL'))
    print("\nTrade History:")
    print(mm.get_trades())

if __name__ == "__main__":
    main()