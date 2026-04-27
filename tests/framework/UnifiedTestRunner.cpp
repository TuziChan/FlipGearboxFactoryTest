/**
 * @brief Unified Test Runner
 *
 * Discovers and executes all project test executables,
 * aggregates results, and generates JSON/HTML/JUnit reports.
 *
 * Usage: UnifiedTestRunner [options]
 *   --build-dir <path>     Build directory containing test executables
 *   --output-dir <path>    Directory for generated reports
 *   --filter <pattern>     Only run tests matching pattern
 *   --format <json|html|junit|md>  Report formats to generate
 *   --ctest                Run via ctest instead of direct execution
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include "src/infrastructure/testing/TestOrchestrator.h"

using namespace Infrastructure::Testing;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("UnifiedTestRunner");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Unified test runner for FlipGearboxFactoryTest");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption buildDirOption(QStringList() << "b" << "build-dir",
                                      "Build directory containing test executables",
                                      "path", QDir::currentPath());
    parser.addOption(buildDirOption);

    QCommandLineOption outputDirOption(QStringList() << "o" << "output-dir",
                                       "Directory for generated reports",
                                       "path", QDir::currentPath() + "/test-reports");
    parser.addOption(outputDirOption);

    QCommandLineOption filterOption(QStringList() << "f" << "filter",
                                    "Only run tests matching pattern",
                                    "pattern");
    parser.addOption(filterOption);

    QCommandLineOption formatOption(QStringList() << "format",
                                    "Report formats (comma-separated: json,html,junit,md)",
                                    "formats", "json,html");
    parser.addOption(formatOption);

    QCommandLineOption ctestOption(QStringList() << "ctest",
                                   "Run tests via ctest instead of direct execution");
    parser.addOption(ctestOption);

    QCommandLineOption qtPathOption(QStringList() << "qt-path",
                                    "Qt bin directory for PATH",
                                    "path");
    parser.addOption(qtPathOption);

    parser.process(app);

    QString buildDir = parser.value(buildDirOption);
    QString outputDir = parser.value(outputDirOption);
    QString filter = parser.value(filterOption);
    QString formats = parser.value(formatOption);
    QString qtPath = parser.value(qtPathOption);
    bool useCtest = parser.isSet(ctestOption);

    // Ensure output directory exists
    QDir outDir(outputDir);
    if (!outDir.exists()) {
        outDir.mkpath(".");
    }

    TestOrchestrator orchestrator;
    orchestrator.setBuildDirectory(buildDir);

    if (!qtPath.isEmpty()) {
        orchestrator.setTestPath(qtPath);
    }

    if (useCtest) {
        qInfo() << "Running tests via ctest...";
        bool ctestOk = orchestrator.runCtest(buildDir);
        return ctestOk ? 0 : 1;
    }

    // Discover tests
    qInfo() << "Discovering tests in:" << buildDir;
    orchestrator.discoverTests(buildDir);

    QStringList tests = orchestrator.registeredTests();
    if (tests.isEmpty()) {
        qWarning() << "No test executables found in" << buildDir;
        qWarning() << "Falling back to registered test list...";

        // Fallback: register known test names
        QStringList knownTests = {
            "QmlSmokeTests",
            "BoundaryProtectionTests",
            "HistoryViewModelTests",
            "RecipeViewModelTests",
            "ModbusCrcTests",
            "AutoTestFrameworkTests",
            "PerformanceMonitorTests",
            "TestReportGeneratorTests"
        };

        for (const QString& name : knownTests) {
            QString exePath = QDir(buildDir).absoluteFilePath(
#ifdef Q_OS_WIN
                name + ".exe"
#else
                name
#endif
            );
            if (QFile::exists(exePath)) {
                orchestrator.registerTestExecutable(name, exePath);
            }
        }

        tests = orchestrator.registeredTests();
    }

    if (tests.isEmpty()) {
        qCritical() << "No tests found or registered. Exiting.";
        return 1;
    }

    qInfo() << "Found" << tests.size() << "test executables:";
    for (const QString& name : tests) {
        qInfo() << "  -" << name;
    }

    // Apply filter
    QStringList testsToRun = tests;
    if (!filter.isEmpty()) {
        testsToRun.clear();
        for (const QString& name : tests) {
            if (name.contains(filter, Qt::CaseInsensitive)) {
                testsToRun.append(name);
            }
        }
        qInfo() << "Filter applied:" << filter << "->" << testsToRun.size() << "tests match";
    }

    // Run tests
    AggregatedTestReport report;
    if (testsToRun.size() == tests.size()) {
        report = orchestrator.runAll("FlipGearboxFactoryTest Full Suite");
    } else {
        report = orchestrator.runFiltered(testsToRun, "FlipGearboxFactoryTest Filtered Suite");
    }

    // Generate reports
    QStringList formatList = formats.split(",");
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

    for (const QString& fmt : formatList) {
        QString trimmed = fmt.trimmed().toLower();
        if (trimmed == "json") {
            QString path = outDir.absoluteFilePath(QStringLiteral("test_report_%1.json").arg(timestamp));
            orchestrator.saveJsonReport(report, path);
            qInfo() << "JSON report:" << path;
        } else if (trimmed == "html") {
            QString path = outDir.absoluteFilePath(QStringLiteral("test_report_%1.html").arg(timestamp));
            orchestrator.saveHtmlReport(report, path);
            qInfo() << "HTML report:" << path;
        } else if (trimmed == "junit") {
            QString path = outDir.absoluteFilePath(QStringLiteral("test_report_%1.xml").arg(timestamp));
            orchestrator.saveJUnitXmlReport(report, path);
            qInfo() << "JUnit XML report:" << path;
        } else if (trimmed == "md") {
            QString path = outDir.absoluteFilePath(QStringLiteral("test_report_%1.md").arg(timestamp));
            QFile file(path);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                stream.setEncoding(QStringConverter::Utf8);
                stream << report.toMarkdown();
                file.close();
                qInfo() << "Markdown report:" << path;
            }
        }
    }

    // Print summary
    qInfo() << "\n========================================";
    qInfo() << "TEST SUITE SUMMARY";
    qInfo() << "========================================";
    qInfo() << "Total tests:" << report.totalExecutables;
    qInfo() << "Passed:" << report.passedExecutables;
    qInfo() << "Failed:" << report.failedExecutables;
    qInfo() << "Duration:" << report.totalDurationMs << "ms";
    qInfo() << "========================================";

    for (const auto& r : report.results) {
        if (!r.passed) {
            qWarning() << "FAILED:" << r.name << "-" << r.errorOutput.left(200);
        }
    }

    return report.failedExecutables == 0 ? 0 : 1;
}
