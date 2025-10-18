#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QVector3D>
#include <QList>
#include <QPair>
#include <QString>
#include <QMatrix4x4>

class Object : public QObject
{
    Q_OBJECT
public:
    Object();
    ~Object();

signals:
    // Signal used for notifying the UI about object's state
    void stateChanged(QVector3D position, float rotationX, float rotationY, float rotationZ, float scale);

public slots:
    void setTransitionDuration(int duration);
    void setRandomChangeDuration(int duration);
    void setMaxRotationSpeed(double speed);

public:
    // Methods that correspond to each affine transformation
    void translate(float tx, float ty, float tz);
    void scale(float factor);
    void rotateX(float phi);
    void rotateY(float psi);
    void rotateZ(float chi);
    // Reset object's state
    void reset();

    // Calculate the model matrix by getting matrices for each affine transformation and multiplying them
    QMatrix4x4 getModelMatrix() const;
    // Accessors
    const QVector<QVector3D>& getVertices() const;
    const QVector<QVector<int32_t>>& getFaces() const;
    // Toggle animation between on and off
    void toggleAnimation();
    // This method is called each animation tick to update object's position
    void updateAnimation();
    // Accessor
    bool isAnimationRunning() const;
    // Calculate the world center's coords
    QVector3D getWorldCenter() const;

private:
    // Loads object from a file
    void loadObject(const QString& path);
    // Calculate the translation matrix
    QMatrix4x4 translationMatrix() const;
    // Calculate the scaling matrix
    QMatrix4x4 scalingMatrix() const;
    // Calculate the rotation matrix (X-axis)
    QMatrix4x4 rotationXMatrix() const;
    // Calculate the rotation matrix (Y-axis)
    QMatrix4x4 rotationYMatrix() const;
    // Calculate the rotation matrix (Z-axis)
    QMatrix4x4 rotationZMatrix() const;
    // Calculate the object's geometrical center after load
    void calculateCenter();
    // Calculate translation matrix based on offset from center
    QMatrix4x4 centeringMatrix() const;

private:
    const QString filepath = "/home/kava/coding/cpp/computer-graphics/lab-2/data.csv"; // Hardcoded
    QVector<QVector3D> m_vertices; // Coordinates of a vertices
    QVector<QVector<int32_t>> m_faces; // Indices of vertices that belong to the same surface

    const float MIN_SCALE_FACTOR = 1.5f;
    const float MAX_SCALE_FACTOR = 7.0f;

    float m_s;              // Scale variable
    float m_tx, m_ty, m_tz; // Translation variables
    float m_rx, m_ry, m_rz; // Rotation variables

    QVector3D m_centerOffset; // Object's offset from center

    // Variables used for animation
    float m_maxRotationSpeed = 0.05f;
    float m_transitionDuration = 60.0f;
    float m_randomChangeDuration = 180.0f;

    bool m_isAnimationRunning;
    float m_randomChangeTimer;
    float m_transitionTimer;
    float m_targetVelocity;
    float m_angularVelocity;
    float m_startVelocity;
    // ------------------------------------
};

#endif // OBJECT_H
