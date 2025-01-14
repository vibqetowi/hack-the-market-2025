// utils.cc
#include <boost/python.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <deque>

class MarketMaker {
private:
    double position;
    double ref_price;
    std::deque<double> price_history;
    size_t vol_window = 20; // Rolling window for volatility calculation
    
public:
    MarketMaker() : position(0), ref_price(0) {}
    
    double calculateVolatility() {
        if (price_history.size() < 2) return 1.0;
        
        // Calculate returns
        std::vector<double> returns;
        for (size_t i = 1; i < price_history.size(); ++i) {
            double ret = std::log(price_history[i] / price_history[i-1]);
            returns.push_back(ret);
        }
        
        // Calculate standard deviation
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
        
        // Annualize volatility (assuming daily prices)
        double vol = std::sqrt(variance) * std::sqrt(252);
        return std::max(vol, 0.01); // Minimum volatility floor
    }
    
    void updatePriceHistory(double price) {
        price_history.push_back(price);
        if (price_history.size() > vol_window) {
            price_history.pop_front();
        }
    }
    
    std::pair<double,double> calculateSpread(double price, double volume) {
        ref_price = price;
        updatePriceHistory(price);
        double vol_multiplier = calculateVolatility();
        
        // No position - symmetric 2% spread adjusted by volatility
        if (position == 0) {
            double spread = 0.02 * vol_multiplier;
            return std::make_pair(price * (1.0 - spread), price * (1.0 + spread));
        }
        
        // Long position - tighter bid (1%), wider offer (7%) adjusted by volatility
        if (position > 0) {
            return std::make_pair(
                price * (1.0 - 0.01 * vol_multiplier),
                price * (1.0 + 0.07 * vol_multiplier)
            );
        }
        
        // Short position - wider bid (7%), tighter offer (1%) adjusted by volatility
        return std::make_pair(
            price * (1.0 - 0.07 * vol_multiplier),
            price * (1.0 + 0.01 * vol_multiplier)
        );
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

BOOST_PYTHON_MODULE(utils) {
    using namespace boost::python;
    
    class_<MarketMaker>("MarketMaker")
        .def(init<>())
        .def("calculate_spread", &MarketMaker::calculateSpread)
        .def("update_position", &MarketMaker::updatePosition)
        .def("get_position", &MarketMaker::getPosition)
        .def("get_volatility", &MarketMaker::getVolatility)
    ;
}