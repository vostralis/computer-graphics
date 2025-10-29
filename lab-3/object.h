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
    void setResolution(int resolution);
    void setDiffuseReflectionCoefficient(int value);
    void setAmbientReflectionCoefficient(int value);

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

    // Struct that represents a single triangle of the triangulated object
    struct Triangle {
        Triangle(int32_t a, int32_t b, int32_t c) : a(a), b(b), c(c) {}
        int32_t a, b, c; // Vertices of a triangle
    };

    // Accessors
    const QVector<QVector3D>& getVertices() const;
    const QVector<Triangle>& getTriangles() const;
    const QVector<QVector3D>& getVertexNormals() const;
    int getResolution() const;
    float getDiffuseCoefficient() const;
    float getAmbientCoefficient() const;
    // Calculate the world center's coords
    QVector3D getWorldCenter() const;

private:
    // Loads object from a file
    void loadObject(const QString& path);
    // Fill the vertex array by rotating the initial curve around Y-axis
    void createRotationalFigure();
    // Triangulate the vertex array
    void triangulate();
    // Precalculate normals for each vertex
    void calculateVertexNormals();

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
    // Apply a center offset for each vertex
    void centerVertices();

private:
    const QString filepath = "/home/kava/coding/cpp/computer-graphics/lab-3/data.csv"; // Hardcoded

    QVector<QPointF> m_curvePoints;     // 2D points defining the initial curve in xOY plane
    size_t m_curvePointsNumber;         // Number of curve points
    QVector<QVector3D> m_vertices;      // Generated 3D vertices after rotating the curve around Y-axis
    QVector<QVector3D> m_vertexNormals; // Per-vertex normals for lighting calculations
    int32_t m_resolution;               // Number of subdivisions around the Y-axis
    QVector<Triangle> m_triangles;      // List of mesh triangles
    float m_diffuseCoefficient;         // Diffuse lighting coefficient
    float m_ambientCoefficient;         // Ambient lighting coefficient

    static constexpr float MIN_SCALE_FACTOR = 1.5f;
    static constexpr float MAX_SCALE_FACTOR = 7.0f;

    float m_s;              // Scale variable
    float m_tx, m_ty, m_tz; // Translation variables
    float m_rx, m_ry, m_rz; // Rotation variables

    QVector3D m_centerOffset; // Object's offset from center
};

#endif // OBJECT_H
