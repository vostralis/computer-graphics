#include "StreetMap.hpp"

void StreetMap::init() {
    loadTextures();
    
    genBlackTexture();
    initRoad();
    initCurbs();
    initSidewalks();
    initShop1();
    initShop2();
    initShop3();
    initShop4();
    initShop5();
    initBenches();
    initLamps();
}

void StreetMap::loadTextures() {
    m_roadTexture         = loadTexture("./textures/asphalt.png");
    m_curbTexture         = loadTexture("./textures/curb.png");
    m_sidewalkTexture     = loadTexture("./textures/sidewalk.png");
    m_shop1Texture        = loadTexture("./textures/plaster.png");
    m_shop2BaseTexture    = loadTexture("./textures/concrete1.jpg");
    m_shop2RoofTexture    = loadTexture("./textures/roof.jpg");
    m_shop3Texture        = loadTexture("./textures/plaster.png");
    m_shop4BaseTexture    = loadTexture("./textures/concrete2.png");
    m_shop4RoofTexture    = loadTexture("./textures/concrete2.png");
    m_shop5RoofTexture    = loadTexture("./textures/roof2.jpg");
    m_benchWoodTexture    = loadTexture("./textures/wood.jpg");
}

void StreetMap::genBlackTexture() {
    glGenTextures(1, &m_blackTexture);
    glBindTexture(GL_TEXTURE_2D, m_blackTexture);
    unsigned char blackPixel[] = { 0, 0, 0 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, blackPixel);
}

void StreetMap::initRoad() {
    float roadInner = m_roadLength;
    float roadOuter = roadInner + m_roadWidth;
    float uvScale = 0.7f;

    // Calculate tiling
    float uWidth = m_roadWidth * uvScale; 
    float vLength = m_roadLength * uvScale;

    // Vertical road
    addRectangle(m_roadMesh.vertices, 
        roadInner, 0.0f,
        roadOuter, 0.0f,
        roadInner, m_roadLength,
        roadOuter, m_roadLength,
        uWidth,    vLength       
    );
    
    // Intersection 
    addRectangle(m_roadMesh.vertices, 
        roadInner, m_roadLength,
        roadOuter, m_roadLength, 
        roadInner, roadOuter,      
        roadOuter, roadOuter,
        uWidth,    uWidth
    );

    // Horizontal road
    addRectangle(m_roadMesh.vertices,
        0.0f,      roadInner,
        roadInner, roadInner,
        0.0f,      roadOuter,
        roadInner, roadOuter,
        vLength,   uWidth 
    );

    m_roadMesh.setup();
}

void StreetMap::initCurbs() {
    float roadInner = m_roadLength;
    float roadOuter = roadInner + m_roadWidth;
    float uvScale = 2.0f;

    // Calculate tiling
    float uWidth = m_curbWidth * uvScale; 
    float vLength = m_innerSidewalkLength * uvScale;

    // 1. Inner curb
    // Vertical    
    addRectangle(m_innerCurbMesh.vertices,
        m_innerSidewalkLength, 0.0f,                  // Top left
        roadInner,             0.0f,                  // Top right
        m_innerSidewalkLength, m_innerSidewalkLength, // Bottom left
        roadInner,           m_innerSidewalkLength,   // Bottom right
        m_curbWidth * uvScale, m_innerSidewalkLength * uvScale
    );

    // Intersection
    addRectangle(m_innerCurbMesh.vertices,
        m_innerSidewalkLength, m_innerSidewalkLength, // Top left
        roadInner,             m_innerSidewalkLength, // Top right
        m_innerSidewalkLength, roadInner,             // Bottom left
        roadInner,             roadInner,             // Bottom right
        m_curbWidth * uvScale, m_curbWidth * uvScale  // UV
    );

    // Vertical
    addRectangle(m_innerCurbMesh.vertices,
        0.0f,                   m_innerSidewalkLength, // Top left
        m_innerSidewalkLength,  m_innerSidewalkLength, // Top Right
        0.0f,                   roadInner,             // Bottom left
        m_innerSidewalkLength,  roadInner,             // Bottom Right
        m_innerSidewalkLength * uvScale, m_curbWidth * uvScale // UV
    );

    // Outer - connected to the road, inner - connected to the sidewalk

    // Vertical outer curb wall
    addWall(m_innerCurbMesh.vertices, 
        roadInner, 0.0f,      // Top
        roadInner, roadInner, // Bottom
        0.15f,
        roadInner * uvScale,
        true
    );

    // Horizontal outer curb wall
    addWall(m_innerCurbMesh.vertices,
        0.0f,      roadInner, // Left
        roadInner, roadInner, // Right
        0.15f, 
        roadInner * uvScale,
        false
    );

    // Vertical inner curb wall
    addWall(m_innerCurbMesh.vertices,
        m_innerSidewalkLength, 0.0f,                // Top
        m_innerSidewalkLength, m_innerSidewalkLength, // Bottom
        0.05f,
        m_innerSidewalkLength * uvScale,
        false
    );

    // Horizontal inner curb wall
    addWall(m_innerCurbMesh.vertices, 
        0.0f,                m_innerSidewalkLength, // Left
        m_innerSidewalkLength, m_innerSidewalkLength, // Right
        0.05f,
        m_innerSidewalkLength * uvScale,
        true
    );

    m_innerCurbMesh.setup();
    
    // 2. Outer curb
    // Vertical
    addRectangle(m_outerCurbMesh.vertices,
        roadOuter,                   0.0f,      // Top left
        roadOuter + m_curbWidth, 0.0f,      // Top right
        roadOuter,                   roadOuter, // Bottom left
        roadOuter + m_curbWidth, roadOuter, // Bottom right
        m_curbWidth * uvScale, roadOuter * uvScale
    );

    // Intersection
    addRectangle(m_outerCurbMesh.vertices,
        roadOuter,                   roadOuter,                   // Top left
        roadOuter + m_curbWidth, roadOuter,                   // Top right
        roadOuter,                   roadOuter + m_curbWidth, // Bottom left
        roadOuter + m_curbWidth, roadOuter + m_curbWidth, // BottomRight
        m_curbWidth * uvScale, m_curbWidth * uvScale
    );

    // Horizontal
    addRectangle(m_outerCurbMesh.vertices,
        0.0f,      roadOuter,                   // Top left
        roadOuter, roadOuter,                   // Top right
        0.0f,      roadOuter + m_curbWidth, // Bottom left
        roadOuter, roadOuter + m_curbWidth, // Bottom right
        roadOuter * uvScale, m_curbWidth * uvScale
    );
    
    // Vertical outer curb wall
    addWall(m_outerCurbMesh.vertices,
        roadOuter, 0.0f,
        roadOuter, roadOuter,
        0.15f,
        roadOuter * uvScale,
        false
    );

    // Horizontal outer curb wall
    addWall(m_outerCurbMesh.vertices, 
        0.0f, roadOuter,
        roadOuter, roadOuter,
        0.15f,
        roadOuter * uvScale,
        true
    );

    // Vertical inner curb wall
    addWall(m_outerCurbMesh.vertices, 
        roadOuter + m_curbWidth, 0.0f,
        roadOuter + m_curbWidth, roadOuter + m_curbWidth,
        0.05f,
        (roadOuter + m_curbWidth) * uvScale,
        true
    );

    // Horizontal inner curb wall
    addWall(m_outerCurbMesh.vertices, 
        0.0f, roadOuter + m_curbWidth,
        roadOuter + m_curbWidth, roadOuter + m_curbWidth,
        0.05f,
        (roadOuter + m_curbWidth) * uvScale,
        false
    );

    m_outerCurbMesh.setup();
}

void StreetMap::initSidewalks() {
    float uvScale = 0.8f;

    // 1. Inner sidewalk
    addRectangle(m_innerSidewalkMesh.vertices,
        0.0f,                0.0f,
        m_innerSidewalkLength, 0.0f,
        0.0f,                m_innerSidewalkLength,
        m_innerSidewalkLength, m_innerSidewalkLength,
        m_innerSidewalkLength * uvScale, m_innerSidewalkLength * uvScale 
    );    

    m_innerSidewalkMesh.setup();

    // 2. Outer sidewalk
    addRectangle(m_outerSidewalkMesh.vertices,
        m_outerVerticalSidewalkLength,                                0.0f,                                   // Top left
        m_outerVerticalSidewalkLength + m_outerVerticalSidewalkWidth, 0.0f,                                   // Top right
        m_outerVerticalSidewalkLength,                                m_outerVerticalSidewalkLength,          // Bottom left
        m_outerVerticalSidewalkLength + m_outerVerticalSidewalkWidth, m_outerVerticalSidewalkLength,          // Bottom right
        m_outerVerticalSidewalkWidth * uvScale,                       m_outerVerticalSidewalkLength * uvScale
    );

    addRectangle(m_outerSidewalkMesh.vertices, 
        m_outerVerticalSidewalkLength,                                m_outerVerticalSidewalkLength,
        m_outerVerticalSidewalkLength + m_outerVerticalSidewalkWidth, m_outerVerticalSidewalkLength,
        m_outerVerticalSidewalkLength,                                m_outerVerticalSidewalkLength + m_outerHorizontalSidewalkWidth,
        m_outerVerticalSidewalkLength + m_outerVerticalSidewalkWidth, m_outerVerticalSidewalkLength + m_outerHorizontalSidewalkWidth,
        m_outerVerticalSidewalkWidth * uvScale,                       m_outerHorizontalSidewalkWidth * uvScale
    );

    addRectangle(m_outerSidewalkMesh.vertices,
        0.0f,                                    m_outerVerticalSidewalkLength,
        m_outerVerticalSidewalkLength,           m_outerVerticalSidewalkLength,
        0.0f,                                    m_outerVerticalSidewalkLength + m_outerHorizontalSidewalkWidth,
        m_outerVerticalSidewalkLength,           m_outerVerticalSidewalkLength + m_outerHorizontalSidewalkWidth,
        m_outerVerticalSidewalkLength * uvScale, m_outerHorizontalSidewalkWidth * uvScale                       
    );

    m_outerSidewalkMesh.setup();
}

void StreetMap::initShop1() {
    float w = 2.5f;
    float h = 2.0f;
    float d = 2.5f;
    float uvScale = 1.5f;

    float x = m_roadOuter - 1.5f;
    float z = m_roadOuter + 2.5f;
    
    float overhang = 0.2f;

    // --- Base
    addCube(m_shop1BaseMesh.vertices, x, 0.0001f, z, w, h, d, uvScale);
    m_shop1BaseMesh.setup();
    
    // --- Roof
    addCube(m_shop1RoofMesh.vertices,
        x - overhang, 
        h, 
        z - overhang, 
        w + (overhang * 2), 
        0.2f, 
        d + (overhang * 2), 
        uvScale
    );
    
    m_shop1RoofMesh.setup();

    // --- Entrance
    addEntrance(m_shop1EntranceMesh.vertices, x, z, w, uvScale);
    m_shop1EntranceMesh.setup();
}

void StreetMap::initShop2() {
    float uvScale = 0.7f;
    float w = 2.5f;
    float h = 2.0f;
    float d = 2.5f;
    float roofHeight = 1.0f;

    float x = m_roadOuter - 5.0f;
    float z = m_roadOuter + 2.5f;

    // --- Base
    addCube(m_shop2BaseMesh.vertices, x, 0.0001f, z, w, h, d, uvScale);
    m_shop2BaseMesh.setup();

    float overhang = 0.3f;

    // --- Roof
    addPrism(m_shop2RoofMesh.vertices, 
        x - overhang,
        h,
        z - overhang,
        w + (overhang * 2),
        roofHeight,
        d + (overhang * 2),
        uvScale
    );

    m_shop2RoofMesh.setup();

    // --- Entrance
    addEntrance(m_shop2EntranceMesh.vertices, x, z, w, uvScale);
    m_shop2EntranceMesh.setup();
}

void StreetMap::initShop3() {
    float uvScale = 0.7;
    float shopWidth = 2.5f;
    float h = 2.0f;
    float d = 2.5f;

    // --- Awning
    addStripedAwning(
        m_awningEvenMesh.vertices, 
        m_awningOddMesh.vertices, 
        m_roadOuter - 8.5f,
        2.5f, 
        m_roadOuter + 2.5f, 
        shopWidth,
        0.8f, 
        0.3f,
        8
    );

    m_awningEvenMesh.setup();
    m_awningOddMesh.setup();

    float x = m_roadOuter - 8.5f;
    float z = m_roadOuter + 2.5f;

    // --- Base
    addCube(m_shop3BaseMesh.vertices,
        m_roadOuter - 8.5f,
        0.0001f,
        m_roadOuter + 2.5f,
        shopWidth, 
        h, 
        d, 
        uvScale
    );

    m_shop3BaseMesh.setup();

    // --- Entrance
    addEntrance(m_shop3EntranceMesh.vertices, x, z, h, uvScale);
    m_shop3EntranceMesh.setup();

    float winW = 1.5f;
    float winH = 1.5f;
    float winY = 1.0f; // Height from ground
    
    // Z position: Shop Front Z + slight offset (0.05)
    float winZ = z + d + 0.05f; 
    float winX = x + 2.5f; // Adjust to fit your shop layout

    // Add geometry to the GLOWING mesh
    addCube(m_glowingWindowMesh.vertices, 
        winX, winY, winZ, 
        winW, winH, 0.1f, // Thin glass pane
        1.0f
    );

    m_glowingWindowMesh.setup();
}

void StreetMap::initShop4() {
    float x = 0.0f;
    float y = 0.01f;
    float z = 0.0f;
    float w = 2.0f;
    float h = 2.0f;
    float d = 2.0f;
    float uvScale = 0.7f;
    
    // --- Base
    addCube(m_shop4BaseMesh.vertices, x, y, z, w, h, d, 0.35f);
    m_shop4BaseMesh.setup();

    // --- Roof
    float overhang = 0.2f;
    float roofH = 0.2f;

    addCube(m_shop4RoofMesh.vertices,
        x - overhang,
        y + h,
        z - overhang,
        w + (overhang * 2.0f),
        roofH,
        d + (overhang * 2.0f),
        uvScale
    );

    m_shop4RoofMesh.setup();

    // --- Entrance
    addEntrance(m_shop4EntranceMesh.vertices, x, z, h, uvScale);
    m_shop4EntranceMesh.setup();
}

void StreetMap::initShop5() {
    float x = 0.0f;
    float y = 0.01f;
    float z = 0.0f;
    
    float w = 2.0f;
    float h = 2.0f;
    float d = 2.0f;
    
    float roofH = 1.0f;
    float uvScale = 0.7f;

    // --- Base
    addCube(m_shop5BaseMesh.vertices, x, y, z, w, h, d, uvScale);
    m_shop5BaseMesh.setup();

    // --- Roof
    float overhang = 0.2f;

    addPrism(m_shop5RoofMesh.vertices, 
        x - overhang,
        y + h,
        z - overhang,
        w + (overhang * 2.0f),
        roofH,  
        d + (overhang * 2.0f),
        uvScale
    );

    m_shop5RoofMesh.setup();

    // --- Entrance
    addEntrance(m_shop5EntranceMesh.vertices, x, z, h, uvScale);
    m_shop5EntranceMesh.setup();
}

void StreetMap::addEntrance(std::vector<float>& v, float x, float z, float h, float uvScale) {
    float columnWidth = 0.1f;
    float columnHeight = h * 0.66f;
    float columnDepth = 0.05f;
    float columnX = x + 1.2f;
    float columnZ = z - columnDepth;

    // Left column
    addCube(v, 
        columnX, 
        0.0001f, 
        columnZ, 
        columnWidth, 
        columnHeight, 
        columnDepth, 
        uvScale
    );

    // Right column
    addCube(v, 
        columnX - 0.6f, 
        0.0001f, 
        columnZ, 
        columnWidth, 
        columnHeight, 
        columnDepth, 
        uvScale
    );

    // Header
    addCube(v, 
        columnX - 0.6f,
        columnHeight,
        columnZ,
        columnWidth + 0.6f,
        0.1f,
        columnDepth,
        uvScale
    );

    // 'Door'
    addCube(v,
        columnX - 0.5f,
        0.0001f,
        columnZ + columnDepth - 0.03f,
        columnWidth * 5,
        columnHeight,
        0.01f,
        uvScale
    );
}

void StreetMap::drawLitObjects(Shader& shader) {
    glm::mat4 model = glm::mat4(1.0f);

    // Matte surfaces (specular = 0)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_blackTexture); 
    shader.setInt("material.specular", 1);
    shader.setFloat("material.shininess", 4.0f);

    // Draw road
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_roadTexture);
    shader.setInt("material.diffuse", 0);
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setMat4("model", model);
    m_roadMesh.draw();

    // Draw curbs
    glBindTexture(GL_TEXTURE_2D, m_curbTexture);
    model = glm::translate(model, glm::vec3(0.0f, 0.15f, 0.0f)); // Raise a bit
    shader.setMat4("model", model);
    shader.setVec3("objectColor", glm::vec3(0.7f, 0.7f, 0.69f));
    m_innerCurbMesh.draw();    
    m_outerCurbMesh.draw();

    // Draw sidewalks
    glBindTexture(GL_TEXTURE_2D, m_sidewalkTexture);
    model = glm::translate(model, glm::vec3(0.0f, -0.05f, 0.0f));
    shader.setMat4("model", model);
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_innerSidewalkMesh.draw();
    m_outerSidewalkMesh.draw();

    // -- Buildings
    shader.setFloat("material.shininess", 16.0f);

    // Shop 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_shop1Texture);
    shader.setVec3("objectColor", glm::vec3(0.80f, 0.45f, 0.40f));
    m_shop1BaseMesh.draw();
    shader.setVec3("objectColor", glm::vec3(0.50f, 0.20f, 0.15f));
    m_shop1RoofMesh.draw();
    m_shop1EntranceMesh.draw();

    // Shop 2
    glBindTexture(GL_TEXTURE_2D, m_shop2BaseTexture);
    shader.setVec3("objectColor", glm::vec3(0.75f, 0.55f, 0.35f));
    m_shop2BaseMesh.draw();
    shader.setVec3("objectColor", glm::vec3(0.55f, 0.35f, 0.15f));
    m_shop2EntranceMesh.draw();
    glBindTexture(GL_TEXTURE_2D, m_shop2RoofTexture);
    shader.setVec3("objectColor", glm::vec3(1.0f));
    m_shop2RoofMesh.draw();
    
    // Shop 3
    glBindTexture(GL_TEXTURE_2D, m_shop3Texture);
    shader.setVec3("objectColor", glm::vec3(0.57f, 0.42f, 0.01f));
    m_shop3BaseMesh.draw();
    shader.setVec3("objectColor", glm::vec3(0.36f, 0.31f, 0.31f)); 
    m_shop3EntranceMesh.draw();
    
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-3.5f, -0.55f, -23.0f));
    shader.setMat4("model", model);
    m_awningEvenMesh.draw();
    shader.setVec3("objectColor", glm::vec3(0.95f, 0.95f, 0.95f));
    m_awningOddMesh.draw();

    // Shop 4
    glBindTexture(GL_TEXTURE_2D, m_shop4BaseTexture);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.1f, 0.5f));
    shader.setVec3("objectColor", glm::vec3(0.43f, 0.53f, 0.62f));
    shader.setMat4("model", model);
    m_shop4BaseMesh.draw();
    shader.setVec3("objectColor", glm::vec3(0.33f, 0.43f, 0.52f));
    model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.5f, 0.0f, -2.5f));
    shader.setMat4("model", model);
    m_shop4RoofMesh.draw();
    m_shop4EntranceMesh.draw();

    // Shop 5
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 3.0f));
    shader.setMat4("model", model);
    shader.setVec3("objectColor", glm::vec3(0.42f, 0.43f, 0.42f));
    m_shop5BaseMesh.draw();
    glBindTexture(GL_TEXTURE_2D, m_shop5RoofTexture);
    model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(3.0f, 0.0f, -2.5f));
    shader.setMat4("model", model);
    m_shop5RoofMesh.draw();
    m_shop5EntranceMesh.draw();

    // -- Metal
    shader.setFloat("material.shininess", 64.0f);
    glActiveTexture(GL_TEXTURE0); // Switch back to diffuse unit
    shader.setVec3("objectColor", glm::vec3(0.2f, 0.2f, 0.2f)); // Dark Grey

    // Lamp poles
    for (const auto& pos : m_lampPositions) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos); 
        shader.setMat4("model", model);
        
        m_lampPoleMesh.draw();
    }

    // Metal bench legs
    for (const auto& bench : m_benches) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, bench.position);
        model = glm::rotate(model, glm::radians(bench.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", model);

        shader.setVec3("objectColor", glm::vec3(0.3f, 0.3f, 0.35f)); 
        m_benchMetalMesh.draw();
    }

    // Matte surfaces (specular = 0)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_blackTexture); 
    shader.setInt("material.specular", 1);
    shader.setFloat("material.shininess", 4.0f);

    // Draw benches
    glBindTexture(GL_TEXTURE_2D, m_benchWoodTexture);
    for (const auto& bench : m_benches) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, bench.position);
        model = glm::rotate(model, glm::radians(bench.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", model);

        // Wood parts
        shader.setVec3("objectColor", glm::vec3(0.6f, 0.4f, 0.2f));
        m_benchWoodMesh.draw();
    }
}

void StreetMap::drawEmissives(Shader& shader) {
    shader.setVec3("lightColor", glm::vec3(1.0f, 0.9f, 0.7f)); 

    for (size_t i = 0; i < m_lampPositions.size(); ++i) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_lampPositions[i]);
        shader.setMat4("model", model);
        m_lampBulbMesh.draw();
    }
}

void StreetMap::applyLightningState(Shader& shader) {
    shader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f)); 
    shader.setVec3("dirLight.ambient",  glm::vec3(0.03f, 0.03f, 0.07f));
    shader.setVec3("dirLight.diffuse",  glm::vec3(0.15f, 0.15f, 0.2f));
    shader.setVec3("dirLight.specular", glm::vec3(0.1f, 0.1f, 0.1f));

    for (int i = 0; i < m_lampPositions.size(); ++i) {
        std::string index = std::to_string(i);
        glm::vec3 bulbOffset(0.0f, 2.2f, 0.0f); // Bulb height in model space
        shader.setVec3("pointLights[" + index + "].position", m_lampPositions[i] + bulbOffset);
        
        shader.setVec3("pointLights[" + index + "].ambient",  glm::vec3(0.07f, 0.07f, 0.07f));
        shader.setVec3("pointLights[" + index + "].diffuse",  glm::vec3(0.8f)); 
        shader.setVec3("pointLights[" + index + "].specular", glm::vec3(0.2f));

        shader.setFloat("pointLights[" + index + "].constant", 1.0f);
        shader.setFloat("pointLights[" + index + "].linear", 0.07f);
        shader.setFloat("pointLights[" + index + "].quadratic", 0.08f);
    }
}

void StreetMap::addRectangle(std::vector<float>& vec, float x1, float z1, float x2, float z2, float x3, float z3, float x4, float z4, float uMax, float vMax) {
    auto push = [&](float x, float y, float z, float u, float v) {
        vec.push_back(x); vec.push_back(y); vec.push_back(z);
        vec.push_back(0.0f); vec.push_back(1.0f); vec.push_back(0.0f);
        vec.push_back(u); vec.push_back(v);
    };

    // Triangle 1
    push(x1, 0.0f, z1, 0.0f, 0.0f); // 1. Start Inner
    push(x2, 0.0f, z2, uMax, 0.0f); // 2. Start Outer
    push(x3, 0.0f, z3, 0.0f, vMax); // 3. End inner

    // Triangle 2
    push(x2, 0.0f, z2, uMax, 0.0f); // 1. Start Outer
    push(x4, 0.0f, z4, uMax, vMax); // 2. End outer
    push(x3, 0.0f, z3, 0.0f, vMax); // 3. End inner
}

void StreetMap::addWall(std::vector<float>& v, float x1, float z1, float x2, float z2, float height, float vScale, bool flipNormal) {
    // Calculate Wall Direction vector
    float dx = x2 - x1;
    float dz = z2 - z1;
    float len = std::sqrt(dx*dx + dz*dz);

    // Calculate Normal Vector
    float nx = -dz / len;
    float ny = 0.0f;
    float nz = dx / len;

    if (flipNormal) {
        nx = -nx;
        nz = -nz;
    }

    auto push = [&](float px, float py, float pz, float u, float tv) {
        v.push_back(px); v.push_back(py); v.push_back(pz);
        v.push_back(nx); v.push_back(ny); v.push_back(nz);
        v.push_back(u);  v.push_back(tv);
    };

    // Triangle 1
    push(x1, 0.0f,    z1, 0.0f,   0.0f); // Bottom 1
    push(x1, -height, z1, 0.0f,   1.0f); // Top 1
    push(x2, 0.0f,    z2, vScale, 0.0f); // Bottom 2

    // Triangle 2
    push(x1, -height, z1, 0.0f,   1.0f); // Top 1
    push(x2, -height, z2, vScale, 1.0f); // Top 2
    push(x2, 0.0f,    z2, vScale, 0.0f); // Bottom 2
}

void StreetMap::cleanup() {
    m_roadMesh.destroy();
    m_innerCurbMesh.destroy();
    m_outerCurbMesh.destroy();
    m_innerSidewalkMesh.destroy();
    m_outerSidewalkMesh.destroy();
    m_shop1BaseMesh.destroy();
    m_shop1RoofMesh.destroy();
    m_shop1EntranceMesh.destroy();
    m_shop2BaseMesh.destroy();
    m_shop2RoofMesh.destroy();
    m_shop2EntranceMesh.destroy();
    m_shop3BaseMesh.destroy();
    m_shop3EntranceMesh.destroy();
    m_awningEvenMesh.destroy();
    m_awningOddMesh.destroy();
    m_glowingWindowMesh.destroy();
    m_shop4BaseMesh.destroy();
    m_shop4RoofMesh.destroy();
    m_shop4EntranceMesh.destroy();
    m_shop5BaseMesh.destroy();
    m_shop5RoofMesh.destroy();
    m_shop5EntranceMesh.destroy();
    m_benchWoodMesh.destroy();
    m_benchMetalMesh.destroy();
    m_lampPoleMesh.destroy();
    m_lampBulbMesh.destroy();
    m_lampMesh.destroy();
}

GLuint StreetMap::loadTexture(const char *path) {
    unsigned textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Filtering ensures it doesn't look pixelated at distance
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void StreetMap::addCube(std::vector<float>& v, float x, float y, float z, float w, float h, float d, float uvScale) {
    auto push = [&](float px, float py, float pz, float nx, float ny, float nz, float u, float tv) {
        v.push_back(px); v.push_back(py); v.push_back(pz); // Position
        v.push_back(nx); v.push_back(ny); v.push_back(nz); // Normal
        v.push_back(u);  v.push_back(tv);                  // Texture
    };

    float uW = w * uvScale;
    float vH = h * uvScale;
    float uD = d * uvScale;
    float vD = d * uvScale;

    // Front face
    push(x,   y,   z+d,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f);
    push(x+w, y,   z+d,  0.0f, 0.0f, 1.0f,  uW,   0.0f);
    push(x,   y+h, z+d,  0.0f, 0.0f, 1.0f,  0.0f, vH);
    
    push(x+w, y,   z+d,  0.0f, 0.0f, 1.0f,  uW,   0.0f);
    push(x+w, y+h, z+d,  0.0f, 0.0f, 1.0f,  uW,   vH);
    push(x,   y+h, z+d,  0.0f, 0.0f, 1.0f,  0.0f, vH);

    // Back face
    push(x+w, y,   z,    0.0f, 0.0f, -1.0f, 0.0f, 0.0f); 
    push(x,   y,   z,    0.0f, 0.0f, -1.0f, uW,   0.0f);
    push(x+w, y+h, z,    0.0f, 0.0f, -1.0f, 0.0f, vH);
    
    push(x,   y,   z,    0.0f, 0.0f, -1.0f, uW,   0.0f);
    push(x,   y+h, z,    0.0f, 0.0f, -1.0f, uW,   vH);
    push(x+w, y+h, z,    0.0f, 0.0f, -1.0f, 0.0f, vH);

    // Left face
    push(x,   y,   z,   -1.0f, 0.0f, 0.0f,  0.0f, 0.0f);
    push(x,   y,   z+d, -1.0f, 0.0f, 0.0f,  uD,   0.0f);
    push(x,   y+h, z,   -1.0f, 0.0f, 0.0f,  0.0f, vH);
    
    push(x,   y,   z+d, -1.0f, 0.0f, 0.0f,  uD,   0.0f);
    push(x,   y+h, z+d, -1.0f, 0.0f, 0.0f,  uD,   vH);
    push(x,   y+h, z,   -1.0f, 0.0f, 0.0f,  0.0f, vH);

    // Right face
    push(x+w, y,   z+d,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f);
    push(x+w, y,   z,    1.0f, 0.0f, 0.0f,  uD,   0.0f);
    push(x+w, y+h, z+d,  1.0f, 0.0f, 0.0f,  0.0f, vH);

    push(x+w, y,   z,    1.0f, 0.0f, 0.0f,  uD,   0.0f);
    push(x+w, y+h, z,    1.0f, 0.0f, 0.0f,  uD,   vH);
    push(x+w, y+h, z+d,  1.0f, 0.0f, 0.0f,  0.0f, vH);

    // Top face
    push(x,   y+h, z+d,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f);
    push(x+w, y+h, z+d,  0.0f, 1.0f, 0.0f,  uW,   0.0f);
    push(x,   y+h, z,    0.0f, 1.0f, 0.0f,  0.0f, vD);

    push(x+w, y+h, z+d,  0.0f, 1.0f, 0.0f,  uW,   0.0f);
    push(x+w, y+h, z,    0.0f, 1.0f, 0.0f,  uW,   vD);
    push(x,   y+h, z,    0.0f, 1.0f, 0.0f,  0.0f, vD);

    // Bottom face
    push(x,   y,   z,    0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
    push(x+w, y,   z,    0.0f, -1.0f, 0.0f, uW,   0.0f);
    push(x,   y,   z+d,  0.0f, -1.0f, 0.0f, 0.0f, vD);

    push(x+w, y,   z,    0.0f, -1.0f, 0.0f, uW,   0.0f);
    push(x+w, y,   z+d,  0.0f, -1.0f, 0.0f, uW,   vD);
    push(x,   y,   z+d,  0.0f, -1.0f, 0.0f, 0.0f, vD);
}

void StreetMap::addPrism(std::vector<float>& v, float x, float y, float z, float w, float h, float d, float uvScale) {
    float halfW = w * 0.5f;
    float peakX = x + halfW;
    float peakY = y + h;

    float slopeLen = std::sqrt(halfW * halfW + h * h);
    
    // Left Slope Normal: Points Left and Up
    float nLx = -h / slopeLen;
    float nLy = halfW / slopeLen;
    float nLz = 0.0f;

    // Right Slope Normal: Points Right and Up
    float nRx = h / slopeLen;
    float nRy = halfW / slopeLen;
    float nRz = 0.0f;

    float uW = w * uvScale;
    float uD = d * uvScale;
    float vH = h * uvScale;
    float vSlope = slopeLen * uvScale;

    // Helper to push vertex
    auto push = [&](float px, float py, float pz, float nx, float ny, float nz, float u, float tv) {
        v.push_back(px); v.push_back(py); v.push_back(pz);
        v.push_back(nx); v.push_back(ny); v.push_back(nz);
        v.push_back(u);  v.push_back(tv);
    };

    // --- Front face
    push(x,      y,     z+d,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f);
    push(x+w,    y,     z+d,  0.0f, 0.0f, 1.0f,  uW,   0.0f);
    push(peakX,  peakY, z+d,  0.0f, 0.0f, 1.0f,  uW*0.5f, vH);

    // --- Back face
    push(x+w,    y,     z,    0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    push(x,      y,     z,    0.0f, 0.0f, -1.0f, uW,   0.0f);
    push(peakX,  peakY, z,    0.0f, 0.0f, -1.0f, uW*0.5f, vH);

    // --- Left slope
    // Triangle 1
    push(x,      y,     z+d,  nLx, nLy, nLz,     0.0f, 0.0f);
    push(x,      y,     z,    nLx, nLy, nLz,     uD,   0.0f);
    push(peakX,  peakY, z,    nLx, nLy, nLz,     uD,   vSlope);

    // Triangle 2
    push(peakX,  peakY, z,    nLx, nLy, nLz,     uD,   vSlope);
    push(peakX,  peakY, z+d,  nLx, nLy, nLz,     0.0f, vSlope);
    push(x,      y,     z+d,  nLx, nLy, nLz,     0.0f, 0.0f);

    // --- Right slope
    // Triangle 1
    push(x+w,    y,     z+d,  nRx, nRy, nRz,     0.0f, 0.0f);
    push(peakX,  peakY, z+d,  nRx, nRy, nRz,     0.0f, vSlope);
    push(peakX,  peakY, z,    nRx, nRy, nRz,     uD,   vSlope);

    // Triangle 2
    push(peakX,  peakY, z,    nRx, nRy, nRz,     uD,   vSlope);
    push(x+w,    y,     z,    nRx, nRy, nRz,     uD,   0.0f);
    push(x+w,    y,     z+d,  nRx, nRy, nRz,     0.0f, 0.0f);

    // --- Bottom face
    push(x,      y,     z,    0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
    push(x+w,    y,     z,    0.0f, -1.0f, 0.0f, uW,   0.0f);
    push(x,      y,     z+d,  0.0f, -1.0f, 0.0f, 0.0f, uD);

    push(x+w,    y,     z,    0.0f, -1.0f, 0.0f, uW,   0.0f);
    push(x+w,    y,     z+d,  0.0f, -1.0f, 0.0f, uW,   uD);
    push(x,      y,     z+d,  0.0f, -1.0f, 0.0f, 0.0f, uD);
}

void StreetMap::initBenchModel() {
    float width = 1.6f;
    float plankThick = 0.05f;
    float plankDepth = 0.1f;
    float legWidth = 0.1f;
    
    // --- Metal frame
    // Left Leg
    addCube(m_benchMetalMesh.vertices, -0.6f, 0.0f, 0.0f, legWidth, 0.45f, 0.5f, 1.0f);
    // Right Leg
    addCube(m_benchMetalMesh.vertices, 0.6f, 0.0f, 0.0f, legWidth, 0.45f, 0.5f, 1.0f);
    
    // Back Supports
    // Left
    addCube(m_benchMetalMesh.vertices, -0.6f, 0.45f, 0.0f, legWidth, 0.5f, 0.05f, 1.0f);
    // Right
    addCube(m_benchMetalMesh.vertices, 0.6f, 0.45f, 0.0f, legWidth, 0.5f, 0.05f, 1.0f);


    // --- Wood planks
    float startX = -(width - legWidth) / 2.0f; // Center the planks on X

    // Seat
    for(int i = 0; i < 3; i++) {
        float zOffset = 0.1f + (i * (plankDepth + 0.02f));
        addCube(m_benchWoodMesh.vertices, startX, 0.45f, zOffset, width, plankThick, plankDepth, 1.0f);
    }

    // Backrest
    for(int i = 0; i < 3; i++) {
        float yOffset = 0.6f + (i * (plankDepth + 0.02f));
        addCube(m_benchWoodMesh.vertices, startX, yOffset, plankThick, width, plankDepth, plankThick, 1.0f);
    }

    m_benchMetalMesh.setup();
    m_benchWoodMesh.setup();
}

void StreetMap::initBenches() {
    initBenchModel();

    m_benches.clear();
    m_benches.push_back({ glm::vec3(10.5f, 0.10f, 1.5f), -90.0f });
    m_benches.push_back({ glm::vec3(10.5f, 0.10f, 6.0f), -90.0f });
    m_benches.push_back({ glm::vec3(4.5f, 0.10f, 1.5f), 90.0f });
    m_benches.push_back({ glm::vec3(4.5f, 0.10f, 4.0f), 90.0f });
}

void StreetMap::addStripedAwning(
    std::vector<float>& vEven, // Target for Color A
    std::vector<float>& vOdd,  // Target for Color B
    float x, float y, float z, 
    float width, float depth, float drop, 
    int numStripes) 
{
    float stripeWidth = width / numStripes;
    float flapHeight = 0.15f; // The little bit hanging down vertically at the front

    // Calculate Slope Normal
    float slopeLen = sqrt(drop*drop + depth*depth);
    float ny = depth / slopeLen;
    float nz = drop / slopeLen;

    auto push = [&](std::vector<float>& v, float px, float py, float pz, float nx, float ny, float nz) {
        v.push_back(px); v.push_back(py); v.push_back(pz);
        v.push_back(nx); v.push_back(ny); v.push_back(nz);
        v.push_back(0.0f); v.push_back(0.0f);
    };

    for (int i = 0; i < numStripes; ++i) {
        // Decide which vector to write to
        std::vector<float>& target = (i % 2 == 0) ? vEven : vOdd;

        float sx = x + (i * stripeWidth); // Current Stripe X start
        float ex = sx + stripeWidth;      // Current Stripe X end

        // --- The slope
        // Triangle 1
        push(target, sx, y,      z,       0, ny, nz);
        push(target, sx, y-drop, z+depth, 0, ny, nz);
        push(target, ex, y,      z,       0, ny, nz);

        // Triangle 2
        push(target, ex, y,      z,       0, ny, nz);
        push(target, sx, y-drop, z+depth, 0, ny, nz);
        push(target, ex, y-drop, z+depth, 0, ny, nz);

        // --- The flap
        // Triangle 1
        push(target, sx, y-drop,            z+depth, 0, 0, 1);
        push(target, sx, y-drop-flapHeight, z+depth, 0, 0, 1);
        push(target, ex, y-drop,            z+depth, 0, 0, 1);
        
        // Triangle 2
        push(target, ex, y-drop,            z+depth, 0, 0, 1);
        push(target, sx, y-drop-flapHeight, z+depth, 0, 0, 1);
        push(target, ex, y-drop-flapHeight, z+depth, 0, 0, 1);
    }
}

void StreetMap::initLampModel() {
    float poleHeight = 2.2f;
    float poleWidth = 0.10f;
    float baseWidth = 0.3f;
    
    // --- The pole
    // Base
    addCube(m_lampPoleMesh.vertices, -baseWidth/2, 0.0f, -baseWidth/2, baseWidth, 0.1f, baseWidth, 1.0f);
    
    // Main Shaft
    addCube(m_lampPoleMesh.vertices, -poleWidth/2, 0.1f, -poleWidth/2, poleWidth, poleHeight, poleWidth, 1.0f);

    // Lantern Cap
    float capWidth = 0.5f;
    addPrism(m_lampPoleMesh.vertices, -capWidth/2, poleHeight + 0.5f, -capWidth/2, capWidth, 0.15f, capWidth, 1.0f);


    // --- The bulb
    float bulbSize = 0.30f;
    addCube(m_lampBulbMesh.vertices, -bulbSize/2, poleHeight, -bulbSize/2, bulbSize, 0.5f, bulbSize, 1.0f);

    m_lampPoleMesh.setup();
    m_lampBulbMesh.setup();
}

void StreetMap::initLamps() {
    initLampModel();

    m_lampPositions.clear();
    m_lampPositions.push_back(glm::vec3(5.0f, 0.10f, 5.0f));
    m_lampPositions.push_back(glm::vec3(10.0f, 0.10f, 3.8f));
    m_lampPositions.push_back(glm::vec3(10.0f, 0.10f, 9.0f));
    m_lampPositions.push_back(glm::vec3(1.0f, 0.10f, 10.0f));
}