#ifndef SCENE_H
#define SCENE_H

#include <QWidget>
#include <QPainter>
#include <QPen>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QTimer>
#include <QWheelEvent>
#include <QKeyEvent>

#include "object.h"

class Scene : public QWidget
{
    Q_OBJECT
public:
    explicit Scene(QWidget *parent = nullptr);
    virtual ~Scene();

public:
    // Accessor
    Object* getObject() const;
    // Toggle animation by pressing button or space key
    void toggleAnimation();

protected:
    // Events
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // Setup view matrix based on a camera position, eye and center
    void setupViewMatrix();
    // Setup projection matrix (this implementation uses a perspective projection)
    void setupProjectionMatrix();
    // Draw axis X, Y and Z
    void drawAxis(QPainter& painter);
    // Project 3D point onto a 2D plane
    QPointF project(const QVector3D& vertex, const QMatrix4x4& model) const;
    // Method that is called each animation tick to update object's state
    void onAnimationTick();

private:
    Object *object;                // Pointer to an object
    QMatrix4x4 m_viewMatrix;       // View matrix
    QMatrix4x4 m_projectionMatrix; // Projection matrix

    float m_worldWidth, m_worldHeight, m_worldDepth;
    QPointF m_lastMousePos;
    float m_translationSensetivity;
    float m_rotationSensitivity;

    QTimer *animationTimer;
};

#endif // SCENE_H
