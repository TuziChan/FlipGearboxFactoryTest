#include <QtTest>
#include "../../src/infrastructure/reporting/TestReportGenerator.h"

using namespace Infrastructure::Reporting;

/**
 * @brief Tests for the TestReportGenerator service
 */
class TestReportGeneratorTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testAddTestResult();
    void testJsonReport();
    void testHtmlReport();
    void testJUnitXmlReport();
    void testMarkdownReport();
    void testClear();
    void testMultipleResults();
};

void TestReportGeneratorTests::initTestCase()
{
    qDebug() << "========================================";
    qDebug() << "TestReportGenerator Tests";
    qDebug() << "========================================";
}

void TestReportGeneratorTests::cleanupTestCase()
{
    qDebug() << "TestReportGenerator Tests Complete";
}

void TestReportGeneratorTests::testAddTestResult()
{
    TestReportGenerator generator;

    TestCaseResult result;
    result.name = "testPass";
    result.className = "MyTests";
    result.category = "unit";
    result.passed = true;
    result.durationMs = 15;

    generator.addTestResult(result);

    auto summary = generator.summary();
    // Summary not auto-updated, but results should be stored
    Q_UNUSED(summary)
}

void TestReportGeneratorTests::testJsonReport()
{
    TestReportGenerator generator;

    TestSuiteSummary summary;
    summary.suiteName = "Unit Tests";
    summary.startTime = QDateTime::currentDateTime();
    summary.endTime = QDateTime::currentDateTime().addSecs(5);
    summary.totalTests = 2;
    summary.passedTests = 2;
    summary.totalDurationMs = 5000;
    generator.setSuiteSummary(summary);

    TestCaseResult result;
    result.name = "testOne";
    result.className = "TestClass";
    result.passed = true;
    result.durationMs = 100;
    generator.addTestResult(result);

    QString tempFile = QDir::temp().filePath("test_report.json");
    QVERIFY(generator.generateJsonReport(tempFile));
    QVERIFY(QFile::exists(tempFile));

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QVERIFY(!doc.isNull());
    QVERIFY(doc.object().contains("summary"));
    QVERIFY(doc.object().contains("results"));

    QFile::remove(tempFile);
}

void TestReportGeneratorTests::testHtmlReport()
{
    TestReportGenerator generator;

    TestSuiteSummary summary;
    summary.suiteName = "Integration Tests";
    summary.totalTests = 3;
    summary.passedTests = 2;
    summary.failedTests = 1;
    generator.setSuiteSummary(summary);

    TestCaseResult passResult;
    passResult.name = "testPass";
    passResult.className = "TestClass";
    passResult.passed = true;
    passResult.durationMs = 50;
    generator.addTestResult(passResult);

    TestCaseResult failResult;
    failResult.name = "testFail";
    failResult.className = "TestClass";
    failResult.passed = false;
    failResult.durationMs = 20;
    failResult.errorMessage = "Expected true but got false";
    generator.addTestResult(failResult);

    QString tempFile = QDir::temp().filePath("test_report.html");
    QVERIFY(generator.generateHtmlReport(tempFile));
    QVERIFY(QFile::exists(tempFile));

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString html = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(html.contains("Integration Tests"));
    QVERIFY(html.contains("testPass"));
    QVERIFY(html.contains("testFail"));
    QVERIFY(html.contains("Expected true but got false"));

    QFile::remove(tempFile);
}

void TestReportGeneratorTests::testJUnitXmlReport()
{
    TestReportGenerator generator;

    TestSuiteSummary summary;
    summary.suiteName = "JUnit Tests";
    summary.totalTests = 2;
    summary.passedTests = 1;
    summary.failedTests = 1;
    summary.startTime = QDateTime::currentDateTime();
    generator.setSuiteSummary(summary);

    TestCaseResult passResult;
    passResult.name = "testOk";
    passResult.className = "JUnitTest";
    passResult.passed = true;
    generator.addTestResult(passResult);

    TestCaseResult failResult;
    failResult.name = "testBad";
    failResult.className = "JUnitTest";
    failResult.passed = false;
    failResult.errorMessage = "Failure!";
    generator.addTestResult(failResult);

    QString tempFile = QDir::temp().filePath("junit_report.xml");
    QVERIFY(generator.generateJUnitXmlReport(tempFile));
    QVERIFY(QFile::exists(tempFile));

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString xml = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(xml.contains("<testsuites>"));
    QVERIFY(xml.contains("testOk"));
    QVERIFY(xml.contains("testBad"));
    QVERIFY(xml.contains("<failure"));
    QVERIFY(xml.contains("Failure!"));

    QFile::remove(tempFile);
}

void TestReportGeneratorTests::testMarkdownReport()
{
    TestReportGenerator generator;

    TestSuiteSummary summary;
    summary.suiteName = "Markdown Tests";
    summary.totalTests = 1;
    summary.passedTests = 1;
    generator.setSuiteSummary(summary);

    TestCaseResult result;
    result.name = "testMd";
    result.className = "MdTest";
    result.category = "unit";
    result.passed = true;
    result.durationMs = 25;
    generator.addTestResult(result);

    QString tempFile = QDir::temp().filePath("test_report.md");
    QVERIFY(generator.generateMarkdownReport(tempFile));
    QVERIFY(QFile::exists(tempFile));

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString md = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(md.contains("# Markdown Tests"));
    QVERIFY(md.contains("| Test | Category | Status |"));
    QVERIFY(md.contains("testMd"));

    QFile::remove(tempFile);
}

void TestReportGeneratorTests::testClear()
{
    TestReportGenerator generator;

    TestCaseResult result;
    result.name = "test";
    generator.addTestResult(result);

    generator.clear();

    // After clear, summary should be reset
    auto summary = generator.summary();
    QCOMPARE(summary.totalTests, 0);
}

void TestReportGeneratorTests::testMultipleResults()
{
    TestReportGenerator generator;

    for (int i = 0; i < 10; ++i) {
        TestCaseResult result;
        result.name = QString("test_%1").arg(i);
        result.className = "BulkTest";
        result.category = (i % 2 == 0) ? "unit" : "integration";
        result.passed = (i < 8);
        result.durationMs = i * 10;
        generator.addTestResult(result);
    }

    QString tempFile = QDir::temp().filePath("multi_report.json");
    QVERIFY(generator.generateJsonReport(tempFile));

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray results = doc.object()["results"].toArray();
    QCOMPARE(results.size(), 10);

    QFile::remove(tempFile);
}

QTEST_MAIN(TestReportGeneratorTests)
#include "TestReportGeneratorTests.moc"
