import utils
import numpy as np
import pandas as pd

class PythonMarketMaker:
    def __init__(self):
        self.mm = utils.MarketMaker()
        self.trades = []
        self.positions = {}
        self.price_history = {}
        
    def quote_price(self, ticker, ref_price, volume):
        # Get current position
        pos = self.positions.get(ticker, 0)
        
        # Store price history for volatility calculation
        if ticker not in self.price_history:
            self.price_history[ticker] = []
        self.price_history[ticker].append(ref_price)
        
        # Calculate spread based on position and volatility
        bid, offer = self.mm.calculate_spread(ref_price, pos)
        
        # Get current volatility for logging
        vol = self.mm.get_volatility()
        
        return {
            'ticker': ticker,
            'bid': bid,
            'offer': offer,
            'ref_price': ref_price,
            'volume': volume,
            'volatility': vol
        }
        
    def execute_trade(self, quote, is_buy):
        ticker = quote['ticker']
        price = quote['bid'] if is_buy else quote['offer']
        volume = quote['volume']
        
        # Update position
        self.mm.update_position(volume, is_buy)
        self.positions[ticker] = self.positions.get(ticker, 0) + (volume if is_buy else -volume)
        
        # Record trade with volatility info
        self.trades.append({
            'ticker': ticker,
            'price': price,
            'volume': volume,
            'side': 'buy' if is_buy else 'sell',
            'position': self.positions[ticker],
            'volatility': quote['volatility'],
            'spread_pct': (quote['offer'] - quote['bid']) / quote['ref_price']
        })
        
    def get_positions(self):
        return self.positions
        
    def get_trades(self):
        return pd.DataFrame(self.trades)
    
    def get_market_stats(self, ticker):
        trades_df = self.get_trades()
        ticker_trades = trades_df[trades_df['ticker'] == ticker]
        
        return {
            'avg_spread': ticker_trades['spread_pct'].mean(),
            'avg_vol': ticker_trades['volatility'].mean(),
            'position': self.positions.get(ticker, 0)
        }

def main():
    mm = PythonMarketMaker()
    
    # Example usage with price series
    prices = [150.0, 151.2, 149.8, 152.3, 151.5]
    for price in prices:
        quote = mm.quote_price('AAPL', price, 100)
        print(f"Quote: {quote}")
        
        # Simulate random trading
        if np.random.random() > 0.5:
            mm.execute_trade(quote, np.random.random() > 0.5)
    
    # Print trading stats
    print("\nTrading Statistics:")
    print(mm.get_market_stats('AAPL'))
    print("\nTrade History:")
    print(mm.get_trades())

if __name__ == "__main__":
    main()