#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QFont>
#include <QDebug>
#include <QDir>

#include "src/infrastructure/config/StationConfig.h"
#include "src/infrastructure/config/ConfigLoader.h"
#include "src/infrastructure/config/DeviceConfigService.h"
#include "src/infrastructure/config/StationRuntimeFactory.h"
#include "src/infrastructure/config/RecipeConfig.h"
#include "src/viewmodels/TestExecutionViewModel.h"
#include "src/viewmodels/DiagnosticsViewModel.h"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QGuiApplication app(argc, argv);

    // Load HarmonyOS Sans fonts
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Regular.ttf");
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Medium.ttf");
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Bold.ttf");
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Light.ttf");

    // Set default application font
    QFont defaultFont("HarmonyOS Sans SC");
    defaultFont.setPixelSize(14);
    app.setFont(defaultFont);

    Infrastructure::Config::StationConfig stationConfig;
    stationConfig.stationId = "STATION-01";
    stationConfig.stationName = "Gearbox Test Station 1";
    stationConfig.aqmdConfig.portName = "COM3";
    stationConfig.aqmdConfig.slaveId = 1;
    stationConfig.aqmdConfig.baudRate = 9600;
    stationConfig.dyn200Config.portName = "COM4";
    stationConfig.dyn200Config.slaveId = 2;
    stationConfig.dyn200Config.baudRate = 9600;
    stationConfig.encoderConfig.portName = "COM5";
    stationConfig.encoderConfig.slaveId = 3;
    stationConfig.encoderConfig.baudRate = 9600;
    stationConfig.encoderConfig.encoderResolution = 4096;
    stationConfig.brakeConfig.portName = "COM6";
    stationConfig.brakeConfig.slaveId = 4;
    stationConfig.brakeConfig.baudRate = 9600;
    stationConfig.brakeChannel = 1;
    stationConfig.defaultRecipe = "GBX-42A";

    const QString stationConfigPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../config/station.json");
    Infrastructure::Config::ConfigLoader configLoader;
    if (!configLoader.loadStationConfig(stationConfigPath, stationConfig)) {
        qWarning() << "Failed to load station config from" << stationConfigPath << ":" << configLoader.lastError();
    }

    // Create station runtime
    auto runtime = Infrastructure::Config::StationRuntimeFactory::create(stationConfig);
    
    // Initialize runtime (will fail if devices not connected, but that's OK for UI testing)
    if (!runtime->initialize()) {
        qWarning() << "Failed to initialize station runtime:" << runtime->lastError();
        qWarning() << "Continuing anyway for UI testing...";
    }

    // Create ViewModel
    auto viewModel = new ViewModels::TestExecutionViewModel(runtime.get(), &app);
    auto diagnosticsViewModel = new ViewModels::DiagnosticsViewModel(runtime.get(), &app);
    auto deviceConfigService = new Infrastructure::Config::DeviceConfigService(stationConfigPath, &app);

    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Expose ViewModel to QML
    engine.rootContext()->setContextProperty("testViewModel", viewModel);
    engine.rootContext()->setContextProperty("diagnosticsViewModel", diagnosticsViewModel);
    engine.rootContext()->setContextProperty("deviceConfigService", deviceConfigService);
    
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    
    engine.loadFromModule("FlipGearboxFactoryTest", "Main");

    int result = QCoreApplication::exec();
    
    // Cleanup
    runtime->shutdown();
    
    return result;
}
