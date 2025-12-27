#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f));

    enum class Direction {
        FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN
    };

    glm::mat4 getViewMatrix() const;
    float getZoom() const;
    void processKeyboard(Direction direction, float delta);
    void processMouseMovement(float xOffset, float yOffset);
    void processMouseScroll(float yOffset);
    glm::vec3 getPosition() const;

private:
    void update();

private:
    glm::vec3 m_position;
    glm::vec3 m_right;
    glm::vec3 m_up;
    glm::vec3 m_worldUp;
    glm::vec3 m_front;

    float m_yaw;
    float m_pitch;

    float m_speed;
    float m_sensetivity;
    float m_zoom;
};

#endif