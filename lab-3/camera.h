#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>

class Camera
{
public:
    Camera();
    ~Camera();

    // Return view matrix for a current camera's position
    QMatrix4x4 getViewMatrix() const;
    // Set camera's movement trajectory
    void setPath(const QVector3D& start, const QVector3D& end, const QVector3D& target);
    // Update camera's position each frame
    void update(float delta);

private:
    QVector3D m_startPos;   // Start position
    QVector3D m_endPos;     // End position
    QVector3D m_targetPos;  // Target position - where the camera is looking

    QVector3D m_currentPos; // Current position

    float m_progress;       // Progress
    float m_speed;          // Movement speed
    int m_direction;        // Movement direction: 1 - from start to end, -1 - backwards
};

#endif // CAMERA_H
