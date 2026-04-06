#pragma once
#include "platform.h"
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#if defined(BRAINSIM_MOBILE)
#include <QScrollArea>
#include <QToolBar>
#endif

#include "glwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onStatsUpdated(int neurons, int synapses, int spikes,
                        float firingRate, float speed, float energy,
                        int foodEaten, float distance, const QString& behavior,
                        float dopamine, float serotonin, float octopamine);

#if defined(BRAINSIM_MOBILE)
    void toggleControlPanel();
#endif

private:
    void setupUI();
    QWidget* createControlPanel();
    QGroupBox* createBrainStatsGroup();
    QGroupBox* createCreatureStatsGroup();
    QGroupBox* createControlsGroup();

    GLWidget* m_glWidget;

    // Brain stats
    QLabel* m_lblNeurons;
    QLabel* m_lblSynapses;
    QLabel* m_lblSpikes;
    QLabel* m_lblFiringRate;
    QLabel* m_lblDopamine;
    QLabel* m_lblSerotonin;
    QLabel* m_lblOctopamine;

    // Creature stats
    QLabel* m_lblSpeed;
    QLabel* m_lblEnergy;
    QLabel* m_lblFoodEaten;
    QLabel* m_lblDistance;
    QLabel* m_lblBehavior;

    // Controls
    QPushButton* m_btnStartStop;
    QPushButton* m_btnReset;
    QPushButton* m_btnStep;
    QSlider* m_sldSpeed;
    QLabel* m_lblSimSpeed;
    QCheckBox* m_chkShowBrain;
    QComboBox* m_cmbCamera;

#if defined(BRAINSIM_MOBILE)
    QWidget* m_controlPanel;
    bool m_controlsVisible = false;
#endif
};
