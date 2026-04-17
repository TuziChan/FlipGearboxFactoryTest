#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>

namespace Infrastructure {
namespace Config {

/**
 * @brief Application-level configuration
 */
struct AppConfig {
    QString appName = "FlipGearboxFactoryTest";
    QString version = "1.0.0";
    QString logPath = "./logs";
    QString reportPath = "./reports";
    int maxLogFiles = 30;
    bool enableDebugLog = false;
};

} // namespace Config
} // namespace Infrastructure

#endif // APPCONFIG_H
