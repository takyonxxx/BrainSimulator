#include "platform.h"   // MUST be first - defines NOMINMAX before windows.h
#include "glwidget.h"
#include <QOpenGLContext>
#include <cmath>

#if defined(BRAINSIM_WINDOWS)
    #include <windows.h>
    #include <GL/glu.h>
#elif defined(BRAINSIM_MACOS)
    // macOS deprecated OpenGL but still supports it
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    #include <OpenGL/glu.h>
#elif defined(BRAINSIM_LINUX)
    #include <GL/glu.h>
#elif defined(BRAINSIM_IOS) || defined(BRAINSIM_ANDROID)
    // GLES has no gluPerspective/gluLookAt - we implement our own
#endif

// ============================================================================
// GLU replacements for platforms without GLU (iOS, Android)
// ============================================================================
#if defined(BRAINSIM_GLES)

static void brainsim_gluPerspective(float fovY, float aspect, float zNear, float zFar)
{
    float f = 1.0f / tanf(fovY * M_PI / 360.0f);
    float m[16] = {0};
    m[0]  = f / aspect;
    m[5]  = f;
    m[10] = (zFar + zNear) / (zNear - zFar);
    m[11] = -1.0f;
    m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
    glMultMatrixf(m);
}

static void brainsim_gluLookAt(float eyeX, float eyeY, float eyeZ,
                                float centerX, float centerY, float centerZ,
                                float upX, float upY, float upZ)
{
    float fx = centerX - eyeX, fy = centerY - eyeY, fz = centerZ - eyeZ;
    float fl = sqrtf(fx*fx + fy*fy + fz*fz);
    fx /= fl; fy /= fl; fz /= fl;

    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;
    float sl = sqrtf(sx*sx + sy*sy + sz*sz);
    sx /= sl; sy /= sl; sz /= sl;

    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    float m[16] = {
        sx,  ux, -fx, 0,
        sy,  uy, -fy, 0,
        sz,  uz, -fz, 0,
        0,   0,   0,  1
    };
    glMultMatrixf(m);
    glTranslatef(-eyeX, -eyeY, -eyeZ);
}

#define gluPerspective brainsim_gluPerspective
#define gluLookAt brainsim_gluLookAt

#endif // BRAINSIM_GLES

// ============================================================================
// Constructor / Destructor
// ============================================================================

GLWidget::GLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

#if defined(BRAINSIM_TOUCH_INPUT)
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    grabGesture(Qt::PinchGesture);
#endif

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        if (m_running) {
            updateSimulation();
        }
        update();
    });
    m_timer->setInterval(BRAINSIM_TIMER_INTERVAL_MS);
    m_elapsed.start();
}

GLWidget::~GLWidget() {}

// ============================================================================
// OpenGL Initialization
// ============================================================================

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0.15f, 0.18f, 0.25f, 1.0f);
    glEnable(GL_DEPTH_TEST);

#if defined(BRAINSIM_GL_DESKTOP)
    // Desktop OpenGL legacy features
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setupLighting();
#elif defined(BRAINSIM_GLES)
    // OpenGL ES - legacy fixed pipeline still available in ES 1.1 compat
    // For ES 2.0+ we would need shaders, but Qt's QOpenGLWidget
    // provides compatibility wrappers on most platforms
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    #if !defined(BRAINSIM_IOS)
    // Android Qt often provides legacy compat
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);
    setupLighting();
    #endif
#endif

    // Initialize simulation
    m_creature.initialize();
    m_world.initialize();
    m_elapsed.restart();
    m_timer->start();
}

void GLWidget::setupLighting()
{
#if defined(BRAINSIM_GL_DESKTOP) || !defined(BRAINSIM_IOS)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Main directional light (sun)
    GLfloat light0Pos[]  = {5.0f, 10.0f, 5.0f, 0.0f};
    GLfloat light0Amb[]  = {0.25f, 0.25f, 0.3f, 1.0f};
    GLfloat light0Dif[]  = {0.9f, 0.85f, 0.8f, 1.0f};
    GLfloat light0Spec[] = {1.0f, 1.0f, 0.9f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0Amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Spec);

    // Fill light
    GLfloat light1Pos[] = {-3.0f, 5.0f, -3.0f, 0.0f};
    GLfloat light1Dif[] = {0.3f, 0.35f, 0.4f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Dif);
#endif
}

// ============================================================================
// Camera
// ============================================================================

void GLWidget::setupCamera()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float cx = m_creature.x();
    float cy = m_creature.y();
    float cz = m_creature.z();

    switch (m_camMode) {
    case CameraMode::Follow: {
        float radX = m_camRotX * (float)M_PI / 180.0f;
        float radY = m_camRotY * (float)M_PI / 180.0f;
        float eyeX = cx + m_camDist * cosf(radX) * sinf(radY);
        float eyeY = cy + m_camDist * sinf(radX);
        float eyeZ = cz + m_camDist * cosf(radX) * cosf(radY);
        gluLookAt(eyeX, eyeY, eyeZ, cx, cy + 0.1f, cz, 0, 1, 0);
        break;
    }
    case CameraMode::Free: {
        float radX = m_camRotX * (float)M_PI / 180.0f;
        float radY = m_camRotY * (float)M_PI / 180.0f;
        float eyeX = m_camPanX + m_camDist * cosf(radX) * sinf(radY);
        float eyeY = m_camDist * sinf(radX);
        float eyeZ = m_camPanZ + m_camDist * cosf(radX) * cosf(radY);
        gluLookAt(eyeX, eyeY, eyeZ, m_camPanX, 0, m_camPanZ, 0, 1, 0);
        break;
    }
    case CameraMode::TopDown: {
        gluLookAt(cx, 8.0f, cz + 0.01f, cx, 0, cz, 0, 0, -1);
        break;
    }
    case CameraMode::BrainView: {
        gluLookAt(cx + 2.0f, cy + 2.0f, cz + 2.0f,
                  cx, cy + 1.5f, cz, 0, 1, 0);
        break;
    }
    }
}

// ============================================================================
// Rendering
// ============================================================================

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setupCamera();

    // Draw world
    m_worldRenderer.draw(this, m_world);

    // Draw creature
    m_creatureRenderer.draw(this, m_creature);

    // Draw brain visualization (floating above creature)
    if (m_showBrain) {
#if defined(BRAINSIM_GL_DESKTOP)
        glDisable(GL_LIGHTING);
#endif
        float brainY = m_creature.y() + 1.2f;
        m_brainRenderer.draw(this, m_creature.brain(),
                             m_creature.x(), brainY, m_creature.z(), 0.6f);

        // HUD overlay
        m_brainRenderer.drawHUD(m_creature.brain(), width(), height());

#if defined(BRAINSIM_GL_DESKTOP)
        glEnable(GL_LIGHTING);
#endif
    }
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

#if defined(BRAINSIM_MOBILE)
    // Wider FOV on mobile for better visibility
    float aspect = (float)w / (std::max)(1, h);
    gluPerspective(55.0f, aspect, 0.1f, 100.0f);
#else
    float aspect = (float)w / (std::max)(1, h);
    gluPerspective(45.0f, aspect, 0.1f, 100.0f);
#endif

    glMatrixMode(GL_MODELVIEW);
}

// ============================================================================
// Simulation
// ============================================================================

void GLWidget::updateSimulation()
{
    float dt = (1.0f / BRAINSIM_TARGET_FPS) * m_simSpeed;

    auto foodPos = m_world.getFoodPositions();
    auto obsPos = m_world.getObstaclePositions();

    m_creature.update(dt, foodPos, obsPos, m_world.size());
    m_world.update(dt, m_creature);

    emitStats();
}

void GLWidget::emitStats()
{
    QString behavior;
    switch (m_creature.currentBehavior()) {
    case Creature::Behavior::Exploring: behavior = "Kesfediyor"; break;
    case Creature::Behavior::Feeding:   behavior = "Besleniyor"; break;
    case Creature::Behavior::Avoiding:  behavior = "Kaciyor"; break;
    case Creature::Behavior::Turning:   behavior = "Donuyor"; break;
    case Creature::Behavior::Idle:      behavior = "Bekliyor"; break;
    }

    emit statsUpdated(
        m_creature.brain().neuronCount(),
        m_creature.brain().synapseCount(),
        m_creature.brain().totalSpikes(),
        m_creature.brain().meanFiringRate(),
        m_creature.speed(),
        m_creature.energy(),
        m_creature.foodEaten(),
        m_creature.distanceTraveled(),
        behavior,
        m_creature.brain().dopamineLevel(),
        m_creature.brain().serotoninLevel(),
        m_creature.brain().octopamineLevel()
    );
}

// ============================================================================
// Slots
// ============================================================================

void GLWidget::toggleSimulation()
{
    m_running = !m_running;
}

void GLWidget::resetSimulation()
{
    m_running = false;
    m_creature.reset();
    m_creature.initialize();
    m_world.reset();
    update();
    emitStats();
}

void GLWidget::setSimulationSpeed(float speed)
{
    m_simSpeed = speed;
}

void GLWidget::setShowBrain(bool show)
{
    m_showBrain = show;
    update();
}

void GLWidget::setCameraMode(CameraMode mode)
{
    m_camMode = mode;
    update();
}

void GLWidget::stepOnce()
{
    updateSimulation();
    update();
}

// ============================================================================
// Desktop Mouse/Keyboard Input
// ============================================================================

#if defined(BRAINSIM_MOUSE_INPUT)

void GLWidget::mousePressEvent(QMouseEvent* e)
{
    m_lastMouse = e->pos();
    m_mousePressed = true;
    m_mouseButton = (int)e->button();
}

void GLWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_mousePressed) return;

    int dx = e->pos().x() - m_lastMouse.x();
    int dy = e->pos().y() - m_lastMouse.y();

    if (m_mouseButton == (int)Qt::LeftButton) {
        m_camRotY += dx * 0.5f;
        m_camRotX += dy * 0.5f;
        m_camRotX = (std::clamp)(m_camRotX, 5.0f, 85.0f);
    } else if (m_mouseButton == (int)Qt::RightButton) {
        m_camPanX += dx * 0.02f;
        m_camPanZ += dy * 0.02f;
    }

    m_lastMouse = e->pos();
    update();
}

void GLWidget::mouseReleaseEvent(QMouseEvent* e)
{
    Q_UNUSED(e);
    m_mousePressed = false;
}

void GLWidget::wheelEvent(QWheelEvent* e)
{
    m_camDist -= e->angleDelta().y() * 0.005f;
    m_camDist = (std::clamp)(m_camDist, 1.0f, 30.0f);
    update();
}

void GLWidget::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {
    case Qt::Key_Space:
        toggleSimulation();
        break;
    case Qt::Key_R:
        resetSimulation();
        break;
    case Qt::Key_B:
        setShowBrain(!m_showBrain);
        break;
    case Qt::Key_1:
        setCameraMode(CameraMode::Follow);
        break;
    case Qt::Key_2:
        setCameraMode(CameraMode::Free);
        break;
    case Qt::Key_3:
        setCameraMode(CameraMode::TopDown);
        break;
    case Qt::Key_4:
        setCameraMode(CameraMode::BrainView);
        break;
    case Qt::Key_S:
        stepOnce();
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        setSimulationSpeed((std::min)(10.0f, m_simSpeed * 1.5f));
        break;
    case Qt::Key_Minus:
        setSimulationSpeed((std::max)(0.1f, m_simSpeed / 1.5f));
        break;
    default:
        QOpenGLWidget::keyPressEvent(e);
    }
}

#endif // BRAINSIM_MOUSE_INPUT

// ============================================================================
// Mobile Touch Input (iOS / Android)
// ============================================================================

#if defined(BRAINSIM_TOUCH_INPUT)

bool GLWidget::event(QEvent* e)
{
    switch (e->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        handleTouchEvent(static_cast<QTouchEvent*>(e));
        return true;
    case QEvent::Gesture: {
        auto* ge = static_cast<QGestureEvent*>(e);
        if (auto* pinch = static_cast<QPinchGesture*>(ge->gesture(Qt::PinchGesture))) {
            handlePinchGesture(pinch);
        }
        return true;
    }
    default:
        return QOpenGLWidget::event(e);
    }
}

void GLWidget::handleTouchEvent(QTouchEvent* e)
{
    const auto& points = e->points();
    m_touchPointCount = points.size();

    if (points.size() == 1) {
        // Single finger drag -> camera rotation
        QPointF pos = points[0].position();

        if (e->type() == QEvent::TouchBegin) {
            m_lastTouch = pos;
        } else if (e->type() == QEvent::TouchUpdate) {
            float dx = pos.x() - m_lastTouch.x();
            float dy = pos.y() - m_lastTouch.y();

            m_camRotY += dx * 0.3f;
            m_camRotX += dy * 0.3f;
            m_camRotX = (std::clamp)(m_camRotX, 5.0f, 85.0f);

            m_lastTouch = pos;
            update();
        }
    } else if (points.size() == 2) {
        // Two-finger drag -> camera pan
        QPointF center = (points[0].position() + points[1].position()) / 2.0f;

        if (e->type() == QEvent::TouchUpdate) {
            float dx = center.x() - m_lastTouch.x();
            float dy = center.y() - m_lastTouch.y();
            m_camPanX += dx * 0.01f;
            m_camPanZ += dy * 0.01f;
            update();
        }
        m_lastTouch = center;
    }

    // Double tap to toggle simulation
    if (e->type() == QEvent::TouchBegin && points.size() == 1) {
        static qint64 lastTapTime = 0;
        qint64 now = m_elapsed.elapsed();
        if (now - lastTapTime < 300) {
            toggleSimulation();
        }
        lastTapTime = now;
    }
}

void GLWidget::handlePinchGesture(QPinchGesture* gesture)
{
    if (gesture->state() == Qt::GestureStarted) {
        m_lastPinchScale = 1.0f;
    }

    float scaleFactor = gesture->scaleFactor();
    m_camDist /= scaleFactor;
    m_camDist = (std::clamp)(m_camDist, 1.0f, 30.0f);
    update();
}

#endif // BRAINSIM_TOUCH_INPUT
