#ifndef GOLOS_LEAKY_BUCKET_H
#define GOLOS_LEAKY_BUCKET_H


#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <set>
#include <list>
#include <unordered_set>
#include <vector>


namespace golos {
    namespace plugins {
        namespace webserver {

            using namespace std::chrono;

            class leaky_bucket final {
            public:
                using clock = std::chrono::steady_clock;
                using millisecond = std::chrono::milliseconds;
                using second = std::chrono::milliseconds;

                leaky_bucket(const uint64_t lim = 0, const uint64_t period = 1)
                        : limit(lim)
                         ,time_period(period) {
                };

                leaky_bucket() = delete;
                ~leaky_bucket() = default;
                leaky_bucket(const leaky_bucket &) = default;
                leaky_bucket &operator=(const leaky_bucket &)= default;
                leaky_bucket(leaky_bucket &&) = default;
                leaky_bucket &operator=(leaky_bucket &&)= default;

                bool increment();

            private:
                bool update();

                // Time stamps of bucket increments
                std::list<clock::time_point> increment_stamps;
                const uint64_t limit;
                const uint64_t time_period;
            };

            typedef std::shared_ptr<leaky_bucket> leaky_bucket_ptr;
        }
    }
}

#endif //GOLOS_LEAKY_BUCKET_H
