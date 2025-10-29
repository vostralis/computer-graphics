#include "scene.h"
#include <QDebug>

Scene::Scene(QWidget *parent)
    : QWidget{parent}
{
    m_object = new Object();

    m_camera = new Camera();
    m_cameraMovement = false;

    m_lightSource = new LightSource();

    setupProjectionMatrix();
    m_worldWidth = m_worldHeight = m_worldDepth = 20;
    m_translationSensetivity = 2.0f;
    m_rotationSensitivity = 0.01f;

    m_LightingMode = LightingMode::Simple;
    m_ambientIntensity = 0.1f;

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
}

Scene::~Scene() {
    delete m_object;
    delete m_camera;
    delete m_lightSource;
}

void Scene::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_cameraMovement) {
        m_camera->update(1.0f / 60.0f);
    }

    QMatrix4x4 model = m_object->getModelMatrix();
    QMatrix4x4 view = m_camera->getViewMatrix();

    QMatrix4x4 modelView = view * model;

    // Draw the axis
    renderAxis(painter, view);
    // Render triangulated object
    renderTriangles(painter, modelView);
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

QPointF Scene::project(const QVector3D& vertex, const QMatrix4x4& modelView) const {
    QMatrix4x4 mvp = m_projectionMatrix * modelView;
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

void Scene::renderArrow(QPainter& painter, QPointF start, QPointF end, const QColor& color) {
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

void Scene::renderAxis(QPainter& painter, const QMatrix4x4& view) {
    QMatrix4x4 identity; // Is set to identity matrix by default
    identity.setToIdentity();

    // Projecting origin and axis' end points onto screen
    QPointF origin = project(QVector3D(0, 0, 0), view * identity);
    QPointF x      = project(QVector3D(10, 0, 0), view * identity);
    QPointF y      = project(QVector3D(0, 10, 0), view * identity);
    QPointF z      = project(QVector3D(0, 0, 10), view * identity);

    // Drawing the X-axis
    QPen axisPen(QColor(0, 255, 0, 40), 2);
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

    renderArrow(painter, origin, x, Qt::green); // X-Axis
    renderArrow(painter, origin, y, Qt::red);   // Y-Axis
    renderArrow(painter, origin, z, Qt::blue);  // Z-Axis
}

void Scene::renderTriangles(QPainter& painter, const QMatrix4x4& modelView) {
    const QVector<QVector3D>& vertices = m_object->getVertices();
    const QVector<Triangle>& triangles = m_object->getTriangles();
    const QVector<QVector3D>& vertexNormals = m_object->getVertexNormals();

    QVector<QPointF> screenCoordinates;

    for (const QVector3D& vertex : vertices) {
        screenCoordinates.append(project(vertex, modelView));
    }

    QImage frameBuffer(width(), height(), QImage::Format_ARGB32);
    frameBuffer.fill(Qt::transparent);

    for (const auto& triangle : triangles) {
        if (!isFaceVisible(triangle, modelView)) continue;

        if (m_LightingMode == LightingMode::Simple) {
            QPointF p1 = screenCoordinates[triangle.a];
            QPointF p2 = screenCoordinates[triangle.b];
            QPointF p3 = screenCoordinates[triangle.c];

            float intensity = calculateLightIntensity(triangle);
            float hue = 255 * intensity;
            QColor color(hue, hue, hue);

            painter.setBrush(color);
            painter.setPen(color);
            painter.drawPolygon({p1, p2, p3});
        } else {
            drawTriangleGouraud(frameBuffer, triangle, modelView);
        }
    }

    if (m_LightingMode == LightingMode::GouraudShading) {
        painter.drawImage(0, 0, frameBuffer);
    }
}

float Scene::calculateVertexIntensity(const QVector3D& vertexWorld, const QVector3D& normalWorld) const {
    QVector3D light = (m_lightSource->position() - vertexWorld).normalized();

    float ambientComponent = m_ambientIntensity * m_object->getAmbientCoefficient();

    float dot = std::max(0.0f, QVector3D::dotProduct(normalWorld, light));
    float diffuseComponent = dot * m_lightSource->intensity() * m_object->getDiffuseCoefficient();

    float intensity = std::min(1.0f, ambientComponent + diffuseComponent);
    return intensity;
}

float Scene::calculateLightIntensity(const Triangle& triangle) const {
    QMatrix4x4 modelMatrix = m_object->getModelMatrix();
    const QVector<QVector3D>& vertices = m_object->getVertices();

    // Local vertices of a triangle
    QVector3D p0 = vertices[triangle.a];
    QVector3D p1 = vertices[triangle.b];
    QVector3D p2 = vertices[triangle.c];

    // World vertices of a triangle
    QVector3D p0_world = modelMatrix.map(p0);
    QVector3D p1_world = modelMatrix.map(p1);
    QVector3D p2_world = modelMatrix.map(p2);

    // 2 vectors on a surface of a triangle
    QVector3D A = p1_world - p0_world;
    QVector3D B = p2_world - p0_world;

    QVector3D centroid = (p0_world + p1_world + p2_world) / 3.0f; // Centroid of a triangle
    QVector3D light = (m_lightSource->position() - centroid).normalized(); // Light vector
    QVector3D normal = QVector3D::crossProduct(A, B).normalized(); // Normal vector to triangle's surface

    float ambientComponent = m_ambientIntensity * m_object->getAmbientCoefficient();

    float dot = std::max(0.0f, QVector3D::dotProduct(normal, light));
    float diffuseComponent = dot * m_lightSource->intensity() * m_object->getDiffuseCoefficient();

    float intensity = std::min(1.0f, ambientComponent + diffuseComponent);
    return intensity;
}

void Scene::drawTriangleGouraud(QImage& frameBuffer, const Triangle& triangle, const QMatrix4x4& modelView) {
    const QVector<QVector3D>& vertices = m_object->getVertices();
    const QVector<QVector3D>& vertexNormals = m_object->getVertexNormals();
    QMatrix4x4 model = m_object->getModelMatrix();
    QMatrix3x3 normalMatrix = model.normalMatrix();

    auto transformNormal = [](const QMatrix3x3& m, const QVector3D& n) {
        return QVector3D(
            m(0,0) * n.x() + m(0,1) * n.y() + m(0,2) * n.z(),
            m(1,0) * n.x() + m(1,1) * n.y() + m(1,2) * n.z(),
            m(2,0) * n.x() + m(2,1) * n.y() + m(2,2) * n.z()
        );
    };

    // Get vertices in world space
    const QVector3D& v0 = vertices[triangle.a];
    const QVector3D& v1 = vertices[triangle.b];
    const QVector3D& v2 = vertices[triangle.c];

    QVector3D v0_world = model.map(v0);
    QVector3D v1_world = model.map(v1);
    QVector3D v2_world = model.map(v2);

    // Get vertex normals in world space
    const QVector3D& n0 = vertexNormals[triangle.a];
    const QVector3D& n1 = vertexNormals[triangle.b];
    const QVector3D& n2 = vertexNormals[triangle.c];

    QVector3D n0_world = transformNormal(normalMatrix, n0).normalized();
    QVector3D n1_world = transformNormal(normalMatrix, n1).normalized();
    QVector3D n2_world = transformNormal(normalMatrix, n2).normalized();

    // Calculate intensity at each vertex
    float i0 = calculateVertexIntensity(v0_world, n0_world);
    float i1 = calculateVertexIntensity(v1_world, n1_world);
    float i2 = calculateVertexIntensity(v2_world, n2_world);

    // Project each vertex onto a 2D scene
    QPointF p0 = project(v0, modelView);
    QPointF p1 = project(v1, modelView);
    QPointF p2 = project(v2, modelView);

    // Fill the triangle using Gouraud shading
    fillTriangleGouraud(frameBuffer, p0,i0, p1,i1, p2,i2);
}

void Scene::fillTriangleGouraud(
    QImage& frameBuffer,
    const QPointF& p0, float i0,
    const QPointF& p1, float i1,
    const QPointF& p2, float i2)
{
    // Struct that combines the vertex and its intensity
    struct Vertex2D {
        qreal x, y, i;
    };

    Vertex2D v[3] = {
        { p0.x(), p0.y(), i0 },
        { p1.x(), p1.y(), i1 },
        { p2.x(), p2.y(), i2 }
    };

    // Sort by Y ascending
    std::sort(v, v + 3, [](auto& a, auto& b) { return a.y < b.y; });

    // Interpolate X and intensity at a given Y along the edge of a triangle
    auto interpolateEdgeAtY = [](const Vertex2D& start, const Vertex2D& end, float y) {
        // Y-distance from start to end for a given Y
        float t = (end.y == start.y) ? 0.0f : (y - start.y) / (end.y - start.y);
        float x = start.x + t * (end.x - start.x);
        float intensity = start.i + t * (end.i - start.i);
        return std::pair<float, float>(x, intensity);
    };

    // Draw horizontal scanline at a given Y from x1 to x2
    auto drawHorizontalScanline = [&](int y, float x1, float i1, float x2, float i2) {
        if (x1 > x2) {
            std::swap(x1, x2);
            std::swap(i1, i2);
        }

        // Convert to a pixel coordinates
        int xStart = std::max(0, int(std::ceil(x1)));
        int xEnd   = std::min(frameBuffer.width() - 1, int(std::floor(x2)));

        float dx = x2 - x1; // Horizontal length of a scanline
        float di = (dx != 0) ? (i2 - i1) / dx : 0.0f; // Intensity increment per pixel

        QRgb* line = reinterpret_cast<QRgb*>(frameBuffer.scanLine(y)); // Raw pointer to a y-th framebuffer row
        float currI = i1 + (xStart - x1) * di;

        // Loop over each pixel in y-th row
        for (int x = xStart; x <= xEnd; ++x) {
            int val = std::clamp(int(currI * 255), 0, 255); // Calculate intensity for current pixel
            line[x] = qRgb(val, val, val); // Set the pixel color
            currI += di; // Increment intensity by di
        }
    };

    // Convert to a pixel coordinates
    int yStart = std::max(0, int(std::ceil(v[0].y)));
    int yEnd   = std::min(frameBuffer.height() - 1, int(std::floor(v[2].y)));

    // Loop over each scanline from top to bottom of a current triangle
    for (int y = yStart; y <= yEnd; ++y) {
        if (y < v[1].y) { // Upper half of a triangle
            auto [xL, iL] = interpolateEdgeAtY(v[0], v[1], y);
            auto [xR, iR] = interpolateEdgeAtY(v[0], v[2], y);
            drawHorizontalScanline(y, xL, iL, xR, iR);
        } else { // Lower half
            auto [xL, iL] = interpolateEdgeAtY(v[1], v[2], y);
            auto [xR, iR] = interpolateEdgeAtY(v[0], v[2], y);
            drawHorizontalScanline(y, xL, iL, xR, iR);
        }
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
            m_object->translate(0.0f, 0.0f, tz);
        } else {
            float tx = delta.x() / width() * m_worldWidth * m_translationSensetivity;
            float ty = -delta.y() / height() * m_worldHeight * m_translationSensetivity;
            m_object->translate(tx, ty, 0.0f);
        }
    } else if (event->buttons() & Qt::LeftButton) {
        // Left mouse buttom is responsible for rotation
        QPointF delta = event->position() - m_lastMousePos;
        m_lastMousePos = event->position();

        if (event->modifiers() & Qt::ShiftModifier) {
            float angleZ = delta.x() * m_rotationSensitivity;
            m_object->rotateZ(angleZ);
        } else {
            float angleX = -delta.y() * m_rotationSensitivity;
            float angleY = -delta.x() * m_rotationSensitivity;

            m_object->rotateX(angleX);
            m_object->rotateY(angleY);
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
    m_object->scale(std::pow(scaleFactor, steps));

    event->accept();
}

void Scene::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    setupProjectionMatrix();
}

void Scene::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_R) {
        m_object->reset();
    } else if (event->key() == Qt::Key_F) {
        m_cameraMovement = !m_cameraMovement;
    }
}

Object* Scene::getObject() const {
    return m_object;
}

bool Scene::isFaceVisible(const Triangle& triangle, const QMatrix4x4& modelView) const {
    // Get first three vertices of a face
    const QVector<QVector3D>& vertices = m_object->getVertices();

    QVector3D p0 = vertices[triangle.a];
    QVector3D p1 = vertices[triangle.b];
    QVector3D p2 = vertices[triangle.c];

    QVector3D p0_transformed = modelView.map(p0);
    QVector3D p1_transformed = modelView.map(p1);
    QVector3D p2_transformed = modelView.map(p2);

    // Create 2 vectors based on this points
    QVector3D A = p1_transformed - p0_transformed;
    QVector3D B = p2_transformed - p0_transformed;

    // Calculate their cross product to get a normal vector of a surface
    QVector3D normal = QVector3D::crossProduct(A, B).normalized();
    // Since we operate in view space, our camera is at (0,0,0)
    // so view vector should be the point toward the camera
    QVector3D viewVector = -p0_transformed;

    // Since we use counter-clockwise order of faces' vertices,
    // dot product should be positive in order for a face to be visible
    return QVector3D::dotProduct(normal, viewVector) > 0.0f;
}

void Scene::setLightingModel(LightingMode mode) {
    if (m_LightingMode != mode) {
        m_LightingMode = mode;
    }
}

LightSource* Scene::getLightSource() const {
    return m_lightSource;
}

float Scene::getAmbientLightIntensity() const {
    return m_ambientIntensity;
}

void Scene::setAmbientLightIntensity(int value) {
    m_ambientIntensity = value / 100.0f;
}
