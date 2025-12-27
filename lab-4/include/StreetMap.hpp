#ifndef STREETMAP_HPP
#define STREETMAP_HPP

#include "Mesh.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <cmath>

class StreetMap {
public:
    void init();
    void cleanup();
    void drawLitObjects(Shader& shader);
    void drawEmissives(Shader& shader);
    void applyLightningState(Shader& shader);
    
private:
    void loadTextures();

    void addRectangle(std::vector<float>& v, float x1, float z1, float x2, float z2, float x3, float z3, float x4, float z4, float vStart, float vEnd);
    void addWall(std::vector<float>& v, float x1, float z1, float x2, float z2, float height, float vScale, bool flipNormal);
    void addCube(std::vector<float>& v, float x, float y, float z, float w, float h, float d, float uvScale);
    void addPrism(std::vector<float>& v, float x, float y, float z, float w, float h, float d, float uvScale);
    void addStripedAwning(
        std::vector<float>& vEven, std::vector<float>& vOdd, 
        float x, float y, float z, float width, float depth, float drop, int numStripes
    ); 

    void addEntrance(std::vector<float>& v, float x, float z, float h, float uvScale);

    void genBlackTexture();
    void initRoad();
    void initCurbs();
    void initSidewalks();
    void initShop1();
    void initShop2();
    void initShop3();
    void initShop4();
    void initShop5();
    void initBenchModel();
    void initBenches();
    void initLampModel();
    void initLamps();

    GLuint loadTexture(const char *path);

private:
    float m_roadWidth = 3.0f;
    float m_roadLength = 6.0f;
    float m_curbWidth = 0.25f;
    float m_innerSidewalkLength = m_roadLength - m_curbWidth;
    float m_roadOuter = m_innerSidewalkLength + m_curbWidth + m_roadWidth;
    
    // Sidewalks
    float m_outerVerticalSidewalkLength = m_roadOuter + m_curbWidth;
    float m_outerVerticalSidewalkWidth = 1.5f;
    float m_outerHorizontalSidewalkWidth = 5.0f;

    GLuint m_blackTexture;

    // --- Road
    Mesh m_roadMesh;
    GLuint m_roadTexture;
    
    // --- Curb
    Mesh m_outerCurbMesh;
    Mesh m_innerCurbMesh;
    GLuint m_curbTexture;
    
    // -- Sidewalk
    Mesh m_innerSidewalkMesh;
    Mesh m_outerSidewalkMesh;
    GLuint m_sidewalkTexture;

    // --- Shops
    Mesh m_shop1BaseMesh;
    Mesh m_shop1RoofMesh;
    Mesh m_shop1EntranceMesh;
    GLuint m_shop1Texture;

    Mesh m_shop2BaseMesh;
    Mesh m_shop2RoofMesh;
    Mesh m_shop2EntranceMesh;
    GLuint m_shop2BaseTexture;
    GLuint m_shop2RoofTexture;

    Mesh m_shop3BaseMesh;
    Mesh m_shop3EntranceMesh;
    Mesh m_awningEvenMesh;
    Mesh m_awningOddMesh;
    Mesh m_glowingWindowMesh;

    GLuint m_shop3Texture;

    Mesh m_shop4BaseMesh;
    Mesh m_shop4RoofMesh;
    Mesh m_shop4EntranceMesh;
    GLuint m_shop4BaseTexture;
    GLuint m_shop4RoofTexture;

    Mesh m_shop5BaseMesh;
    Mesh m_shop5RoofMesh;
    Mesh m_shop5EntranceMesh;
    GLuint m_shop5BaseTexture;
    GLuint m_shop5RoofTexture;

    // --- Benches
    Mesh m_benchWoodMesh;
    GLuint m_benchWoodTexture;
    Mesh m_benchMetalMesh;

    struct BenchLocation {
        glm::vec3 position;
        float rotation;
    };
    std::vector<BenchLocation> m_benches;

    // --- Lamps
    Mesh m_lampPoleMesh;
    Mesh m_lampBulbMesh;

public:
    std::vector<glm::vec3> m_lampPositions;
    Mesh m_lampMesh;
};

#endif // STREETMAP_HPP