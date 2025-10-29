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
#include "lightsource.h"
#include "camera.h"

using Triangle = Object::Triangle;

class Scene : public QWidget
{
    Q_OBJECT
public:
    explicit Scene(QWidget *parent = nullptr);
    virtual ~Scene();

    enum class LightingMode {
        Simple,
        GouraudShading
    };

public slots:
    void setLightingModel(LightingMode mode);
    void setAmbientLightIntensity(int value);

public:
    // Accessors
    Object* getObject() const;
    LightSource* getLightSource() const;
    float getAmbientLightIntensity() const;

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
    // Setup projection matrix (this implementation uses a perspective projection)
    void setupProjectionMatrix();
    // Draw the X, Y and Z axes
    void renderAxis(QPainter& painter, const QMatrix4x4& view);
    void renderArrow(QPainter& painter, QPointF start, QPointF end, const QColor& color);
    // Project 3D point onto a 2D plane
    QPointF project(const QVector3D& vertex, const QMatrix4x4& modelView) const;
    // Determine if a triangle is facing the camera (back-face culling)
    bool isFaceVisible(const Triangle& triangle, const QMatrix4x4& modelView) const;
    // Render all triangles of the object
    void renderTriangles(QPainter& painter, const QMatrix4x4& modelView);
    // Calculate lighting intensity for a triangle (flat shading)
    float calculateLightIntensity(const Triangle& triangle) const;
    // Calculate lighting intensity for a vertex (used in Gouraud shading)
    float calculateVertexIntensity(const QVector3D& vertexWorld, const QVector3D& normalWorld) const;
    // Apply Gouraud shading to a triangle and draw it into the framebuffer
    void drawTriangleGouraud(QImage& frameBuffer, const Triangle& triangle, const QMatrix4x4& modelView);
    // Fill a triangle using interpolated vertex intensities (helper for Gouraud shading)
    void fillTriangleGouraud(QImage& frameBuffer, const QPointF& p0, float i0, const QPointF& p1, float i1, const QPointF& p2, float i2);

private:
    Object* m_object;                // Pointer to the object
    Camera* m_camera;                // Pointer to the camera
    bool m_cameraMovement;           // Flag indicating whether the camera is currently moving
    QMatrix4x4 m_projectionMatrix;   // Projection matrix
    LightSource* m_lightSource;      // Pointer to the scene’s light source
    float m_ambientIntensity;        // Ambient light intensity for shading calculations

    float m_worldWidth, m_worldHeight, m_worldDepth;
    QPointF m_lastMousePos;
    float m_translationSensetivity;
    float m_rotationSensitivity;

    LightingMode m_LightingMode; // Current lighting mode
};

#endif // SCENE_H
