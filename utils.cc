// utils.cc
#include <cmath>
#include <deque>
#include <vector>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

class MarketMaker {
private:
    double position;
    double ref_price;
    std::deque<double> price_history;
    size_t vol_window = 20;
    
public:
    MarketMaker() : position(0), ref_price(0) {}
    
    double calculateVolatility() {
        if (price_history.size() < 2) return 1.0;
        
        std::vector<double> returns;
        for (size_t i = 1; i < price_history.size(); ++i) {
            double ret = std::log(price_history[i] / price_history[i-1]);
            returns.push_back(ret);
        }
        
        double mean = 0.0;
        for (double ret : returns) {
            mean += ret;
        }
        mean /= returns.size();
        
        double variance = 0.0;
        for (double ret : returns) {
            variance += (ret - mean) * (ret - mean);
        }
        variance /= (returns.size() - 1);
        
        double vol = std::sqrt(variance) * std::sqrt(252);
        return std::max(vol, 0.01);
    }
    
    void updatePriceHistory(double price) {
        price_history.push_back(price);
        if (price_history.size() > vol_window) {
            price_history.pop_front();
        }
    }
    
    void calculateSpread(double price, double volume, double* bid, double* offer) {
        ref_price = price;
        updatePriceHistory(price);
        double vol_multiplier = calculateVolatility();
        
        if (position == 0) {
            double spread = 0.02 * vol_multiplier;
            *bid = price * (1.0 - spread);
            *offer = price * (1.0 + spread);
            return;
        }
        
        if (position > 0) {
            *bid = price * (1.0 - 0.01 * vol_multiplier);
            *offer = price * (1.0 + 0.07 * vol_multiplier);
            return;
        }
        
        *bid = price * (1.0 - 0.07 * vol_multiplier);
        *offer = price * (1.0 + 0.01 * vol_multiplier);
    }

    void updatePosition(double volume, bool is_buy) {
        position += is_buy ? volume : -volume;
    }

    double getPosition() const {
        return position;
    }
    
    double getVolatility() const {
        return calculateVolatility();
    }
};

extern "C" {
    EXPORT MarketMaker* createMarketMaker() {
        return new MarketMaker();
    }
    
    EXPORT void deleteMarketMaker(MarketMaker* mm) {
        delete mm;
    }
    
    EXPORT void calculateSpread(MarketMaker* mm, double price, double volume, double* bid, double* offer) {
        mm->calculateSpread(price, volume, bid, offer);
    }
    
    EXPORT void updatePosition(MarketMaker* mm, double volume, bool is_buy) {
        mm->updatePosition(volume, is_buy);
    }
    
    EXPORT double getPosition(MarketMaker* mm) {
        return mm->getPosition();
    }
    
    EXPORT double getVolatility(MarketMaker* mm) {
        return mm->getVolatility();
    }
}