#include "camera.h"
#include <QMatrix4x4>

Camera::Camera() :
    m_progress(0.0f),
    m_speed(0.2f),
    m_direction(1)
{
    setPath(QVector3D(15, 5, 15), QVector3D(20, 5, -5), QVector3D(0, 0, 0));
}

Camera::~Camera() = default;

void Camera::setPath(const QVector3D& start, const QVector3D& end, const QVector3D& target) {
    m_startPos = start;
    m_endPos = end;
    m_targetPos = target;

    // Reset animation state
    m_currentPos = m_startPos;
    m_progress = 0.0f;
    m_direction = 1;
}

QMatrix4x4 Camera::getViewMatrix() const {
    QMatrix4x4 viewMatrix;
    QVector3D up(0.0f, 1.0f, 0.0f);
    viewMatrix.lookAt(m_currentPos, m_targetPos, up);
    return viewMatrix;
}

void Camera::update(float delta) {
    m_progress += m_speed * m_direction * delta;

    // If progress = 1 - then we reach the end point of the path and should start moving in the opposite direction
    if (m_progress >= 1.0f) {
        m_progress = 1.0f;
        m_direction = -1;
    } else if (m_progress <= 0.0f) {
        m_progress = 0.0f;
        m_direction = 1;
    }

    // Interpolating current camera's position (linear)
    m_currentPos = m_startPos * (1 - m_progress) + m_endPos * m_progress;
}
