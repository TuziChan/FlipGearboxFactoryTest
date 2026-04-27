#ifndef STATIONRUNTIMEFACTORY_H
#define STATIONRUNTIMEFACTORY_H

#include "StationRuntime.h"
#include "StationConfig.h"
#include <memory>

namespace Infrastructure {
namespace Config {

/**
 * @brief Factory for creating StationRuntime from configuration
 */
class StationRuntimeFactory {
public:
    /**
     * @brief Create runtime from station configuration
     */
    static std::unique_ptr<StationRuntime> create(const StationConfig& config);

private:
    StationRuntimeFactory() = delete;
};

} // namespace Config
} // namespace Infrastructure

#endif // STATIONRUNTIMEFACTORY_H
