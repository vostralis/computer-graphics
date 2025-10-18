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

    m_renderMode = RenderMode::NoCulling;
}

Scene::~Scene() {
    delete object;
}

void Scene::setupViewMatrix() {
    QVector3D eye(15, 10, 15);
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

void Scene::drawArrow(QPainter &painter, QPointF start, QPointF end, const QColor &color) {
    // 1. Set up the pen and brush for this arrow
    painter.setPen(QPen(color, 2));
    painter.setBrush(QBrush(color));

    // 2. Define the main line of the arrow
    QLineF line(start, end);
    if (line.length() < 2.0) return;

    // 3. Calculate the geometry of the arrowhead
    const double angle = std::atan2(-line.dy(), line.dx());
    const qreal arrowSize = 15.0;

    // Calculate the two points for the base of the arrowhead triangle
    QPointF arrowP1 = line.p2() - QPointF(std::sin(angle + M_PI / 3) * arrowSize,
                                          std::cos(angle + M_PI / 3) * arrowSize);
    QPointF arrowP2 = line.p2() - QPointF(std::sin(angle + M_PI - M_PI / 3) * arrowSize,
                                          std::cos(angle + M_PI - M_PI / 3) * arrowSize);
    QPolygonF arrowheadPolygon;
    arrowheadPolygon << line.p2() << arrowP1 << arrowP2;

    // 4. Draw the line and the filled arrowhead
    painter.drawLine(line);
    painter.drawPolygon(arrowheadPolygon);
    painter.setBrush(Qt::NoBrush);
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

    drawArrow(painter, origin, x, Qt::green); // X-Axis
    drawArrow(painter, origin, y, Qt::red);   // Y-Axis
    drawArrow(painter, origin, z, Qt::blue);  // Z-Axis
}

void Scene::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw the axis
    drawAxis(painter);
    // Draw the object by drawing each of its faces
    renderFaces(painter);
}

void Scene::buildFaces(QVector<QVector<QPointF>>& visiblePolygons, QVector<QVector<QPointF>>& invisiblePolygons) {
    const QVector<QVector3D>& vertices = object->getVertices();
    const QVector<QVector<int32_t>>& faces = object->getFaces();
    QMatrix4x4 model = object->getModelMatrix();
    QMatrix4x4 modelView = m_viewMatrix * model;
    QVector<QPointF> screenCoordinates;

    // Evaluate screen coordinate for each vertex
    for (const QVector3D& vertex : vertices) {
        screenCoordinates.append(project(vertex, model));
    }

    // Construct each face
    for (const QVector<int32_t>& face : faces) {
        bool isVisible = isFaceVisible(face, modelView);

        if (m_renderMode == RenderMode::BackfaceCulling && !isVisible) {
            continue;
        }

        QVector<QPointF> polygon;
        bool outOfBounds = false;

        for (int32_t vertex : face) {
            QPointF point = screenCoordinates[vertex];

            // Discard point if it's outside of a scene to prevent visual bugs
            if (point.x() < -1e5 || point.y() < -1e5) {
                outOfBounds = true;
                break;
            }

            polygon.emplaceBack(point);
        }

        if (outOfBounds) continue;

        // If current render mode is visualization, then render
        // invisible polygons first and visible ones last
        if (m_renderMode == RenderMode::Visualization) {
            if (isVisible) {
                visiblePolygons.emplaceBack(polygon);
            } else {
                invisiblePolygons.emplaceBack(polygon);
            }
        } else {
            visiblePolygons.emplaceBack(polygon);
        }
    }
}

void Scene::renderFaces(QPainter& painter) {
    // Construct polygons for the painter
    QVector<QVector<QPointF>> visiblePolygons, invisiblePolygons;
    buildFaces(visiblePolygons, invisiblePolygons);

    // Set up pens and brushes for rendering
    static constexpr qreal width = 3;
    static const QPen defaultPen(QColor(0, 0, 0), width);
    static const QPen visiblePen(QColor(0, 0, 180), width);
    static QPen invisiblePen(QColor(180, 0, 0), width);
    invisiblePen.setDashPattern({4, 4});

    if (m_renderMode == RenderMode::Visualization) {
        painter.setPen(invisiblePen);
    } else {
        painter.setPen(defaultPen);
    }

    // Draw invisible polygons first
    for (const auto& polygon : std::as_const(invisiblePolygons)) {
        painter.drawPolygon(polygon);
    }

    if (m_renderMode == RenderMode::Visualization) {
        painter.setPen(visiblePen);
    }

    // Draw visible polygons last
    for (const auto& polygon : std::as_const(visiblePolygons)) {
        painter.drawPolygon(polygon);
    }
}

bool Scene::isFaceVisible(const QVector<int32_t>& face, const QMatrix4x4& modelViewMatrix) const {
    // Get first three vertices of a face
    QVector<QVector3D> vertices = object->getVertices();
    QVector3D p0 = vertices[face[0]];
    QVector3D p1 = vertices[face[1]];
    QVector3D p2 = vertices[face[2]];

    QVector3D p0_transformed = modelViewMatrix.map(p0);
    QVector3D p1_transformed = modelViewMatrix.map(p1);
    QVector3D p2_transformed = modelViewMatrix.map(p2);

    // Create 2 vectors based on this points
    QVector3D A = p1_transformed - p0_transformed;
    QVector3D B = p2_transformed - p0_transformed;

    // Calculate their cross product to get a normal vector of a surface
    QVector3D normal = QVector3D::crossProduct(A, B);
    QVector3D viewVector = -p0_transformed;

    // Since we use counter-clockwise traversal order of faces' vertices,
    // dot product should be positive in order for a face to be visible
    return QVector3D::dotProduct(normal, viewVector) > 1e-6f;
}

void Scene::setRenderMode(RenderMode mode) {
    if (m_renderMode != mode) {
        m_renderMode = mode;
        update();
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
    } else if (event->key() == Qt::Key_R) {
        object->reset();
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
