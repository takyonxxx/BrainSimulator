#include "platform.h"
#include "mainwindow.h"
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("BrainSim - C. elegans Connectome Simulation");

#if defined(BRAINSIM_DESKTOP)
    resize(1400, 900);
#endif

    setupUI();
}

void MainWindow::setupUI()
{
    auto* centralWidget = new QWidget(this);

#if defined(BRAINSIM_MOBILE)
    // ---- Mobile layout: GL fullscreen + overlay toolbar + slide-out panel ----
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Toolbar at top
    auto* toolbar = new QToolBar("Kontrol", this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(32, 32));

    m_btnStartStop = new QPushButton("Baslat");
    m_btnStartStop->setStyleSheet("QPushButton { background: #2a7a2a; color: white; "
                                   "padding: 10px 20px; font-weight: bold; border-radius: 6px; "
                                   "min-height: 44px; font-size: 16px; }");
    toolbar->addWidget(m_btnStartStop);

    m_btnReset = new QPushButton("Sifirla");
    m_btnReset->setStyleSheet("min-height: 44px; font-size: 16px;");
    toolbar->addWidget(m_btnReset);

    auto* btnPanel = new QPushButton("Panel");
    btnPanel->setStyleSheet("min-height: 44px; font-size: 16px;");
    connect(btnPanel, &QPushButton::clicked, this, &MainWindow::toggleControlPanel);
    toolbar->addWidget(btnPanel);

    // Behavior label in toolbar
    m_lblBehavior = new QLabel("--");
    m_lblBehavior->setStyleSheet("font-weight: bold; font-size: 18px; padding: 8px; color: #4caf50;");
    toolbar->addWidget(m_lblBehavior);

    addToolBar(Qt::TopToolBarArea, toolbar);

    // GL widget fills the screen
    m_glWidget = new GLWidget(this);
    mainLayout->addWidget(m_glWidget, 1);

    // Slide-out control panel (initially hidden)
    m_controlPanel = createControlPanel();
    m_controlPanel->setFixedWidth(BRAINSIM_CONTROL_PANEL_WIDTH);
    m_controlPanel->setParent(this);
    m_controlPanel->setVisible(false);
    m_controlPanel->setStyleSheet("background: rgba(30,30,46,0.92); border-left: 1px solid #45475a;");
    m_controlsVisible = false;

#else
    // ---- Desktop layout: GL + side panel ----
    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    m_glWidget = new GLWidget(this);
    m_glWidget->setMinimumSize(800, 600);

    auto* controlPanel = createControlPanel();
    controlPanel->setFixedWidth(BRAINSIM_CONTROL_PANEL_WIDTH);

    mainLayout->addWidget(m_glWidget, 1);
    mainLayout->addWidget(controlPanel);

    // Status bar
    statusBar()->showMessage(
        "SPACE: Baslat/Durdur | R: Sifirla | B: Beyin goster/gizle | "
        "1-4: Kamera | S: Tek adim | +/-: Hiz | Fare: Donme/Yakinlasma");
#endif

    setCentralWidget(centralWidget);

    // Connect stats signal
    connect(m_glWidget, &GLWidget::statsUpdated,
            this, &MainWindow::onStatsUpdated);

    // Connect start/stop button
    connect(m_btnStartStop, &QPushButton::clicked, this, [this]() {
        m_glWidget->toggleSimulation();
        m_btnStartStop->setText(m_glWidget->isRunning() ? "Durdur" : "Baslat");
        m_btnStartStop->setStyleSheet(
            m_glWidget->isRunning()
            ? "QPushButton { background: #7a2a2a; color: white; padding: 8px; "
              "font-weight: bold; border-radius: 4px; min-height: "
              + QString::number(BRAINSIM_MIN_BUTTON_HEIGHT) + "px; }"
            : "QPushButton { background: #2a7a2a; color: white; padding: 8px; "
              "font-weight: bold; border-radius: 4px; min-height: "
              + QString::number(BRAINSIM_MIN_BUTTON_HEIGHT) + "px; }");
    });

    connect(m_btnReset, &QPushButton::clicked, m_glWidget, &GLWidget::resetSimulation);
}

#if defined(BRAINSIM_MOBILE)
void MainWindow::toggleControlPanel()
{
    m_controlsVisible = !m_controlsVisible;
    m_controlPanel->setVisible(m_controlsVisible);

    if (m_controlsVisible) {
        // Position on right side of screen
        int panelW = BRAINSIM_CONTROL_PANEL_WIDTH;
        m_controlPanel->setGeometry(width() - panelW, 0, panelW, height());
        m_controlPanel->raise();
    }
}
#endif

QWidget* MainWindow::createControlPanel()
{
    auto* panel = new QWidget(this);
    auto* outerLayout = new QVBoxLayout(panel);

#if defined(BRAINSIM_MOBILE)
    // Scrollable on mobile
    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    auto* scrollContent = new QWidget();
    auto* layout = new QVBoxLayout(scrollContent);
    layout->setSpacing(12);
#else
    auto* layout = new QVBoxLayout();
    outerLayout->addLayout(layout);
    layout->setSpacing(8);
#endif

    // Title
    auto* title = new QLabel("<h2>BrainSim</h2>"
                             "<p style='color:#888;'>C. elegans Connectome<br>"
                             "302 Noron | LIF Model</p>");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    layout->addWidget(createControlsGroup());
    layout->addWidget(createBrainStatsGroup());
    layout->addWidget(createCreatureStatsGroup());
    layout->addStretch();

    // Info
    auto* info = new QLabel(
        "<small>"
        "<b>Model:</b> Leaky Integrate-and-Fire<br>"
        "<b>Veri:</b> C. elegans konnektom<br>"
        "(White et al. 1986, Cook et al. 2019)<br>"
        "<b>Devreler:</b> Kemotaksi, hareket,<br>"
        "dokunma, beslenme, kacma, CPG<br>"
        "<b>Noromodulatorler:</b> DA, 5-HT, OA"
        "</small>");
    info->setWordWrap(true);
    info->setStyleSheet("color: #aaa; padding: 8px;");
    layout->addWidget(info);

#if defined(BRAINSIM_MOBILE)
    scrollArea->setWidget(scrollContent);
    outerLayout->addWidget(scrollArea);
#endif

    return panel;
}

QGroupBox* MainWindow::createControlsGroup()
{
    auto* group = new QGroupBox("Kontrol");
    auto* layout = new QVBoxLayout(group);

#if defined(BRAINSIM_DESKTOP)
    // Desktop: buttons inside panel
    auto* btnLayout = new QHBoxLayout();
    m_btnStartStop = new QPushButton("Baslat");
    m_btnStartStop->setStyleSheet("QPushButton { background: #2a7a2a; color: white; "
                                   "padding: 8px; font-weight: bold; border-radius: 4px; }");
    m_btnReset = new QPushButton("Sifirla");
    m_btnStep = new QPushButton("Adim");
    connect(m_btnStep, &QPushButton::clicked, m_glWidget, &GLWidget::stepOnce);

    btnLayout->addWidget(m_btnStartStop);
    btnLayout->addWidget(m_btnReset);
    btnLayout->addWidget(m_btnStep);
    layout->addLayout(btnLayout);
#else
    // Mobile: Start/Stop and Reset are in toolbar, add Step here
    m_btnStep = new QPushButton("Tek Adim");
    m_btnStep->setStyleSheet("min-height: 44px;");
    connect(m_btnStep, &QPushButton::clicked, m_glWidget, &GLWidget::stepOnce);
    layout->addWidget(m_btnStep);
#endif

    // Speed slider
    auto* speedLayout = new QHBoxLayout();
    speedLayout->addWidget(new QLabel("Hiz:"));
    m_sldSpeed = new QSlider(Qt::Horizontal);
    m_sldSpeed->setRange(1, 100);
    m_sldSpeed->setValue(10);
    m_lblSimSpeed = new QLabel("1.0x");
    connect(m_sldSpeed, &QSlider::valueChanged, this, [this](int val) {
        float speed = val / 10.0f;
        m_glWidget->setSimulationSpeed(speed);
        m_lblSimSpeed->setText(QString("%1x").arg(speed, 0, 'f', 1));
    });
    speedLayout->addWidget(m_sldSpeed);
    speedLayout->addWidget(m_lblSimSpeed);
    layout->addLayout(speedLayout);

    // Show brain checkbox
    m_chkShowBrain = new QCheckBox("Beyin Gorselini Goster");
    m_chkShowBrain->setChecked(true);
    connect(m_chkShowBrain, &QCheckBox::toggled, m_glWidget, &GLWidget::setShowBrain);
    layout->addWidget(m_chkShowBrain);

    // Camera mode
    auto* camLayout = new QHBoxLayout();
    camLayout->addWidget(new QLabel("Kamera:"));
    m_cmbCamera = new QComboBox();
    m_cmbCamera->addItems({"Takip", "Serbest", "Kusbakisi", "Beyin Gorunumu"});
    connect(m_cmbCamera, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        m_glWidget->setCameraMode(static_cast<GLWidget::CameraMode>(idx));
    });
    camLayout->addWidget(m_cmbCamera);
    layout->addLayout(camLayout);

    return group;
}

QGroupBox* MainWindow::createBrainStatsGroup()
{
    auto* group = new QGroupBox("Beyin Durumu");
    auto* layout = new QVBoxLayout(group);

    auto addStat = [&](const QString& label, QLabel*& valueLabel) {
        auto* row = new QHBoxLayout();
        row->addWidget(new QLabel(label));
        valueLabel = new QLabel("--");
        valueLabel->setAlignment(Qt::AlignRight);
        valueLabel->setStyleSheet("font-weight: bold; color: #4fc3f7;");
        row->addWidget(valueLabel);
        layout->addLayout(row);
    };

    addStat("Noronlar:", m_lblNeurons);
    addStat("Sinapslar:", m_lblSynapses);
    addStat("Toplam Spike:", m_lblSpikes);
    addStat("Ort. Ates Hizi:", m_lblFiringRate);

    // Neuromodulators
    auto* modGroup = new QGroupBox("Noromodulatorler");
    auto* modLayout = new QVBoxLayout(modGroup);

    auto addModStat = [&](const QString& label, QLabel*& valueLabel) {
        auto* row = new QHBoxLayout();
        row->addWidget(new QLabel(label));
        valueLabel = new QLabel("--");
        valueLabel->setAlignment(Qt::AlignRight);
        valueLabel->setStyleSheet("font-weight: bold;");
        row->addWidget(valueLabel);
        modLayout->addLayout(row);
    };

    addModStat("Dopamin:", m_lblDopamine);
    addModStat("Serotonin:", m_lblSerotonin);
    addModStat("Oktopamin:", m_lblOctopamine);
    layout->addWidget(modGroup);

    return group;
}

QGroupBox* MainWindow::createCreatureStatsGroup()
{
    auto* group = new QGroupBox("Canli Durumu");
    auto* layout = new QVBoxLayout(group);

    auto addStat = [&](const QString& label, QLabel*& valueLabel) {
        auto* row = new QHBoxLayout();
        row->addWidget(new QLabel(label));
        valueLabel = new QLabel("--");
        valueLabel->setAlignment(Qt::AlignRight);
        valueLabel->setStyleSheet("font-weight: bold; color: #81c784;");
        row->addWidget(valueLabel);
        layout->addLayout(row);
    };

    addStat("Hiz:", m_lblSpeed);
    addStat("Enerji:", m_lblEnergy);
    addStat("Yenen Yiyecek:", m_lblFoodEaten);
    addStat("Mesafe:", m_lblDistance);

#if defined(BRAINSIM_DESKTOP)
    // Behavior label in panel on desktop
    addStat("Davranis:", m_lblBehavior);
#endif
    // On mobile, m_lblBehavior is in the toolbar (created in setupUI)

    return group;
}

void MainWindow::onStatsUpdated(int neurons, int synapses, int spikes,
                                float firingRate, float speed, float energy,
                                int foodEaten, float distance, const QString& behavior,
                                float dopamine, float serotonin, float octopamine)
{
    m_lblNeurons->setText(QString::number(neurons));
    m_lblSynapses->setText(QString::number(synapses));
    m_lblSpikes->setText(QString::number(spikes));
    m_lblFiringRate->setText(QString::number(firingRate, 'f', 4));

    m_lblDopamine->setText(QString::number(dopamine, 'f', 3));
    m_lblDopamine->setStyleSheet(QString("font-weight: bold; color: rgb(%1, %2, 220);")
                                  .arg((int)(50 + dopamine * 150))
                                  .arg((int)(180 * dopamine)));

    m_lblSerotonin->setText(QString::number(serotonin, 'f', 3));
    m_lblSerotonin->setStyleSheet(QString("font-weight: bold; color: rgb(220, %1, %2);")
                                   .arg((int)(180 * serotonin))
                                   .arg((int)(50 + serotonin * 50)));

    m_lblOctopamine->setText(QString::number(octopamine, 'f', 3));
    m_lblOctopamine->setStyleSheet(QString("font-weight: bold; color: rgb(220, %1, %2);")
                                    .arg((int)(80 * octopamine))
                                    .arg((int)(80 * octopamine)));

    m_lblSpeed->setText(QString::number(speed, 'f', 2));
    m_lblEnergy->setText(QString::number(energy * 100, 'f', 1) + "%");
    m_lblFoodEaten->setText(QString::number(foodEaten));
    m_lblDistance->setText(QString::number(distance, 'f', 1) + " m");
    m_lblBehavior->setText(behavior);

    // Color behavior label
    QString behaviorColor = "#aaa";
    if (behavior == "Kesfediyor") behaviorColor = "#4caf50";
    else if (behavior == "Besleniyor") behaviorColor = "#2196f3";
    else if (behavior == "Kaciyor") behaviorColor = "#f44336";
    else if (behavior == "Donuyor") behaviorColor = "#ff9800";
    m_lblBehavior->setStyleSheet(QString("font-weight: bold; color: %1; "
                                         "font-size: %2px;")
                                  .arg(behaviorColor)
                                  .arg(BRAINSIM_DEFAULT_FONT_SIZE + 2));
}
