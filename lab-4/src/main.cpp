#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include "Camera.hpp"
#include "Shader.hpp"
#include "StreetMap.hpp"

#include <iostream>


Camera camera(glm::vec3(7, 2, 7));
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool mouseNotMoved = true;
float lastX, lastY;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // Movement only occurs if the cursor is captured
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera.processKeyboard(Camera::Direction::FORWARD, deltaTime);
        } if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera.processKeyboard(Camera::Direction::BACKWARD, deltaTime);
        } if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera.processKeyboard(Camera::Direction::LEFT, deltaTime);
        } if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera.processKeyboard(Camera::Direction::RIGHT, deltaTime);
        } if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            camera.processKeyboard(Camera::Direction::UP, deltaTime);
        } if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            camera.processKeyboard(Camera::Direction::DOWN, deltaTime);
        }
    }
}

int main() {
    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const GLsizei WIDTH = 800;
    const GLsizei HEIGHT = 600;

    // Create window and context
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "meow", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, WIDTH, HEIGHT);

    #pragma region Callbacks

    // Mouse capture callback
    glfwSetMouseButtonCallback(window, []([[maybe_unused]]GLFWwindow *window, int button, int action, [[maybe_unused]]int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            mouseNotMoved = true;
        }
    });

    // Cursor move callback
    glfwSetCursorPosCallback(window, []([[maybe_unused]]GLFWwindow *window, double xPos, double yPos) {
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            return;
        }
        
        if (mouseNotMoved) {
            lastX = xPos;
            lastY = yPos;
            mouseNotMoved = false;
        }

        float xOffset = xPos - lastX;
        float yOffset = lastY - yPos;
        lastX = xPos;
        lastY = yPos;

        camera.processMouseMovement(xOffset, yOffset);
    });

    glfwSetScrollCallback(window, []([[maybe_unused]]GLFWwindow *window, [[maybe_unused]]double xOffset, double yOffset) {
        camera.processMouseScroll(static_cast<float>(yOffset));
    });

    // Window resize callback
    glfwSetFramebufferSizeCallback(window, []([[maybe_unused]]GLFWwindow* window, int width, int height){
        glViewport(0, 0, width, height);
    });

    #pragma endregion
    
    Shader mainShader("./shaders/main.vs", "./shaders/main.fs");
    Shader lampShader("./shaders/lamp.vs", "./shaders/lamp.fs");
    Shader shadowShader("./shaders/shadow.vs", "./shaders/shadow.fs");

    StreetMap street;
    street.init();

    glm::vec4 skyboxColor = glm::vec4(glm::vec3(0.1f), 1.0f);

    double lastTime = glfwGetTime();
    int nbFrames = 0;


    #pragma region ShadowMapping

    // Framebuffer
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // 2D Texture
    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // Fix artifacts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 }; 
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    #pragma endregion


    // Main event loop
    while (!glfwWindowShouldClose(window)) {
        // Update time and fps logic
        nbFrames++;
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (currentFrame - lastTime >= 1.0) {
            printf("\rFPS: %d", nbFrames);
            std::fflush(stdout);
            nbFrames = 0;
            lastTime += 1.0f;
        }

        processInput(window); // Processing user input       

        float near_plane = 1.0f, far_plane = 50.0f;
        glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

        // Look from the Moon's direction
        glm::mat4 lightView = glm::lookAt(glm::vec3(5.0f, 15.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        // Render Depth
        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        // Draw everything that casts shadows
        street.drawLitObjects(shadowShader); 
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);

        // Reset Viewport for normal render
        glViewport(0, 0, currentWidth, currentHeight);


        // Clean screen's color buffer
        glClearColor(skyboxColor.x, skyboxColor.y, skyboxColor.z, skyboxColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mainShader.use();
        mainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        mainShader.setInt("shadowMap", 2);
        mainShader.setVec3("viewPos", camera.getPosition());
        mainShader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        mainShader.setInt("numLights", static_cast<int>(street.m_lampPositions.size()));
        
        // Dynamic aspect ratio
        if (currentHeight == 0) currentHeight = 1;

        mainShader.setMat4("view", camera.getViewMatrix());
        
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.getZoom()), 
            static_cast<float>(currentWidth) / static_cast<float>(currentHeight), // Aspect ratio
            0.1f, 
            100.0f
        );
        mainShader.setMat4("projection", projection);

        street.applyLightningState(mainShader);
        street.drawLitObjects(mainShader);

        lampShader.use();
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", camera.getViewMatrix());

        street.drawEmissives(lampShader);

        // Check and call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << std::endl;
    street.cleanup();
    glfwTerminate();
    return 0;
}