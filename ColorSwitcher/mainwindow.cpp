#include "mainwindow.h"
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
    , m_mode(Mode::Cycle)
    , m_clickHandlerAlt(false)
{
    setupUi();
    m_timer.setInterval(600);
    rebuildTimeoutConnection();
    reconnectCanvasHandler();
    setStatus(QStringLiteral("Ready. Click Start/Stop to run the timer."));
}

MainWindow::~MainWindow()
{
    qDebug() << "[DESTROY] MainWindow";
    m_timer.stop();
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("QTimer + Lambda Slots + Dynamic Connections"));
    resize(560, 640);

    m_rootLayout = new QVBoxLayout(this);

    m_canvas.setParent(this);
    m_rootLayout->addWidget(&m_canvas);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    m_rootLayout->addWidget(m_statusLabel);

    m_intervalLabel = new QLabel(this);
    m_modeLabel = new QLabel(this);
    m_rootLayout->addWidget(m_intervalLabel);
    m_rootLayout->addWidget(m_modeLabel);

    auto* controls = new QHBoxLayout();
    auto* btnStart = new QPushButton(QStringLiteral("Start/Stop"), this);
    auto* btnSlow = new QPushButton(QStringLiteral("Slower"), this);
    auto* btnFast = new QPushButton(QStringLiteral("Faster"), this);
    auto* btnMode = new QPushButton(QStringLiteral("Mode"), this);

    controls->addWidget(btnStart);
    controls->addWidget(btnSlow);
    controls->addWidget(btnFast);
    controls->addWidget(btnMode);
    m_rootLayout->addLayout(controls);

    connect(btnStart, &QPushButton::clicked, this, &MainWindow::toggleTimer);
    connect(btnSlow, &QPushButton::clicked, this, &MainWindow::decreaseInterval);
    connect(btnFast, &QPushButton::clicked, this, &MainWindow::increaseInterval);
    connect(btnMode, &QPushButton::clicked, this, &MainWindow::cycleMode);

    auto* palHeader = new QLabel(QStringLiteral("Palette (dynamic buttons):"), this);
    m_rootLayout->addWidget(palHeader);

    m_paletteGrid = new QGridLayout();
    m_rootLayout->addLayout(m_paletteGrid);

    auto* palControls = new QHBoxLayout();
    auto* btnAdd = new QPushButton(QStringLiteral("+ Color"), this);
    auto* btnClear = new QPushButton(QStringLiteral("Clear Palette"), this);
    auto* btnClick = new QPushButton(QStringLiteral("Switch Click Handler"), this);

    palControls->addWidget(btnAdd);
    palControls->addWidget(btnClear);
    palControls->addWidget(btnClick);
    m_rootLayout->addLayout(palControls);

    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::addColorButton);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::clearPaletteButtons);
    connect(btnClick, &QPushButton::clicked, this, &MainWindow::toggleClickHandler);

    m_rootLayout->addStretch();

    updateIntervalLabel();
    updateModeLabel();
}

void MainWindow::setStatus(const QString& text)
{
    if (m_statusLabel) m_statusLabel->setText(text);
}

void MainWindow::updateIntervalLabel()
{
    if (m_intervalLabel)
        m_intervalLabel->setText(QStringLiteral("Interval: %1 ms").arg(m_timer.interval()));
}

void MainWindow::updateModeLabel()
{
    if (m_modeLabel)
        m_modeLabel->setText(QStringLiteral("Mode: %1").arg(modeName()));
}

QString MainWindow::modeName() const
{
    switch (m_mode) {
    case Mode::Cycle: return QStringLiteral("Cycle");
    case Mode::Random: return QStringLiteral("Random");
    case Mode::Blink: return QStringLiteral("Blink");
    }
    return {};
}

void MainWindow::toggleTimer()
{
    if (m_timer.isActive()) {
        m_timer.stop();
        setStatus(QStringLiteral("Timer stopped."));
        qDebug() << "[TIMER] stop();";
    }
    else {
        m_timer.start();
        setStatus(QStringLiteral("Timer started (interval %1 ms).").arg(m_timer.interval()));
        qDebug() << "[TIMER] start(), interval =" << m_timer.interval();
    }
}

void MainWindow::decreaseInterval()
{
    m_timer.setInterval(qMin(m_timer.interval() + 200, 5000));
    updateIntervalLabel();
    qDebug() << "[TIMER] interval ->" << m_timer.interval();
}

void MainWindow::increaseInterval()
{
    m_timer.setInterval(qMax(m_timer.interval() - 200, 50));
    updateIntervalLabel();
    qDebug() << "[TIMER] interval ->" << m_timer.interval();
}

void MainWindow::cycleMode()
{
    switch (m_mode) {
    case Mode::Cycle: m_mode = Mode::Random; break;
    case Mode::Random: m_mode = Mode::Blink; break;
    case Mode::Blink: m_mode = Mode::Cycle; break;
    }
    updateModeLabel();
    rebuildTimeoutConnection();
    setStatus(QStringLiteral("Mode switched: %1").arg(modeName()));
    qDebug() << "[TIMER] mode ->" << modeName() << "(reconnect timeout)";
}

void MainWindow::rebuildTimeoutConnection()
{
    if (m_timeoutConn) {
        disconnect(m_timeoutConn);
        m_timeoutConn = QMetaObject::Connection();
    }

    m_timeoutConn = connect(&m_timer, &QTimer::timeout, this, [this]() {
        switch (m_mode) {
        case Mode::Cycle: m_canvas.stepForward(); break;
        case Mode::Random: m_canvas.setRandomColor(); break;
        case Mode::Blink: m_canvas.toggleBlink(); break;
        }
        qDebug() << "[TIMER] timeout, mode =" << modeName() << ", color =" << m_canvas.currentColor().name();
        });
}

void MainWindow::reconnectCanvasHandler()
{
    if (m_clickConn) {
        disconnect(m_clickConn);
        m_clickConn = QMetaObject::Connection();
    }

    if (!m_clickHandlerAlt) {
        m_clickConn = connect(&m_canvas, &ColorWidget::colorChanged, this, [this](const QColor& c) {
            setStatus(QStringLiteral("Canvas color: %1").arg(c.name()));
            });
    }
    else {
        m_clickConn = connect(&m_canvas, &ColorWidget::colorChanged, this, [this](const QColor& c) {
            setWindowTitle(QStringLiteral("ColorSwitcher - %1").arg(c.name()));
            });
    }
}

void MainWindow::toggleClickHandler()
{
    m_clickHandlerAlt = !m_clickHandlerAlt;
    reconnectCanvasHandler();
    setStatus(m_clickHandlerAlt
        ? QStringLiteral("Click handler: Variant B (window title).")
        : QStringLiteral("Click handler: Variant A (status bar)."));
    qDebug() << "[CANVAS] reconnect colorChanged, alt =" << m_clickHandlerAlt;
}

void MainWindow::addColorButton()
{
    QColor color = QColor::fromHsv(
        QRandomGenerator::global()->bounded(360),
        200 + QRandomGenerator::global()->bounded(55),
        200 + QRandomGenerator::global()->bounded(55));

    auto* btn = new PaletteButton(color, this);
    const int row = m_paletteButtons.size() / 4;
    const int col = m_paletteButtons.size() % 4;
    m_paletteGrid->addWidget(btn, row, col);

    QMetaObject::Connection conn = connect(btn, &QPushButton::clicked, this, [this, btn]() {
        m_canvas.addColor(btn->color());
        setStatus(QStringLiteral("Added color %1 to canvas palette.").arg(btn->color().name()));
        qDebug() << "[DYNAMIC] clicked color button" << btn->color().name();
        });

    m_paletteButtons.append(QPointer<PaletteButton>(btn));
    m_dynamicConnections.append(conn);
    setStatus(QStringLiteral("Created color button %1 (dynamic connection).").arg(color.name()));
    qDebug() << "[DYNAMIC] connection created for" << color.name() << ", total dynamic:" << m_dynamicConnections.size();
}

void MainWindow::clearPaletteButtons()
{
    for (QMetaObject::Connection& c : m_dynamicConnections) {
        if (c) disconnect(c);
    }
    m_dynamicConnections.clear();

    for (QPointer<PaletteButton>& btn : m_paletteButtons) {
        if (!btn.isNull()) btn->deleteLater();
    }
    m_paletteButtons.clear();

    m_canvas.resetPalette();
    setStatus(QStringLiteral("Palette cleared: connections disconnected, buttons deleted."));
    qDebug() << "[DYNAMIC] palette cleared, connections disconnected";
}