#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include <QObject>
#include <QVector3D>

class LightSource : public QObject
{
    Q_OBJECT
public:
    LightSource();
    ~LightSource();

    QVector3D position() const;
    float intensity() const;

public slots:
    void setIntensity(int value);

private:
    float m_x, m_y, m_z; // Position
    float m_intensity;   // Light source intensity ∈ [0;1]
};

#endif // LIGHTSOURCE_H
