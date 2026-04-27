#include <QtTest>
#include "AutoTestFramework.h"

using namespace Tests::Framework;

/**
 * @brief Tests for the AutoTestFramework infrastructure
 */
class AutoTestFrameworkTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testTestClassResult();
    void testTestSuiteReportJson();
    void testTestSuiteReportHtml();
    void testAutoTestBase();
    void testAutoTestRunner();
    void testPerformanceMetrics();
    void testReportGeneration();
};

void AutoTestFrameworkTests::initTestCase()
{
    qDebug() << "========================================";
    qDebug() << "AutoTestFramework Tests";
    qDebug() << "========================================";
}

void AutoTestFrameworkTests::cleanupTestCase()
{
    qDebug() << "AutoTestFramework Tests Complete";
}

void AutoTestFrameworkTests::testTestClassResult()
{
    TestClassResult result;
    result.className = "MyTestClass";

    TestFunctionResult func1;
    func1.name = "testCase1";
    func1.passed = true;
    func1.durationMs = 10;
    result.functionResults.append(func1);
    result.totalFunctions++;
    result.passedFunctions++;

    TestFunctionResult func2;
    func2.name = "testCase2";
    func2.passed = false;
    func2.durationMs = 5;
    func2.errorMessage = "Assertion failed";
    result.functionResults.append(func2);
    result.totalFunctions++;
    result.failedFunctions++;

    result.passed = (result.failedFunctions == 0);

    QCOMPARE(result.className, QString("MyTestClass"));
    QCOMPARE(result.totalFunctions, 2);
    QCOMPARE(result.passedFunctions, 1);
    QCOMPARE(result.failedFunctions, 1);
    QVERIFY(!result.passed);
}

void AutoTestFrameworkTests::testTestSuiteReportJson()
{
    TestSuiteReport report;
    report.suiteName = "Unit Tests";
    report.startTime = QDateTime::currentDateTime();
    report.totalClasses = 2;
    report.passedClasses = 1;
    report.failedClasses = 1;
    report.totalFunctions = 5;
    report.passedFunctions = 4;
    report.failedFunctions = 1;

    QJsonObject json = report.toJson();
    QCOMPARE(json["suite_name"].toString(), QString("Unit Tests"));
    QJsonObject summary = json["summary"].toObject();
    QCOMPARE(summary["total_classes"].toInt(), 2);
    QCOMPARE(summary["passed_functions"].toInt(), 4);
    QVERIFY(json.contains("classes"));
}

void AutoTestFrameworkTests::testTestSuiteReportHtml()
{
    TestSuiteReport report;
    report.suiteName = "Integration Tests";
    report.startTime = QDateTime::currentDateTime();
    report.endTime = QDateTime::currentDateTime().addSecs(10);
    report.totalDurationMs = 10000;
    report.totalClasses = 1;
    report.passedClasses = 1;
    report.totalFunctions = 3;
    report.passedFunctions = 3;

    TestClassResult classResult;
    classResult.className = "ProtocolTests";
    classResult.passed = true;
    classResult.totalFunctions = 3;
    classResult.passedFunctions = 3;

    TestFunctionResult func;
    func.name = "testCrcCalculation";
    func.passed = true;
    func.durationMs = 15;
    classResult.functionResults.append(func);

    report.classResults.append(classResult);

    QString html = report.toHtml();
    QVERIFY(html.contains("Integration Tests"));
    QVERIFY(html.contains("ProtocolTests"));
    QVERIFY(html.contains("testCrcCalculation"));
    QVERIFY(html.contains("PASS"));
}

void AutoTestFrameworkTests::testAutoTestBase()
{
    AutoTestBase base;
    base.setTestMetadata("unit", "Test metadata functionality");

    // recordTestResult is protected - test via subclass or public API
    // Since we can't easily test protected methods, verify the object works
    TestClassResult result = base.testResult();
    QCOMPARE(result.className, QString("Tests::Framework::AutoTestBase"));
}

void AutoTestFrameworkTests::testAutoTestRunner()
{
    AutoTestRunner runner;

    TestSuiteReport report = runner.runAll("Empty Suite");
    QCOMPARE(report.suiteName, QString("Empty Suite"));
    QCOMPARE(report.totalClasses, 0);
    QVERIFY(report.totalDurationMs >= 0);
}

void AutoTestFrameworkTests::testPerformanceMetrics()
{
    auto measurement = PerformanceMetrics::measure("test_op", []() {
        QTest::qSleep(20);
    });

    QCOMPARE(measurement.name, QString("test_op"));
    QVERIFY(measurement.durationMs >= 20);
}

void AutoTestFrameworkTests::testReportGeneration()
{
    TestSuiteReport report;
    report.suiteName = "Report Test";
    report.startTime = QDateTime::currentDateTime();
    report.totalClasses = 1;
    report.passedClasses = 1;

    AutoTestRunner runner;
    QString tempJson = QDir::temp().filePath("test_report.json");
    QString tempHtml = QDir::temp().filePath("test_report.html");

    bool jsonOk = runner.saveJsonReport(report, tempJson);
    bool htmlOk = runner.saveHtmlReport(report, tempHtml);

    QVERIFY(jsonOk);
    QVERIFY(htmlOk);
    QVERIFY(QFile::exists(tempJson));
    QVERIFY(QFile::exists(tempHtml));

    // Cleanup
    QFile::remove(tempJson);
    QFile::remove(tempHtml);
}

QTEST_MAIN(AutoTestFrameworkTests)
#include "AutoTestFrameworkTests.moc"
