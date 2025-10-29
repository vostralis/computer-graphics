#include "lightsource.h"

LightSource::LightSource() :
    m_x(5), m_y(10), m_z(20), m_intensity(1.0f)
{}

LightSource::~LightSource() = default;

QVector3D LightSource::position() const {
    return QVector3D(m_x, m_y, m_z);
}

float LightSource::intensity() const {
    return m_intensity;
}

void LightSource::setIntensity(int value) {
    m_intensity = value / 100.0f;
}
