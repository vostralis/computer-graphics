#include "object.h"

#include <QFile>
#include <QDebug>

#include <cmath>
#include <ctime>

Object::Object() {
    loadObject(filepath);
    calculateCenter();

    m_tx = m_ty = m_tz = m_rx = m_ry = m_rz = 0.0f;
    m_s = 1.5f;

    // Animation stuff
    std::srand(std::time(nullptr));
    m_isAnimationRunning = false;
    m_randomChangeTimer = m_transitionTimer = m_angularVelocity = m_startVelocity = 0.0f;
    m_targetVelocity = 0.05f;
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

        if (parts[0] == 'v' && parts.size() == 4) {
            // Vertex
            m_vertices.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat()));
        } else if (parts[0] == 'e' && parts.size() == 3) {
            // Edge
            m_edges.append(QPair<int32_t, int32_t>(parts[1].toInt() - 1, parts[2].toInt() - 1));
        }
    }

    file.close();
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

    m_isAnimationRunning = false;
    m_randomChangeTimer = 0.0f;
    m_transitionTimer = 0.0f;
    m_angularVelocity = 0.0f;
    m_targetVelocity = m_maxRotationSpeed;

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
    QMatrix4x4 Tc = centeringMatrix();
    QMatrix4x4 Rx = rotationXMatrix();
    QMatrix4x4 Ry = rotationYMatrix();
    QMatrix4x4 Rz = rotationZMatrix();

    return T * (Rz * Ry * Rx * S) * Tc;
}

const QList<QPair<int32_t, int32_t>>& Object::getEdges() const {
    return m_edges;
}

const QList<QVector3D>& Object::getVertices() const {
    return m_vertices;
}

void Object::toggleAnimation() {
    m_isAnimationRunning = !m_isAnimationRunning;
}

void Object::updateAnimation() {
    if (!m_isAnimationRunning) return;

    m_randomChangeTimer += 1.0f;
    // This branch is responsible for random changing the object's rotation direction
    if (m_randomChangeTimer >= m_randomChangeDuration) {
        m_startVelocity = m_angularVelocity;
        m_targetVelocity = (std::rand() % 2 == 0) ? m_maxRotationSpeed : -m_maxRotationSpeed;
        m_randomChangeTimer = 0.0f;
        m_transitionTimer = 0.0f;
    }

    // This branch is responsible for smooth rotation speed changing
    if (m_transitionTimer < m_transitionDuration) {
        if (m_transitionDuration <= 0.0f) {
            m_angularVelocity = m_targetVelocity;
        } else {
            float t = m_transitionTimer / m_transitionDuration;
            float smoothStep = t * t * (3 - 2 * t);
            m_angularVelocity = m_startVelocity + (m_targetVelocity - m_startVelocity) * smoothStep;
            m_transitionTimer += 1.0f;
        }
    } else {
        m_angularVelocity = m_targetVelocity;
    }

    rotateX(m_angularVelocity);
    rotateY(m_angularVelocity * 0.5f);
    emit stateChanged(getWorldCenter(), m_rx, m_ry, m_rz, m_s);
}

bool Object::isAnimationRunning() const {
    return m_isAnimationRunning;
}

void Object::setTransitionDuration(int duration) {
    m_transitionDuration = static_cast<float>(duration);
}

void Object::setRandomChangeDuration(int duration) {
    m_randomChangeDuration = static_cast<float>(duration);
}

void Object::setMaxRotationSpeed(double speed) {
    m_maxRotationSpeed = static_cast<float>(speed);
}

QVector3D Object::getWorldCenter() const {
    QMatrix4x4 model = getModelMatrix();
    QVector4D localCenter(0.0f, 0.0f, 0.0f, 1.0f);
    QVector4D worldCenter = model * localCenter;
    return worldCenter.toVector3D();
}
