#include "Camera.hpp"

Camera::Camera(glm::vec3 position) : 
    m_position(position) 
{
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 direction = glm::normalize(m_position - target);
    m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    m_right = glm::normalize(glm::cross(m_worldUp, direction));
    m_up = glm::cross(direction, m_right);
    m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    
    m_yaw = -90.0f;
    m_pitch = 0.0f;

    m_speed = 3.0f;
    m_sensetivity = 0.1f;
    m_zoom = 45.0f;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::update() {
    glm::vec3 front;
    
    front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front.y = std::sin(glm::radians(m_pitch));
    front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front)); 
}

void Camera::processKeyboard(Direction direction, float delta) {
    float velocity = m_speed * delta;
    
    switch (direction) {
        case Direction::FORWARD:  m_position += m_front * velocity; break;
        case Direction::BACKWARD: m_position -= m_front * velocity; break;
        case Direction::RIGHT:    m_position += m_right * velocity; break;
        case Direction::LEFT:     m_position -= m_right * velocity; break;
        case Direction::UP:       m_position += m_up * velocity; break;
        case Direction::DOWN:     m_position -= m_up * velocity; break;
    }
}

void Camera::processMouseMovement(float xOffset, float yOffset) {
    xOffset *= m_sensetivity;
    yOffset *= m_sensetivity;

    m_yaw   += xOffset;
    m_pitch += yOffset;

    if (m_pitch > 89.0f)  m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;

    update();
}

void Camera::processMouseScroll(float yOffset) {
    m_zoom -= yOffset;

    if (m_zoom < 1.0f)  m_zoom = 1.0f;
    if (m_zoom > 45.0f) m_zoom = 45.0f;
}

float Camera::getZoom() const {
    return m_zoom;
}

glm::vec3 Camera::getPosition() const {
    return m_position;
}