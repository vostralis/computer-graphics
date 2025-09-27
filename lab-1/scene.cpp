#include "scene.h"

Scene::Scene(QWidget *parent)
    : QWidget{parent}
{
    object = new Object();
    setupViewMatrix();
    setupProjectionMatrix();
    m_worldWidth = m_worldHeight = m_worldDepth = 20;
    m_translationSensetivity = 2.0f;
    m_rotationSensitivity = 0.01f;

    // Set scene's background color
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    setPalette(palette);
    setAutoFillBackground(true);

    // Allow MainWindow's inner widget to capture mouse and keyboard events
    setFocusPolicy(Qt::StrongFocus);

    // Timer that is responsible for calling widget's update method each frame
    QTimer *renderTimer = new QTimer(this);
    connect(renderTimer, &QTimer::timeout, this, QOverload<>::of(&Scene::update));
    renderTimer->start(16); // at 60 FPS

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &Scene::onAnimationTick);
}

Scene::~Scene() {
    delete object;
}

void Scene::setupViewMatrix() {
    QVector3D eye(15, 15, 15);
    QVector3D center(0.0f, 0.0f, 0.0f);
    QVector3D up(0.0f, 1.0f, 0.0f);

    m_viewMatrix.lookAt(eye, center, up);
}

void Scene::setupProjectionMatrix() {
    // Perspective projection matrix looks like:
    // [1 0 0  0]
    // [0 1 0  0]
    // [0 0 1 -1/c]
    // [0 0 0  1]
    // where c is a center of projection

    float h = static_cast<float>(height());
    float w = static_cast<float>(width());
    float aspectRatio = (h > 0.0f) ? (w / h) : 1.0f;
    float c = 1.0f;

    m_projectionMatrix.setToIdentity();
    m_projectionMatrix(3, 2) = -1.0f / c;
    m_projectionMatrix.scale(1.0f / aspectRatio, 1.0f, 1.0f);
}

QPointF Scene::project(const QVector3D& vertex, const QMatrix4x4& model) const {
    QMatrix4x4 mvp = m_projectionMatrix * m_viewMatrix * model;
    QVector4D clip = mvp * QVector4D(vertex, 1.0f);

    if (qFuzzyIsNull(clip.w())) return QPointF(-1e6, -1e6);

    QVector4D ndc4 = clip / clip.w();

    // Discard the point if it's outside normal range
    if (ndc4.x() < -1.0f || ndc4.x() > 1.0f || ndc4.y() < -1.0f || ndc4.y() > 1.0f || ndc4.z() < -1.0f || ndc4.z() > 1.0f)
        return QPointF(-1e6, -1e6);

    // Calculate point's screen coordinates
    float x = (ndc4.x() + 1.0f) * 0.5f * width();
    float y = (1.0f - ndc4.y()) * 0.5f * height();

    return QPointF(x, y);
}

void Scene::drawAxis(QPainter& painter) {
    QMatrix4x4 identity;

    // Projecting origin and axis' end points onto screen
    QPointF origin = project(QVector3D(0, 0, 0), identity);
    QPointF x = project(QVector3D(15, 0, 0), identity);
    QPointF y = project(QVector3D(0, 15, 0), identity);
    QPointF z = project(QVector3D(0, 0, 15), identity);

    // Drawing the X-axis
    QPen axisPen(Qt::green, 2);
    painter.setPen(axisPen);
    painter.drawLine(origin, x);

    // Drawing the Y-axis
    axisPen.setColor(Qt::red);
    painter.setPen(axisPen);
    painter.drawLine(origin, y);

    // Drawing the Z-axis
    axisPen.setColor(Qt::blue);
    painter.setPen(axisPen);
    painter.drawLine(origin, z);
}

void Scene::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw the axis
    drawAxis(painter);
    QPen objectPen(Qt::black, 2);
    painter.setPen(objectPen);

    QList<QVector3D> vertices = object->getVertices();
    QList<QPair<int32_t, int32_t>> edges = object->getEdges();
    QMatrix4x4 model = object->getModelMatrix();
    QList<QPointF> screenCoordinates;

    // Evaluate screen coordinate for each vertex
    for (const QVector3D& vertex : vertices) {
        screenCoordinates.append(project(vertex, model));
    }

    // Draw the object by connecting vertices
    for (const QPair<int32_t, int32_t>& edge : edges) {
        QPointF p1 = screenCoordinates[edge.first];
        QPointF p2 = screenCoordinates[edge.second];

        if (p1.x() < -1e5 || p1.y() < -1e5 || p2.x() < -1e5 || p2.y() < -1e5) continue;

        painter.drawLine(p1, p2);
    }
}

void Scene::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & (Qt::RightButton | Qt::LeftButton)) {
        m_lastMousePos = event->position();
    }
}

void Scene::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::RightButton) {
        // Right mouse button is responsible for translation
        QPointF delta = event->position() - m_lastMousePos;
        m_lastMousePos = event->position();

        if (event->modifiers() & Qt::ShiftModifier) {
            float tz = delta.y() / height() * m_worldDepth * m_translationSensetivity;
            object->translate(0.0f, 0.0f, tz);
        } else {
            float tx = delta.x() / width() * m_worldWidth * m_translationSensetivity;
            float ty = -delta.y() / height() * m_worldHeight * m_translationSensetivity;
            object->translate(tx, ty, 0.0f);
        }
    } else if (event->buttons() & Qt::LeftButton) {
        // Left mouse buttom is responsible for rotation
        QPointF delta = event->position() - m_lastMousePos;
        m_lastMousePos = event->position();

        if (event->modifiers() & Qt::ShiftModifier) {
            float angleZ = delta.x() * m_rotationSensitivity;
            object->rotateZ(angleZ);
        } else {
            float angleX = -delta.y() * m_rotationSensitivity;
            float angleY = -delta.x() * m_rotationSensitivity;

            object->rotateX(angleX);
            object->rotateY(angleY);
        }
    }
}

void Scene::mouseReleaseEvent(QMouseEvent *event) {
    if (event->buttons() & (Qt::RightButton | Qt::LeftButton)) {
        m_lastMousePos = event->position();
    }
}

void Scene::wheelEvent(QWheelEvent *event) {
    // Wheel is resposible for scaling
    float scaleFactor = 1.125f;
    float steps = static_cast<float>(event->angleDelta().y()) / 120.0f;
    object->scale(std::pow(scaleFactor, steps));

    event->accept();
}

void Scene::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    setupProjectionMatrix();
}

void Scene::onAnimationTick() {
    object->updateAnimation();

    update();
}

void Scene::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space) {
        toggleAnimation();
    }
}

void Scene::toggleAnimation() {
    object->toggleAnimation();

    if (object->isAnimationRunning()) {
        animationTimer->start(16);
    } else {
        animationTimer->stop();
    }
}

Object* Scene::getObject() const {
    return object;
}
