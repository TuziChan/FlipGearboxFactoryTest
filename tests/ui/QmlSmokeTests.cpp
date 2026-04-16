#include <QtTest>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QVariant>

class QmlSmokeTests : public QObject
{
    Q_OBJECT

private slots:
    void loadsMotorShell();
    void switchesToPlaceholderPageFromNav();
    void opensComponentGalleryFromNav();
    void togglesNavRailExpansion();
    void opensCustomSelectPopup();
    void navigatesCustomSelectOptions();
    void switchesThemeAtRuntime();
    void opensDialogOverlay();
    void opensDropdownMenuAndSelectsItem();
    void togglesTooltipVisibility();
    void switchesTabsContent();
    void opensSheetOverlay();
    void opensPopoverPanel();
    void filtersComboboxOptions();
    void opensCommandPalette();
    void togglesCheckboxAndRadioGroupSelection();
    void togglesToggleAndToggleGroupSelection();
    void opensAlertDialogAndConfirms();
    void operatesMenubarAndContextMenu();
    void togglesAccordionAndCollapsible();
    void loadsIdentityAndFormComponents();
    void keepsIdentitySamplesCenteredAndCompact();
    void operatesTableSliderAndPagination();
    void operatesNativeSelectAndInputOtp();
    void operatesNavigationMenuHoverCardAndDrawer();
    void scrollsScrollAreaAndLoadsInputGroup();
    void operatesCalendarCarouselAndResizable();
    void operatesAspectRatioRangeSliderAndEnhancedTable();
};

void QmlSmokeTests::loadsMotorShell()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");
    QCOMPARE(window->title(), QStringLiteral("齿轮箱产线测试系统 v1.0"));
    QVERIFY2(window->findChild<QObject *>(QStringLiteral("commandBar")) != nullptr,
             "Main shell should expose the command bar.");
    QVERIFY2(window->findChild<QObject *>(QStringLiteral("phasePanel")) != nullptr,
             "Main shell should expose the phase list panel.");
    QVERIFY2(window->findChild<QObject *>(QStringLiteral("metricsGrid")) != nullptr,
             "Main shell should expose the metrics area.");
    QVERIFY2(window->findChild<QObject *>(QStringLiteral("verdictPanel")) != nullptr,
             "Main shell should expose the verdict panel.");
}

void QmlSmokeTests::switchesToPlaceholderPageFromNav()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    const bool changed = shell->setProperty("activeNavIndex", 1);
    QVERIFY2(changed, "App shell should expose a writable activeNavIndex property.");

    auto *contentStack = window->findChild<QObject *>(QStringLiteral("contentStack"));
    QVERIFY2(contentStack != nullptr, "Main shell should expose the page stack.");
    QCOMPARE(contentStack->property("currentIndex").toInt(), 1);

    QVERIFY2(window->findChild<QObject *>(QStringLiteral("recipePage")) != nullptr,
             "Selecting the recipe entry should switch to the recipe placeholder page.");
}

void QmlSmokeTests::opensComponentGalleryFromNav()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    const bool changed = shell->setProperty("activeNavIndex", 5);
    QVERIFY2(changed, "App shell should expose a writable activeNavIndex property.");

    auto *contentStack = window->findChild<QObject *>(QStringLiteral("contentStack"));
    QVERIFY2(contentStack != nullptr, "Main shell should expose the page stack.");
    QCOMPARE(contentStack->property("currentIndex").toInt(), 5);

    auto *page = window->findChild<QObject *>(QStringLiteral("testExecutionPage"));
    QVERIFY2(page != nullptr, "Main shell should expose the test execution page.");

    auto *gallery = qobject_cast<QQuickItem *>(window->findChild<QObject *>(QStringLiteral("componentGalleryPage")));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QVERIFY2(page->findChild<QObject *>(QStringLiteral("sampleDialog")) == nullptr,
             "Business page should not own component sample instances once the gallery page exists.");

    auto *introCard = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("galleryIntroCard")));
    QVERIFY2(introCard != nullptr, "Component gallery should expose a visible intro card.");
    QVERIFY2(introCard->height() > 0, "Component gallery intro card should have a positive height.");
}

void QmlSmokeTests::togglesNavRailExpansion()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    auto *navRail = window->findChild<QObject *>(QStringLiteral("navRail"));
    QVERIFY2(navRail != nullptr, "Main shell should expose the nav rail.");
    QCOMPARE(navRail->property("expanded").toBool(), false);

    const bool expanded = navRail->setProperty("expanded", true);
    QVERIFY2(expanded, "Nav rail should expose a writable expanded property.");
    QCOMPARE(navRail->property("expanded").toBool(), true);
}

void QmlSmokeTests::opensCustomSelectPopup()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *select = window->findChild<QObject *>(QStringLiteral("modelSelectControl"));
    QVERIFY2(select != nullptr, "Command bar should expose the model select control.");

    const bool opened = QMetaObject::invokeMethod(select, "openPopup");
    QVERIFY2(opened, "Custom select should expose openPopup().");

    auto *popup = window->findChild<QObject *>(QStringLiteral("modelSelectPopup"));
    QVERIFY2(popup != nullptr, "Custom select should expose its popup object.");
    QCOMPARE(popup->property("visible").toBool(), true);

    const bool selected = QMetaObject::invokeMethod(
        select,
        "selectIndex",
        Q_ARG(QVariant, QVariant(1)));
    QVERIFY2(selected, "Custom select should expose selectIndex(index).");
    QCOMPARE(select->property("currentText").toString(), QStringLiteral("GBX-42B"));
}

void QmlSmokeTests::navigatesCustomSelectOptions()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *select = window->findChild<QObject *>(QStringLiteral("modelSelectControl"));
    QVERIFY2(select != nullptr, "Command bar should expose the model select control.");

    QVERIFY2(QMetaObject::invokeMethod(select, "openPopup"), "Custom select should expose openPopup().");
    QCOMPARE(select->property("highlightedIndex").toInt(), select->property("currentIndex").toInt());

    QVERIFY2(QMetaObject::invokeMethod(select, "moveHighlight", Q_ARG(QVariant, QVariant(1))),
             "Custom select should expose moveHighlight(delta).");
    QCOMPARE(select->property("highlightedIndex").toInt(), 1);

    QVERIFY2(QMetaObject::invokeMethod(select, "confirmHighlighted"),
             "Custom select should expose confirmHighlighted().");
    QCOMPARE(select->property("currentIndex").toInt(), 1);
    QCOMPARE(select->property("currentText").toString(), QStringLiteral("GBX-42B"));

    QVERIFY2(QMetaObject::invokeMethod(select, "openPopup"), "Custom select should reopen after selecting.");
    QVERIFY2(QMetaObject::invokeMethod(select, "closePopup"), "Custom select should expose closePopup().");

    auto *popup = window->findChild<QObject *>(QStringLiteral("modelSelectPopup"));
    QVERIFY2(popup != nullptr, "Custom select should expose its popup object.");
    QCOMPARE(popup->property("visible").toBool(), false);
}

void QmlSmokeTests::switchesThemeAtRuntime()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    auto *theme = shell->findChild<QObject *>(QStringLiteral("appTheme"));
    QVERIFY2(theme != nullptr, "App shell should expose the active theme object.");

    auto *panel = shell->findChild<QObject *>(QStringLiteral("themeSwitcherPanel"));
    QVERIFY2(panel != nullptr, "App shell should expose the theme switcher panel.");

    const bool expanded = QMetaObject::invokeMethod(panel, "toggleCollapsed");
    QVERIFY2(expanded, "Theme switcher should expose toggleCollapsed().");

    const bool themeChanged = QMetaObject::invokeMethod(
        panel,
        "selectThemeName",
        Q_ARG(QVariant, QVariant(QStringLiteral("zinc"))));
    QVERIFY2(themeChanged, "Theme switcher should expose selectThemeName(name).");
    QCOMPARE(theme->property("themeName").toString(), QStringLiteral("zinc"));

    const bool styleChanged = QMetaObject::invokeMethod(
        panel,
        "selectStyleName",
        Q_ARG(QVariant, QVariant(QStringLiteral("luma"))));
    QVERIFY2(styleChanged, "Theme switcher should expose selectStyleName(name).");
    QCOMPARE(theme->property("styleName").toString(), QStringLiteral("luma"));

    const bool darkToggled = QMetaObject::invokeMethod(panel, "toggleDarkMode");
    QVERIFY2(darkToggled, "Theme switcher should expose toggleDarkMode().");
    QCOMPARE(theme->property("darkMode").toBool(), true);
}

void QmlSmokeTests::opensDialogOverlay()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = qobject_cast<QQuickItem *>(window->findChild<QObject *>(QStringLiteral("componentGalleryPage")));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");

    auto *dialog = gallery->findChild<QObject *>(QStringLiteral("sampleDialog"));
    QVERIFY2(dialog != nullptr, "Component gallery should expose the sample dialog.");
    QCOMPARE(dialog->property("open").toBool(), false);

    const bool opened = QMetaObject::invokeMethod(gallery, "openSampleDialog");
    QVERIFY2(opened, "Component gallery should expose openSampleDialog().");
    QCOMPARE(dialog->property("open").toBool(), true);

    auto *overlay = gallery->findChild<QObject *>(QStringLiteral("sampleDialogOverlay"));
    QVERIFY2(overlay != nullptr, "Dialog should expose its overlay.");
    QCOMPARE(overlay->property("visible").toBool(), true);

    const bool closed = QMetaObject::invokeMethod(dialog, "closeDialog");
    QVERIFY2(closed, "Dialog should expose closeDialog().");
    QCOMPARE(dialog->property("open").toBool(), false);
}

void QmlSmokeTests::opensDropdownMenuAndSelectsItem()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSampleDropdownMenu"),
             "Component gallery should expose openSampleDropdownMenu().");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "selectSampleDropdownIndex", Q_ARG(QVariant, QVariant(1))),
             "Component gallery should expose selectSampleDropdownIndex(index).");
    QCOMPARE(gallery->property("sampleDropdownSelectedText").toString(), QStringLiteral("切换到紧凑视图"));
}

void QmlSmokeTests::togglesTooltipVisibility()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "showSampleTooltip"),
             "Component gallery should expose showSampleTooltip().");
    QCOMPARE(gallery->property("sampleTooltipOpen").toBool(), true);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "hideSampleTooltip"),
             "Component gallery should expose hideSampleTooltip().");
    QCOMPARE(gallery->property("sampleTooltipOpen").toBool(), false);
}

void QmlSmokeTests::switchesTabsContent()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QCOMPARE(gallery->property("sampleTabsCurrentIndex").toInt(), 0);
    QCOMPARE(gallery->property("sampleTabsCurrentText").toString(), QStringLiteral("概览页展示总体信息与默认摘要。"));

    QVERIFY2(QMetaObject::invokeMethod(gallery, "setSampleTabsIndex", Q_ARG(QVariant, QVariant(3))),
             "Component gallery should expose setSampleTabsIndex(index).");
    QCOMPARE(gallery->property("sampleTabsCurrentIndex").toInt(), 3);
    QCOMPARE(gallery->property("sampleTabsCurrentText").toString(), QStringLiteral("管理您的账户偏好和选项。根据您的需求定制您的体验。"));
}

void QmlSmokeTests::opensSheetOverlay()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSampleSheet"),
             "Component gallery should expose openSampleSheet().");
    QCOMPARE(gallery->property("sampleSheetOpen").toBool(), true);

    auto *overlay = gallery->findChild<QObject *>(QStringLiteral("sampleSheetOverlay"));
    QVERIFY2(overlay != nullptr, "Sheet should expose its overlay.");
    QCOMPARE(overlay->property("visible").toBool(), true);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "closeSampleSheet"),
             "Component gallery should expose closeSampleSheet().");
    QCOMPARE(gallery->property("sampleSheetOpen").toBool(), false);
}

void QmlSmokeTests::opensPopoverPanel()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSamplePopover"),
             "Component gallery should expose openSamplePopover().");
    QCOMPARE(gallery->property("samplePopoverOpen").toBool(), true);

    auto *panel = gallery->findChild<QObject *>(QStringLiteral("samplePopoverPanel"));
    QVERIFY2(panel != nullptr, "Popover should expose its panel.");
    QCOMPARE(panel->property("visible").toBool(), true);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "closeSamplePopover"),
             "Component gallery should expose closeSamplePopover().");
    QCOMPARE(gallery->property("samplePopoverOpen").toBool(), false);
}

void QmlSmokeTests::filtersComboboxOptions()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSampleCombobox"),
             "Component gallery should expose openSampleCombobox().");
    QVERIFY2(QMetaObject::invokeMethod(gallery, "setSampleComboboxQuery", Q_ARG(QVariant, QVariant(QStringLiteral("负载")))),
             "Component gallery should expose setSampleComboboxQuery(query).");
    QCOMPARE(gallery->property("sampleComboboxFilteredCount").toInt(), 1);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "confirmSampleCombobox"),
             "Component gallery should expose confirmSampleCombobox().");
    QCOMPARE(gallery->property("sampleComboboxCurrentText").toString(), QStringLiteral("负载上升"));
}

void QmlSmokeTests::opensCommandPalette()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");

    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QCOMPARE(gallery->property("sampleCommandOpen").toBool(), false);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSampleCommand"),
             "Component gallery should expose openSampleCommand().");
    QCOMPARE(gallery->property("sampleCommandOpen").toBool(), true);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "setSampleCommandQuery", Q_ARG(QVariant, QVariant(QStringLiteral("紧凑")))),
             "Component gallery should expose setSampleCommandQuery(query).");
    QCOMPARE(gallery->property("sampleCommandFilteredCount").toInt(), 1);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "confirmSampleCommand"),
             "Component gallery should expose confirmSampleCommand().");
    QCOMPARE(gallery->property("sampleCommandSelectedText").toString(), QStringLiteral("切换到紧凑视图"));
}

void QmlSmokeTests::togglesCheckboxAndRadioGroupSelection()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QCOMPARE(gallery->property("sampleCheckboxChecked").toBool(), false);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "toggleSampleCheckbox"),
             "Component gallery should expose toggleSampleCheckbox().");
    QCOMPARE(gallery->property("sampleCheckboxChecked").toBool(), true);

    QVERIFY2(QMetaObject::invokeMethod(
                 gallery,
                 "selectSampleRadioValue",
                 Q_ARG(QVariant, QVariant(QStringLiteral("draft")))),
             "Component gallery should expose selectSampleRadioValue(value).");
    QCOMPARE(gallery->property("sampleRadioValue").toString(), QStringLiteral("draft"));
}

void QmlSmokeTests::togglesToggleAndToggleGroupSelection()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QCOMPARE(gallery->property("sampleToggleChecked").toBool(), false);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "toggleSampleToggle"),
             "Component gallery should expose toggleSampleToggle().");
    QCOMPARE(gallery->property("sampleToggleChecked").toBool(), true);

    QVERIFY2(QMetaObject::invokeMethod(
                 gallery,
                 "selectSampleToggleGroupValue",
                 Q_ARG(QVariant, QVariant(QStringLiteral("list")))),
             "Component gallery should expose selectSampleToggleGroupValue(value).");
    QCOMPARE(gallery->property("sampleToggleGroupValue").toString(), QStringLiteral("list"));
}

void QmlSmokeTests::opensAlertDialogAndConfirms()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QCOMPARE(gallery->property("sampleAlertDialogOpen").toBool(), false);
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAlertTitle")) != nullptr,
             "Component gallery should expose the sample alert title node.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAlertDescription")) != nullptr,
             "Component gallery should expose the sample alert description node.");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSampleAlertDialog"),
             "Component gallery should expose openSampleAlertDialog().");
    QCOMPARE(gallery->property("sampleAlertDialogOpen").toBool(), true);

    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAlertDialogOverlay")) != nullptr,
             "Alert dialog should expose its overlay node.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAlertDialogContent")) != nullptr,
             "Alert dialog should expose its content node.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAlertDialogConfirm")) != nullptr,
             "Alert dialog should expose its confirm button.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAlertDialogCancel")) != nullptr,
             "Alert dialog should expose its cancel button.");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "confirmSampleAlertDialog"),
             "Component gallery should expose confirmSampleAlertDialog().");
    QCOMPARE(gallery->property("sampleAlertDialogOpen").toBool(), false);
}

void QmlSmokeTests::operatesMenubarAndContextMenu()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSampleMenubar", Q_ARG(QVariant, QVariant(0))),
             "Component gallery should expose openSampleMenubar(index).");
    QVERIFY2(QMetaObject::invokeMethod(gallery, "selectSampleMenubarItem", Q_ARG(QVariant, QVariant(1))),
             "Component gallery should expose selectSampleMenubarItem(index).");
    QCOMPARE(gallery->property("sampleMenubarSelectedText").toString(), QStringLiteral("导出报告"));

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSampleContextMenu"),
             "Component gallery should expose openSampleContextMenu().");
    QVERIFY2(QMetaObject::invokeMethod(gallery, "selectSampleContextMenuItem", Q_ARG(QVariant, QVariant(0))),
             "Component gallery should expose selectSampleContextMenuItem(index).");
    QCOMPARE(gallery->property("sampleContextMenuSelectedText").toString(), QStringLiteral("刷新状态"));
}

void QmlSmokeTests::togglesAccordionAndCollapsible()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QCOMPARE(gallery->property("sampleCollapsibleOpen").toBool(), false);
    QCOMPARE(gallery->property("sampleAccordionExpandedCount").toInt(), 0);
    QCOMPARE(gallery->property("sampleAccordionCurrentText").toString(), QStringLiteral(""));

    QVERIFY2(QMetaObject::invokeMethod(gallery, "toggleSampleCollapsible"),
             "Component gallery should expose toggleSampleCollapsible().");
    QCOMPARE(gallery->property("sampleCollapsibleOpen").toBool(), true);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "toggleSampleAccordion", Q_ARG(QVariant, QVariant(0))),
             "Component gallery should expose toggleSampleAccordion(index).");
    QCOMPARE(gallery->property("sampleAccordionExpandedCount").toInt(), 1);
    QCOMPARE(gallery->property("sampleAccordionCurrentText").toString(), QStringLiteral("这里后续会接入更完整的运行时状态摘要。"));
}

void QmlSmokeTests::loadsIdentityAndFormComponents()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAlert")) != nullptr,
             "Component gallery should expose the sample alert.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAvatar")) != nullptr,
             "Component gallery should expose the sample avatar.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAvatarGroup")) != nullptr,
             "Component gallery should expose the sample avatar group.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleBreadcrumb")) != nullptr,
             "Component gallery should expose the sample breadcrumb.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleButtonGroup")) != nullptr,
             "Component gallery should expose the sample button group.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleField")) != nullptr,
             "Component gallery should expose the sample field.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleForm")) != nullptr,
             "Component gallery should expose the sample form.");
}

void QmlSmokeTests::keepsIdentitySamplesCenteredAndCompact()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = qobject_cast<QQuickItem *>(window->findChild<QObject *>(QStringLiteral("componentGalleryPage")));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");

    auto *content = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleIdentityContent")));
    QVERIFY2(content != nullptr,
             "Identity sample section should expose a dedicated compact content container.");
    QVERIFY2(content->width() <= 520,
             "Identity sample container should stay compact instead of stretching across the full card.");
    const qreal centeredOffset = qAbs((content->x() + content->width() / 2.0) - (gallery->width() / 2.0));
    QVERIFY2(centeredOffset <= 4.0,
             "Identity sample container should be horizontally centered within the gallery page.");

    auto *form = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleForm")));
    auto *input = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleFieldInput")));
    QVERIFY2(form != nullptr && input != nullptr,
             "Identity sample section should expose the sample form and its input.");
    QVERIFY2(form->width() <= content->width(),
             "Sample form should remain inside the compact identity container.");
    QVERIFY2(input->width() >= form->width() - 2.0,
             "Sample input should fill the form width instead of staying at a fixed narrow size.");

    auto *primaryAvatar = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleAvatar")));
    auto *secondaryAvatar = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleAvatarSecondary")));
    auto *countAvatar = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleAvatarCount")));
    QVERIFY2(primaryAvatar != nullptr && secondaryAvatar != nullptr && countAvatar != nullptr,
             "Identity sample section should expose all avatar items.");
    QCOMPARE(primaryAvatar->width(), secondaryAvatar->width());
    QCOMPARE(primaryAvatar->height(), secondaryAvatar->height());
    QCOMPARE(primaryAvatar->width(), countAvatar->width());
    QCOMPARE(primaryAvatar->height(), countAvatar->height());
}

void QmlSmokeTests::operatesTableSliderAndPagination()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleTable")) != nullptr,
             "Component gallery should expose the sample table.");
    QCOMPARE(gallery->property("sampleSliderValue").toInt(), 25);
    QCOMPARE(gallery->property("samplePaginationPage").toInt(), 2);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "setSampleSliderValue", Q_ARG(QVariant, QVariant(72))),
             "Component gallery should expose setSampleSliderValue(value).");
    QCOMPARE(gallery->property("sampleSliderValue").toInt(), 72);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "goToSamplePaginationPage", Q_ARG(QVariant, QVariant(5))),
             "Component gallery should expose goToSamplePaginationPage(page).");
    QCOMPARE(gallery->property("samplePaginationPage").toInt(), 5);
}

void QmlSmokeTests::operatesNativeSelectAndInputOtp()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QCOMPARE(gallery->property("sampleNativeSelectValue").toString(), QStringLiteral("station-03"));
    QCOMPARE(gallery->property("sampleInputOtpValue").toString(), QStringLiteral(""));

    QVERIFY2(QMetaObject::invokeMethod(gallery, "selectSampleNativeSelectIndex", Q_ARG(QVariant, QVariant(1))),
             "Component gallery should expose selectSampleNativeSelectIndex(index).");
    QCOMPARE(gallery->property("sampleNativeSelectValue").toString(), QStringLiteral("station-07"));

    QVERIFY2(QMetaObject::invokeMethod(gallery, "setSampleInputOtpValue", Q_ARG(QVariant, QVariant(QStringLiteral("248613")))),
             "Component gallery should expose setSampleInputOtpValue(value).");
    QCOMPARE(gallery->property("sampleInputOtpValue").toString(), QStringLiteral("248613"));
}

void QmlSmokeTests::operatesNavigationMenuHoverCardAndDrawer()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QCOMPARE(gallery->property("sampleHoverCardOpen").toBool(), false);
    QCOMPARE(gallery->property("sampleDrawerOpen").toBool(), false);

    auto *navMenu = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleNavigationMenu")));
    QVERIFY2(navMenu != nullptr, "Component gallery should expose the sample navigation menu.");
    QVERIFY2(navMenu->width() < gallery->property("width").toReal(),
             "Navigation menu sample should render as a compact menu, not stretch to the full gallery width.");

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSampleNavigationMenu", Q_ARG(QVariant, QVariant(0))),
             "Component gallery should expose openSampleNavigationMenu(index).");
    QVERIFY2(QMetaObject::invokeMethod(gallery, "selectSampleNavigationMenuItem", Q_ARG(QVariant, QVariant(1))),
             "Component gallery should expose selectSampleNavigationMenuItem(index).");
    QCOMPARE(gallery->property("sampleNavigationMenuSelectedText").toString(), QStringLiteral("检查器布局"));

    QVERIFY2(QMetaObject::invokeMethod(gallery, "showSampleHoverCard"),
             "Component gallery should expose showSampleHoverCard().");
    QCOMPARE(gallery->property("sampleHoverCardOpen").toBool(), true);
    QVERIFY2(QMetaObject::invokeMethod(gallery, "hideSampleHoverCard"),
             "Component gallery should expose hideSampleHoverCard().");
    QCOMPARE(gallery->property("sampleHoverCardOpen").toBool(), false);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "openSampleDrawer"),
             "Component gallery should expose openSampleDrawer().");
    QCOMPARE(gallery->property("sampleDrawerOpen").toBool(), true);
    QVERIFY2(QMetaObject::invokeMethod(gallery, "closeSampleDrawer"),
             "Component gallery should expose closeSampleDrawer().");
    QCOMPARE(gallery->property("sampleDrawerOpen").toBool(), false);
}

void QmlSmokeTests::scrollsScrollAreaAndLoadsInputGroup()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleInputGroup")) != nullptr,
             "Component gallery should expose the sample input group.");
    QCOMPARE(gallery->property("sampleScrollAtEnd").toBool(), false);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "scrollSampleScrollAreaToEnd"),
             "Component gallery should expose scrollSampleScrollAreaToEnd().");
    QCOMPARE(gallery->property("sampleScrollAtEnd").toBool(), true);
}

void QmlSmokeTests::operatesCalendarCarouselAndResizable()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleCalendar")) != nullptr,
             "Component gallery should expose the sample calendar.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleCarousel")) != nullptr,
             "Component gallery should expose the sample carousel.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleResizable")) != nullptr,
             "Component gallery should expose the sample resizable panels.");

    const QString beforeMonth = gallery->property("sampleCalendarMonthLabel").toString();
    QVERIFY2(QMetaObject::invokeMethod(gallery, "goToNextSampleCalendarMonth"),
             "Component gallery should expose goToNextSampleCalendarMonth().");
    QVERIFY(beforeMonth != gallery->property("sampleCalendarMonthLabel").toString());

    QCOMPARE(gallery->property("sampleCarouselIndex").toInt(), 0);
    QVERIFY2(QMetaObject::invokeMethod(gallery, "goToNextSampleCarousel"),
             "Component gallery should expose goToNextSampleCarousel().");
    QCOMPARE(gallery->property("sampleCarouselIndex").toInt(), 1);

    QVERIFY2(QMetaObject::invokeMethod(gallery, "setSampleResizableRatio", Q_ARG(QVariant, QVariant(65))),
             "Component gallery should expose setSampleResizableRatio(value).");
    QCOMPARE(gallery->property("sampleResizableRatio").toInt(), 65);
}

void QmlSmokeTests::operatesAspectRatioRangeSliderAndEnhancedTable()
{
    QQmlApplicationEngine engine;
    const auto mainQmlPath = QStringLiteral(TEST_SOURCE_DIR "/Main.qml");
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QVERIFY2(!engine.rootObjects().isEmpty(), "Main.qml should create a root object.");
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QVERIFY2(window != nullptr, "Root object should be a QQuickWindow.");

    auto *shell = window->findChild<QObject *>(QStringLiteral("appShell"));
    QVERIFY2(shell != nullptr, "Main window should expose the app shell.");
    QVERIFY2(shell->setProperty("activeNavIndex", 5),
             "App shell should expose a writable activeNavIndex property.");

    auto *gallery = window->findChild<QObject *>(QStringLiteral("componentGalleryPage"));
    QVERIFY2(gallery != nullptr, "Main shell should expose the component gallery page.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleAspectRatio")) != nullptr,
             "Component gallery should expose the sample aspect ratio.");
    QVERIFY2(gallery->findChild<QObject *>(QStringLiteral("sampleEnhancedTable")) != nullptr,
             "Component gallery should expose the enhanced sample table.");

    auto *calendar = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleCalendar")));
    auto *carousel = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleCarousel")));
    auto *aspectRatio = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleAspectRatio")));
    auto *rangeSlider = qobject_cast<QQuickItem *>(gallery->findChild<QObject *>(QStringLiteral("sampleRangeSlider")));
    QVERIFY2(calendar != nullptr && carousel != nullptr && aspectRatio != nullptr && rangeSlider != nullptr,
             "Component gallery should expose the calendar/carousel/aspect-ratio/range-slider samples.");
    const qreal galleryWidth = gallery->property("width").toReal();
    QVERIFY2(calendar->width() < galleryWidth, "Calendar sample should use a compact intrinsic width.");
    QVERIFY2(carousel->width() < galleryWidth, "Carousel sample should not stretch across the full gallery width.");
    QVERIFY2(aspectRatio->width() < galleryWidth, "Aspect-ratio sample should demonstrate ratio in a bounded frame.");
    QVERIFY2(rangeSlider->width() < galleryWidth, "Range slider sample should not consume the full gallery width.");

    QCOMPARE(gallery->property("sampleRangeSliderValuesText").toString(), QStringLiteral("15-85"));
    QVERIFY2(QMetaObject::invokeMethod(
                 gallery,
                 "setSampleRangeSliderValues",
                 Q_ARG(QVariant, QVariant(QVariantList{QVariant(20), QVariant(80)}))),
             "Component gallery should expose setSampleRangeSliderValues(values).");
    QCOMPARE(gallery->property("sampleRangeSliderValuesText").toString(), QStringLiteral("20-80"));

    QCOMPARE(gallery->property("sampleTableFooterText").toString(), QStringLiteral("总计 3 条"));
}

QTEST_MAIN(QmlSmokeTests)

#include "QmlSmokeTests.moc"
