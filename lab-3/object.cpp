#include "object.h"

#include <QFile>
#include <QDebug>

#include <cmath>

Object::Object() {
    m_tx = m_ty = m_tz = m_rx = m_ry = m_rz = 0.0f;
    m_s = 1.5f;
    m_resolution = 5;
    m_diffuseCoefficient = 0.8f;
    m_ambientCoefficient = 0.8f;

    loadObject(filepath);
    createRotationalFigure();
    triangulate();
    calculateVertexNormals();
}

Object::~Object() = default;

void Object::loadObject(const QString& path) {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[ERROR]: Couldn't open file " << path;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty()) continue;

        QStringList parts = line.split(',');

        if (parts.size() != 2) {
            throw std::runtime_error("Invalid point coordinates.");
        }

        // qDebug() << parts[0] << ' ' << parts[1];
        m_curvePoints.emplaceBack(parts[0].toFloat(), parts[1].toFloat());
    }

    m_curvePointsNumber = m_curvePoints.size();

    file.close();
}

void Object::createRotationalFigure() {
    m_vertices.clear();
    float step = 2 * M_PI / m_resolution;

    for (size_t i = 0; i < m_curvePointsNumber; ++i) {
        float x_i = m_curvePoints[i].x();
        float y_i = m_curvePoints[i].y();

        for (size_t j = 0; j < m_resolution; ++j) {
            float alpha = step * j;

            float x = x_i * std::cos(alpha);
            float y = y_i;
            float z = x_i * std::sin(alpha);

            m_vertices.emplaceBack(x, y, z);
        }
    }

    centerVertices();
}

void Object::triangulate() {
    m_triangles.clear();
    for (size_t i = 0; i < m_curvePointsNumber - 1; ++i) {
        for (size_t j = 0; j < m_resolution; ++j) {
            size_t curr      = i * m_resolution + j;
            size_t next      = i * m_resolution + (j + 1) % m_resolution;
            size_t above     = (i + 1) * m_resolution + j;
            size_t aboveNext = (i + 1) * m_resolution + (j + 1) % m_resolution;

            m_triangles.emplaceBack(curr, above, aboveNext);
            m_triangles.emplaceBack(curr, aboveNext, next);
        }
    }
}

void Object::calculateVertexNormals() {
    QVector<QVector3D> vertexNormals(m_vertices.size(), QVector3D(0, 0, 0));

    for (const auto& triangle : std::as_const(m_triangles)) {
        QVector3D v0 = m_vertices[triangle.a];
        QVector3D v1 = m_vertices[triangle.b];
        QVector3D v2 = m_vertices[triangle.c];

        QVector3D n = QVector3D::normal(v1 - v0, v2 - v0);
        n.normalize();

        vertexNormals[triangle.a] += n;
        vertexNormals[triangle.b] += n;
        vertexNormals[triangle.c] += n;
    }

    for (QVector3D& n : vertexNormals) {
        n.normalize();
    }

    m_vertexNormals = vertexNormals;
}

const QVector<QVector3D>& Object::getVertexNormals() const {
    return m_vertexNormals;
}

void Object::translate(float tx, float ty, float tz) {
    m_tx += tx;
    m_ty += ty;
    m_tz += tz;
    emit stateChanged(getWorldCenter(), m_rx, m_ry, m_rz, m_s); // Notify UI
}

void Object::scale(float factor) {
    // Limiting a scale factor in range [MIN_SCALE_FACTOR; MAX_SCALE_FACTOR]
    m_s = std::max(std::min(m_s * factor, MAX_SCALE_FACTOR), MIN_SCALE_FACTOR);
    emit stateChanged(getWorldCenter(), m_rx, m_ry, m_rz, m_s);
}

void Object::rotateX(float phi) {
    m_rx += phi;
    emit stateChanged(getWorldCenter(), m_rx, m_ry, m_rz, m_s);
}

void Object::rotateY(float psi) {
    m_ry += psi;
    emit stateChanged(getWorldCenter(), m_rx, m_ry, m_rz, m_s);
}

void Object::rotateZ(float chi) {
    m_rz += chi;
    emit stateChanged(getWorldCenter(), m_rx, m_ry, m_rz, m_s);
}

void Object::reset() {
    m_tx = m_ty = m_tz = 0.0f;
    m_rx = m_ry = m_rz = 0.0f;
    m_s = 1.5f;

    emit stateChanged(getWorldCenter(), m_rx, m_ry, m_rz, m_s);
}

QMatrix4x4 Object::translationMatrix() const {
    return QMatrix4x4(
        1, 0, 0, m_tx,
        0, 1, 0, m_ty,
        0, 0, 1, m_tz,
        0, 0, 0,    1
        );
}

QMatrix4x4 Object::scalingMatrix() const {
    return QMatrix4x4(
        m_s, 0, 0, 0,
        0, m_s, 0, 0,
        0, 0, m_s, 0,
        0, 0,  0,  1
        );
}

QMatrix4x4 Object::rotationXMatrix() const {
    return QMatrix4x4(
        1, 0, 0, 0,
        0, std::cos(m_rx), std::sin(m_rx), 0,
        0, -std::sin(m_rx), std::cos(m_rx), 0,
        0, 0, 0, 1
        );
}

QMatrix4x4 Object::rotationYMatrix() const {
    return QMatrix4x4(
        std::cos(m_ry), 0, -std::sin(m_ry), 0,
        0, 1, 0, 0,
        std::sin(m_ry), 0, std::cos(m_ry), 0,
        0, 0, 0, 1
        );
}

QMatrix4x4 Object::rotationZMatrix() const {
    return QMatrix4x4(
        std::cos(m_rz), std::sin(m_rz), 0, 0,
        -std::sin(m_rz), std::cos(m_rz), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        );
}

void Object::calculateCenter() {
    QVector3D sum(0.0f, 0.0f, 0.0f);

    for (const QVector3D& vertex : std::as_const(m_vertices)) {
        sum += vertex;
    }

    m_centerOffset = sum / m_vertices.size();
}

QMatrix4x4 Object::centeringMatrix() const {
    float tx = -m_centerOffset.x();
    float ty = -m_centerOffset.y();
    float tz = -m_centerOffset.z();

    return QMatrix4x4(
        1, 0, 0, tx,
        0, 1, 0, ty,
        0, 0, 1, tz,
        0, 0, 0,  1
        );
}

QMatrix4x4 Object::getModelMatrix() const {
    // Get each matrix and multiply them in specific order to get a model matrix
    QMatrix4x4 S  = scalingMatrix();
    QMatrix4x4 T  = translationMatrix();
    QMatrix4x4 Rx = rotationXMatrix();
    QMatrix4x4 Ry = rotationYMatrix();
    QMatrix4x4 Rz = rotationZMatrix();

    return T * (Rz * Ry * Rx * S);
}

const QVector<QVector3D>& Object::getVertices() const {
    return m_vertices;
}

const QVector<Object::Triangle>& Object::getTriangles() const {
    return m_triangles;
}

QVector3D Object::getWorldCenter() const {
    QMatrix4x4 model = getModelMatrix();
    QVector4D localCenter(0.0f, 0.0f, 0.0f, 1.0f);
    QVector4D worldCenter = model * localCenter;
    return worldCenter.toVector3D();
}

float Object::getDiffuseCoefficient() const {
    return m_diffuseCoefficient;
}

float Object::getAmbientCoefficient() const {
    return m_ambientCoefficient;
}

void Object::centerVertices() {
    calculateCenter();

    for (QVector3D& v : m_vertices) {
        v -= m_centerOffset;
    }

    m_centerOffset = QVector3D(0, 0, 0);
}

void Object::setResolution(int resolution) {
    m_resolution = resolution;
    createRotationalFigure();
    triangulate();
    calculateVertexNormals();
}

int Object::getResolution() const {
    return m_resolution;
}

void Object::setDiffuseReflectionCoefficient(int value) {
    m_diffuseCoefficient = value / 100.0f;
}

void Object::setAmbientReflectionCoefficient(int value) {
    m_ambientCoefficient = value / 100.0f;
}
