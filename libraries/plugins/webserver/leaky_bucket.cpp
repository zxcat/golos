#include <golos/plugins/webserver/leaky_bucket.hpp>

namespace golos {
namespace plugins {
namespace webserver {

    bool leaky_bucket::increment() {
        if (per_second == 0)
            return true;

        clock::time_point current_time = clock::now();

        if (increment_stamps.empty()) {
            increment_stamps.push_back(current_time);
            return true;
        }

        // Leave only the increments for the last time period
        while (!increment_stamps.empty() &&
               duration_cast<seconds>(current_time - increment_stamps.front()) > seconds(1)) {
            increment_stamps.pop_front();
        }

        if (increment_stamps.size() >= per_second) {
            return false;
        }
        else {
            increment_stamps.push_back(current_time);
            return true;
        }
    }
}}}