#pragma once
#include "platform.h"

#include <QOpenGLWidget>
#include <QTimer>
#include <QElapsedTimer>

#if defined(BRAINSIM_MOUSE_INPUT)
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#endif

#if defined(BRAINSIM_TOUCH_INPUT)
#include <QTouchEvent>
#include <QGestureEvent>
#include <QPinchGesture>
#endif

#if defined(BRAINSIM_GLES)
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#else
#include <QOpenGLFunctions_3_3_Core>
#endif

#include "creature/creature.h"
#include "world/world.h"
#include "renderer/creature_renderer.h"
#include "renderer/world_renderer.h"
#include "renderer/brain_renderer.h"

class GLWidget : public QOpenGLWidget,
#if defined(BRAINSIM_GLES)
                 protected QOpenGLExtraFunctions
#else
                 protected QOpenGLFunctions_3_3_Core
#endif
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget* parent = nullptr);
    ~GLWidget();

    Creature& creature() { return m_creature; }
    World& world() { return m_world; }
    Brain& brain() { return m_creature.brain(); }

    bool isRunning() const { return m_running; }
    float simulationSpeed() const { return m_simSpeed; }
    bool showBrainVis() const { return m_showBrain; }

    enum class CameraMode { Follow, Free, TopDown, BrainView };
    CameraMode cameraMode() const { return m_camMode; }

public slots:
    void toggleSimulation();
    void resetSimulation();
    void setSimulationSpeed(float speed);
    void setShowBrain(bool show);
    void setCameraMode(CameraMode mode);
    void stepOnce();

signals:
    void statsUpdated(int neurons, int synapses, int spikes,
                      float firingRate, float speed, float energy,
                      int foodEaten, float distance, const QString& behavior,
                      float dopamine, float serotonin, float octopamine);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

#if defined(BRAINSIM_MOUSE_INPUT)
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
#endif

#if defined(BRAINSIM_TOUCH_INPUT)
    bool event(QEvent* e) override;
    void handleTouchEvent(QTouchEvent* e);
    void handlePinchGesture(QPinchGesture* gesture);
#endif

private:
    void setupLighting();
    void setupCamera();
    void updateSimulation();
    void emitStats();

    Creature m_creature;
    World m_world;
    CreatureRenderer m_creatureRenderer;
    WorldRenderer m_worldRenderer;
    BrainRenderer m_brainRenderer;

    QTimer* m_timer;
    QElapsedTimer m_elapsed;
    float m_simSpeed = 1.0f;
    bool m_running = false;
    bool m_showBrain = true;

    // Camera
    CameraMode m_camMode = CameraMode::Follow;
    float m_camDist = 5.0f;
    float m_camRotX = 30.0f;
    float m_camRotY = 45.0f;
    float m_camPanX = 0, m_camPanZ = 0;

#if defined(BRAINSIM_MOUSE_INPUT)
    QPoint m_lastMouse;
    bool m_mousePressed = false;
    int m_mouseButton = 0;
#endif

#if defined(BRAINSIM_TOUCH_INPUT)
    QPointF m_lastTouch;
    float m_lastPinchScale = 1.0f;
    int m_touchPointCount = 0;
#endif
};
