// utils.cc
#include <cmath>
#include <deque>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

class Position {
private:
    double volume;
    double avg_price;

public:
    Position() : volume(0), avg_price(0) {}

    void update(double trade_volume, double trade_price, bool is_buy) {
        if (volume == 0) {
            volume = is_buy ? trade_volume : -trade_volume;
            avg_price = trade_price;
            return;
        }

        double new_volume = volume + (is_buy ? trade_volume : -trade_volume);
        avg_price = (avg_price * volume + trade_price * trade_volume) / 
                   (std::abs(volume) + trade_volume);
        volume = new_volume;
    }

    double getVolume() const { return volume; }
    double getAvgPrice() const { return avg_price; }
};

class MarketMaker {
private:
    std::unordered_map<std::string, Position> positions;
    std::unordered_map<std::string, std::deque<double>> price_history;
    const size_t vol_window = 20;
    const double min_spread = 0.001;  // 10 bps minimum spread
    const double max_spread = 0.10;   // 1000 bps maximum spread

    double calculateVolatility(const std::string& ticker) {
        auto& history = price_history[ticker];
        if (history.size() < 2) return 1.0;

        std::vector<double> returns;
        returns.reserve(history.size() - 1);

        for (size_t i = 1; i < history.size(); ++i) {
            returns.push_back(std::log(history[i] / history[i-1]));
        }

        double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
        
        double variance = std::accumulate(returns.begin(), returns.end(), 0.0,
            [mean](double acc, double ret) {
                double diff = ret - mean;
                return acc + diff * diff;
            }) / (returns.size() - 1);

        double vol = std::sqrt(variance) * std::sqrt(252);
        return std::clamp(vol, 0.01, 1.0);
    }

    void updatePriceHistory(const std::string& ticker, double price) {
        auto& history = price_history[ticker];
        history.push_back(price);
        if (history.size() > vol_window) {
            history.pop_front();
        }
    }

public:
    struct Quote {
        double bid;
        double offer;
        double volatility;
    };

    Quote calculateSpread(const std::string& ticker, double price, double volume) {
        updatePriceHistory(ticker, price);
        double vol_multiplier = calculateVolatility(ticker);
        double pos_volume = positions[ticker].getVolume();

        Quote quote;
        quote.volatility = vol_multiplier;

        if (pos_volume == 0) {
            // Neutral position - symmetric spread
            double spread = std::clamp(0.02 * vol_multiplier, min_spread, max_spread);
            quote.bid = price * (1.0 - spread);
            quote.offer = price * (1.0 + spread);
        }
        else if (pos_volume > 0) {
            // Long position - tighter bid to reduce position
            quote.bid = price * (1.0 - std::clamp(0.01 * vol_multiplier, min_spread, max_spread));
            quote.offer = price * (1.0 + std::clamp(0.07 * vol_multiplier, min_spread, max_spread));
        }
        else {
            // Short position - tighter offer to reduce position
            quote.bid = price * (1.0 - std::clamp(0.07 * vol_multiplier, min_spread, max_spread));
            quote.offer = price * (1.0 + std::clamp(0.01 * vol_multiplier, min_spread, max_spread));
        }

        return quote;
    }

    void updatePosition(const std::string& ticker, double volume, double price, bool is_buy) {
        positions[ticker].update(volume, price, is_buy);
    }

    double getPosition(const std::string& ticker) const {
        auto it = positions.find(ticker);
        return it != positions.end() ? it->second.getVolume() : 0.0;
    }

    double getAvgPrice(const std::string& ticker) const {
        auto it = positions.find(ticker);
        return it != positions.end() ? it->second.getAvgPrice() : 0.0;
    }
};

extern "C" {
    EXPORT MarketMaker* createMarketMaker() {
        return new MarketMaker();
    }

    EXPORT void deleteMarketMaker(MarketMaker* mm) {
        delete mm;
    }

    EXPORT void calculateSpread(MarketMaker* mm, const char* ticker, double price, double volume,
                              double* bid, double* offer, double* volatility) {
        auto quote = mm->calculateSpread(ticker, price, volume);
        *bid = quote.bid;
        *offer = quote.offer;
        *volatility = quote.volatility;
    }

    EXPORT void updatePosition(MarketMaker* mm, const char* ticker, double volume, double price, bool is_buy) {
        mm->updatePosition(ticker, volume, price, is_buy);
    }

    EXPORT double getPosition(MarketMaker* mm, const char* ticker) {
        return mm->getPosition(ticker);
    }

    EXPORT double getAvgPrice(MarketMaker* mm, const char* ticker) {
        return mm->getAvgPrice(ticker);
    }
}
