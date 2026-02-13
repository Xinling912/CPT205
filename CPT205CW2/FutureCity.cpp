#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <windows.h>
#include <fstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ====================== Camera ======================
float camX = 100.0f, camY = 160.0f, camZ = 320.0f;
float lookX = 0.0f, lookY = 160.0f, lookZ = -600.0f;
float camYaw = 0.0f, camPitch = 0.0f;  // left and right rotation angle and up and down rotation angle
float PITCH_MIN = -45.0f, PITCH_MAX = 45.0f, YAW_MIN = -90.0f, YAW_MAX = 90.0f; 
float CAM_DEFAULT_X = 100.0f, CAM_DEFAULT_Y = 160.0f, CAM_DEFAULT_Z = 320.0f;
float LOOK_DEFAULT_X = 0.0f, LOOK_DEFAULT_Y = 160.0f, LOOK_DEFAULT_Z = -600.0f;
float YAW_DEFAULT = 0.0f, PITCH_DEFAULT = 0.0f;


// ====================== Utility ======================
void setMaterial(float r, float g, float b, float shininess, float alpha = 1.0f) {
    GLfloat ambient[] = { 0.2f * r, 0.2f * g, 0.2f * b, alpha };
    GLfloat diffuse[] = { 0.6f * r, 0.6f * g, 0.6f * b, alpha };
    GLfloat specular[] = { 0.9f, 0.9f, 0.9f, alpha };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void drawBox(float w, float h, float d) {
    float x = w / 2, y = h / 2, z = d / 2;
    glBegin(GL_QUADS);
    // Front
    glNormal3f(0, 0, 1);
    glVertex3f(-x, -y, z); glVertex3f(x, -y, z);
    glVertex3f(x, y, z); glVertex3f(-x, y, z);
    // Back
    glNormal3f(0, 0, -1);
    glVertex3f(-x, -y, -z); glVertex3f(-x, y, -z);
    glVertex3f(x, y, -z); glVertex3f(x, -y, -z);
    // Left
    glNormal3f(-1, 0, 0);
    glVertex3f(-x, -y, -z); glVertex3f(-x, -y, z);
    glVertex3f(-x, y, z); glVertex3f(-x, y, -z);
    // Right
    glNormal3f(1, 0, 0);
    glVertex3f(x, -y, -z); glVertex3f(x, y, -z);
    glVertex3f(x, y, z); glVertex3f(x, -y, z);
    // Top
    glNormal3f(0, 1, 0);
    glVertex3f(-x, y, -z); glVertex3f(-x, y, z);
    glVertex3f(x, y, z); glVertex3f(x, y, -z);
    // Bottom
    glNormal3f(0, -1, 0);
    glVertex3f(-x, -y, -z); glVertex3f(x, -y, -z);
    glVertex3f(x, -y, z); glVertex3f(-x, -y, z);
    glEnd();
}

// ====================== Texture Mapping ======================
GLuint groundTextureID;

//  BMP header
#pragma pack(push, 1)
struct BMPHeader {
    WORD fileType;        // must be "BM"
    DWORD fileSize;
    WORD reserved1;
    WORD reserved2;
    DWORD bitmapOffset;
    DWORD headerSize;
    LONG width;
    LONG height;
    WORD planes;
    WORD bitsPerPixel;
    DWORD compression;
    DWORD sizeOfBitmap;
};
#pragma pack(pop)

bool loadBMPTexture(const char* filename, GLuint& textureID) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        printf("Error: Cannot open texture file %s\n", filename);
        return false;
    }

    BMPHeader header;
    file.read((char*)&header, sizeof(header));

    if (header.fileType != 0x4D42) {
        printf("Error: Not a valid BMP file\n");
        return false;
    }

    if (header.bitsPerPixel != 24) {
        printf("Error: Only 24-bit BMP supported\n");
        return false;
    }

    int width = header.width;
    int height = abs(header.height);
    bool topDown = header.height < 0;

   
    int rowSize = ((width * 3 + 3) & ~3);
    int dataSize = rowSize * height;

    std::vector<GLubyte> raw(dataSize);
    file.seekg(header.bitmapOffset);
    file.read((char*)raw.data(), dataSize);

    
    std::vector<GLubyte> rgb(width * height * 3);
    for (int y = 0; y < height; y++) {
        int srcY = topDown ? y : (height - 1 - y);
        memcpy(&rgb[y * width * 3], &raw[srcY * rowSize], width * 3);

        // BGR -> RGB
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            std::swap(rgb[idx], rgb[idx + 2]);
        }
    }

    // Create OpenGL texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
        GL_RGB, GL_UNSIGNED_BYTE, rgb.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    printf("Texture loaded successfully: %s (%dx%d)\n", filename, width, height);
    return true;
}

// initialize texture
void initTextures() {
    if (!loadBMPTexture("groundTexture.bmp", groundTextureID)) {
        printf("Warning: Using default gray texture\n");

        glGenTextures(1, &groundTextureID);
        glBindTexture(GL_TEXTURE_2D, groundTextureID);

        GLubyte tex[16 * 3];
        memset(tex, 100, sizeof(tex));   //default using grey texture

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0,
            GL_RGB, GL_UNSIGNED_BYTE, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}


// ====================== Scene Components ======================
void drawSky() {
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.7f, 1.0f);
    glVertex3f(-3000, 1500, -3000);
    glColor3f(0.8f, 0.9f, 1.0f);
    glVertex3f(3000, 1500, -3000);
    glColor3f(0.9f, 0.95f, 1.0f);
    glVertex3f(3000, -200, -3000);
    glColor3f(0.6f, 0.8f, 1.0f);
    glVertex3f(-3000, -200, -3000);
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawGround() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTextureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    
    GLfloat mat_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat mat_shininess = 5.0f;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    // draw a big ground plane to use repeated texture
    float groundSize = 2800.0f; // ground size
    float textureRepeat = 50.0f; // texture repeated times

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

   
    glTexCoord2f(0.0f, 0.0f);           glVertex3f(-groundSize, 0.0f, -groundSize);
    glTexCoord2f(0.0f, textureRepeat);  glVertex3f(-groundSize, 0.0f, groundSize);
    glTexCoord2f(textureRepeat, textureRepeat); glVertex3f(groundSize, 0.0f, groundSize);
    glTexCoord2f(textureRepeat, 0.0f);  glVertex3f(groundSize, 0.0f, -groundSize);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawGroundNeonStrip(float x, float y, float startZ, float length) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    setMaterial(0.0f, 0.6f, 1.0f, 120.0f, 0.5f);

    glPushMatrix();
    glTranslatef(x, y, startZ);
    glScalef(5.0f, 1.0f, length);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
// ====================== Road System ======================
void drawRoadSystem() {
    // main road
    setMaterial(0.15f, 0.15f, 0.18f, 30.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -800.0f);
    drawBox(80.0f, 1.0f, 4000.0f);
    glPopMatrix();

    // road side light strip 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // left
    setMaterial(0.3f, 0.7f, 1.0f, 100.0f, 0.6f);
    glPushMatrix();
    glTranslatef(-40.0f, 1.2f, -800.0f); 
    glScalef(2.0f, 0.5f, 4000.0f);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();
    // right
    glPushMatrix();
    glTranslatef(40.0f, 1.2f, -800.0f);
    glScalef(2.0f, 0.5f, 4000.0f);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    // rode marking
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.8f);
    float lineSpacing = 100.0f;
    for (float z = -2800.0f; z < 1200.0f; z += lineSpacing) {
        glPushMatrix();
        glTranslatef(0.0f, 1.2f, z);
        glScalef(4.0f, 0.2f, 20.0f);
        drawBox(1.0f, 1.0f, 1.0f);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);
}

// ====================== Street Lights ======================
void drawStreetLight(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    // lamp post
    setMaterial(0.3f, 0.3f, 0.35f, 40.0f);
    glPushMatrix();
    glTranslatef(0.0f, 40.0f, 0.0f);
    drawBox(2.0f, 80.0f, 2.0f);
    glPopMatrix();

    // lamp head
    glEnable(GL_BLEND);
    setMaterial(1.0f, 1.0f, 0.9f, 100.0f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, 80.0f, 0.0f);
    glScalef(8.0f, 4.0f, 8.0f);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();
    glDisable(GL_BLEND);

    glPopMatrix();
}

void drawStreetLights() {
    float spacing = 250.0f;
    for (float z = -2800.0f; z < 1200.0f; z += spacing) {
        drawStreetLight(60.0f, z);   // right
        drawStreetLight(-60.0f, z);  // left
    }
}
// ====================== Futuristic Trees ======================
void drawFuturisticTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    // trunk
    setMaterial(0.4f, 0.3f, 0.2f, 20.0f);
    glPushMatrix();
    glTranslatef(0.0f, 25.0f, 0.0f);
    drawBox(3.0f, 50.0f, 3.0f);
    glPopMatrix();

    // four-layer gradient cone
    float coneLayers[][6] = {
        // Bottom radius, height, y location, r, g, b
        {18.0f, 30.0f, 25.0f, 0.08f, 0.5f, 0.08f},  
        {14.0f, 25.0f, 40.0f, 0.15f, 0.7f, 0.15f},  
        {10.0f, 20.0f, 50.0f, 0.25f, 0.9f, 0.25f},  
        { 6.0f, 15.0f, 58.0f, 0.35f, 1.0f, 0.35f}   
    };

    for (int i = 0; i < 4; i++) {
        setMaterial(coneLayers[i][3], coneLayers[i][4], coneLayers[i][5], 70 + i * 10, 1.0f);
        glPushMatrix();
        glTranslatef(0.0f, coneLayers[i][2], 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(coneLayers[i][0], coneLayers[i][1], 16, 8);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawGreenBelts() {
    // plant on both side of street
    float treeSpacing = 120.0f;
    for (float z = -1400.0f; z < 400.0f; z += treeSpacing) {
        drawFuturisticTree(100.0f, z);   // right
        drawFuturisticTree(-100.0f, z);  // left
    }
}
// ====================== Holographic Billboard ======================
void drawHolographicBillboard(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // Billboard bracket
    setMaterial(0.5f, 0.5f, 0.6f, 50.0f);
    glPushMatrix();
    glTranslatef(0.0f, 40.0f, 0.0f);
    drawBox(4.0f, 80.0f, 4.0f);
    glPopMatrix();

    // holographic display panel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    setMaterial(0.8f, 0.3f, 0.8f, 90.0f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 90.0f, 0.0f);
    glScalef(50.0f, 100.0f, 2.0f);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    // Scan animation line 
    static float scanLine = 0.0f;
    scanLine += 0.5f;
    if (scanLine > 60.0f) scanLine = -45.0f;

    glDisable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex3f(-25.0f, 75.0f + scanLine, 1.1f);
    glVertex3f(25.0f, 75.0f + scanLine, 1.1f);
    glVertex3f(25.0f, 80.0f + scanLine, 1.1f);
    glVertex3f(-25.0f, 80.0f + scanLine, 1.1f);
    glEnd();
    glEnable(GL_LIGHTING);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    glPopMatrix();
}


// ====================== Vehicle System ======================

void drawFuturisticHoverCar(float x, float z, float t, float offset)
{
    float bodyLen = 20.0f;
    float bodyRad = 5.0f;
    float floatY = 6.0f + sinf(t * 2.0f + offset) * 0.5f; //floating y
    

    static GLUquadric* quad = nullptr;
    if (!quad) {
        quad = gluNewQuadric();
        gluQuadricNormals(quad, GLU_SMOOTH);
    }

    glPushMatrix();
    glTranslatef(x, floatY, z);
    glScaled(2, 2, 2);

    // main body
    setMaterial(0.45f, 0.65f, 0.95f, 128.0f, 1.0f); 
    glPushMatrix();
    glTranslatef(0, 0, -bodyLen / 2);
    gluCylinder(quad, bodyRad, bodyRad * 0.95f, bodyLen, 40, 4);
    // back cover
    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    gluDisk(quad, 0, bodyRad, 40, 1);
    glPopMatrix();
    glPopMatrix();

    // front
    glPushMatrix();
    glTranslatef(0, 0, bodyLen / 2);
    glutSolidSphere(bodyRad * 0.95f, 40, 40);
    glPopMatrix();

    // back
    glPushMatrix();
    glTranslatef(0, 0, -bodyLen / 2);
    glutSolidSphere(bodyRad * 0.9f, 40, 40);
    glPopMatrix();

    //transparent cockpit cover
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setMaterial(0.2f, 0.5f, 0.8f, 90.0f, 0.35f);
    glPushMatrix();
    glTranslatef(0, bodyRad * 0.7f, 0);
    glScalef(1.2f, 0.6f, 0.8f);
    glutSolidSphere(bodyRad * 0.9f, 36, 36);
    glPopMatrix();
    glDisable(GL_BLEND);

    // chassis
    setMaterial(0.1f, 0.3f, 0.9f, 40.0f, 0.4f);
    glPushMatrix();
    glTranslatef(0, -bodyRad * 0.7f, 0);
    glScalef(1.4f, 0.2f, 0.6f);
    glutSolidSphere(bodyRad * 1.1f, 36, 36);
    glPopMatrix();

    // decoration light
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.8f, 1.0f);
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    for (int i = -8; i <= 8; i += 2) {
        glVertex3f(bodyRad, 0, i);
        glVertex3f(bodyRad, 0.5, i + 1.0f);
        glVertex3f(-bodyRad, 0, i);
        glVertex3f(-bodyRad, 0.5, i + 1.0f);
    }
    glEnd();
    glEnable(GL_LIGHTING);

    // front/back light
    setMaterial(1.0f, 0.95f, 0.7f, 120.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0, 0, bodyLen / 2 + 0.5f);
    glutSolidSphere(0.5f, 10, 10);
    glPopMatrix();

    setMaterial(1.0f, 0.3f, 0.2f, 80.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0, 0, -bodyLen / 2 - 0.5f);
    glutSolidSphere(0.5f, 10, 10);
    glPopMatrix();

    glPopMatrix();
}

void drawFuturisticVehicles()
{
    static float t = 0.0f;
    t += 0.02f;


    float startZ = -2500.0f;  
    float endZ = 500.0f;      
    float vehicleSpeed = 10.0f; 

   
    struct Vehicle {
        float x;           // x location
        float currentZ;    // z location
        float startDelay;  // delay time
        bool active;       // activation status
    };

    static Vehicle vehicles[12] = {
        // right lane
        {25.0f, startZ, 0.0f, true},      // activate immediately
        {25.0f, startZ, 50.0f, false},    // delay by 50 unit
        {25.0f, startZ, 100.0f, false},   // delay by 100 unit
        {25.0f, startZ, 150.0f, false},   
        {25.0f, startZ, 200.0f, false},   
        {25.0f, startZ, 250.0f, false},   

        // right lane
        {-25.0f, startZ, 25.0f, false},  
        {-25.0f, startZ, 75.0f, false},   
        {-25.0f, startZ, 125.0f, false},  
        {-25.0f, startZ, 175.0f, false},  
        {-25.0f, startZ, 225.0f, false}, 
        {-25.0f, startZ, 275.0f, false}  
    };

    // update vehicles
    for (int i = 0; i < 12; ++i) {
        Vehicle& v = vehicles[i];

        // check activation status
        if (!v.active && t * vehicleSpeed >= v.startDelay) {
            v.active = true;
            v.currentZ = startZ;
        }

        // update activated vehicle
        if (v.active) {
            v.currentZ += vehicleSpeed;

            // check if reset
            if (v.currentZ > endZ) {
                v.currentZ = startZ; // reset to start point
            }

           
            drawFuturisticHoverCar(v.x, v.currentZ, t, i * 0.5f);
        }
    }
}


// ====================== Building System ======================

struct BuildingType {
    float width, depth, minHeight, maxHeight;
    float r, g, b;  
    const char* style;
};

BuildingType buildingStyles[] = {
    // blue
    {80.0f, 80.0f, 180.0f, 280.0f, 0.5f, 0.6f, 0.8f, "corporate"},
    // silver grey
    {70.0f, 70.0f, 150.0f, 220.0f, 0.6f, 0.65f, 0.7f, "tech"},
    // warm tone
    {60.0f, 60.0f, 120.0f, 180.0f, 0.7f, 0.75f, 0.8f, "residential"},
    // glass
    {90.0f, 90.0f, 100.0f, 160.0f, 0.4f, 0.7f, 0.9f, "commercial"},
    // cool tone
    {50.0f, 50.0f, 80.0f, 130.0f, 0.3f, 0.5f, 0.6f, "data"}
};

void drawDetailedBuilding(float x, float z, const BuildingType& style, float height) {
    glPushMatrix();
    glTranslatef(x, height * 0.5f, z);

    // main body
    setMaterial(style.r, style.g, style.b, 80.0f, 0.95f);
    drawBox(style.width, height, style.depth);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // front
    setMaterial(0.3f, 0.6f, 0.9f, 90.0f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, style.depth * 0.5f + 0.1f);
    drawBox(style.width * 0.95f, height * 0.98f, 1.0f);
    glPopMatrix();

    glDisable(GL_BLEND);

    // glass line
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.8f, 1.0f); 

    // front galss line
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, style.depth * 0.5f + 0.2f);
    glBegin(GL_LINES);

    // line spacing
    float lineSpacingX = style.width / 8.0f; 
    for (float x = -style.width * 0.45f; x <= style.width * 0.45f; x += lineSpacingX) {
        glVertex3f(x, -height * 0.48f, 0.0f);
        glVertex3f(x, height * 0.48f, 0.0f);
    }

    // glass line (horizontal)
    if (height > 150.0f) { // higher buliding have 
        float lineSpacingY = height / 12.0f;
        for (float y = -height * 0.45f; y <= height * 0.45f; y += lineSpacingY) {
            glVertex3f(-style.width * 0.45f, y, 0.0f);
            glVertex3f(style.width * 0.45f, y, 0.0f);
        }
    }

    glEnd();
    glPopMatrix();

    // side
    if (x < 0) { // left side building
        glPushMatrix();
        glTranslatef(style.width * 0.5f + 0.2f, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);

        float sideLineSpacing = style.depth / 6.0f;
        for (float z = -style.depth * 0.45f; z <= style.depth * 0.45f; z += sideLineSpacing) {
            glVertex3f(z, -height * 0.48f, 0.0f);
            glVertex3f(z, height * 0.48f, 0.0f);
        }

        glEnd();
        glPopMatrix();
    }
    else { // right side building
        glPushMatrix();
        glTranslatef(-style.width * 0.5f - 0.2f, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);

        float sideLineSpacing = style.depth / 6.0f;
        for (float z = -style.depth * 0.45f; z <= style.depth * 0.45f; z += sideLineSpacing) {
            glVertex3f(z, -height * 0.48f, 0.0f);
            glVertex3f(z, height * 0.48f, 0.0f);
        }

        glEnd();
        glPopMatrix();
    }

    glEnable(GL_LIGHTING);

    // top
    setMaterial(0.2f, 0.25f, 0.3f, 60.0f);
    glPushMatrix();
    glTranslatef(0.0f, height * 0.5f + 5.0f, 0.0f);
    drawBox(style.width * 0.7f, 10.0f, style.depth * 0.7f);
    glPopMatrix();

    glPopMatrix();
}


void drawBuildingClusters() {
    // left side 
    float leftBuildings[] = {
        -300.0f, -650.0f, 1, 220.0f,  // x, z, style index, height multiplier
        -320.0f, -500.0f, 0, 260.0f,
        -350.0f, -780.0f, 2, 180.0f,
        -300.0f, -300.0f, 3, 150.0f,
        -380.0f, -550.0f, 4, 120.0f,
        -330.0f, -200.0f, 0, 240.0f,
        -360.0f, -100.0f, 1, 200.0f,
        -310.0f, -850.0f, 2, 160.0f,
        -300.0f,  50.0f, 3, 210.0f,
        -340.0f, 150.0f, 1, 250.0f,
        -310.0f, 260.0f, 0, 230.0f,
        -360.0f, 340.0f, 4, 180.0f,
        -330.0f, 400.0f, 2, 200.0f,
        -320.0f,  630.0f, 1, 230.0f,
        -350.0f,  720.0f, 3, 210.0f,
        -300.0f,  520.0f, 0, 250.0f,
        -380.0f,  470.0f, 4, 190.0f
     };

    // right side 
    float rightBuildings[] = {
        280.0f, -600.0f, 0, 280.0f,
        320.0f, -450.0f, 1, 240.0f,
        350.0f, -800.0f, 3, 170.0f,
        300.0f, -250.0f, 2, 190.0f,
        380.0f, -500.0f, 4, 110.0f,
        330.0f, -150.0f, 1, 230.0f,
        360.0f, -50.0f, 0, 270.0f,
        310.0f, -950.0f, 3, 140.0f,
        300.0f,  80.0f, 2, 220.0f,
        340.0f, 180.0f, 4, 260.0f,
        320.0f, 260.0f, 1, 210.0f,
        390.0f, 320.0f, 0, 200.0f,
        310.0f, 400.0f, 3, 170.0f,
        380.0f,  520.0f,   2, 240.0f,
        360.0f,  580.0f,   4, 200.0f,
        300.0f,  650.0f,   1, 260.0f,
        390.0f,  450.0f,   0, 220.0f
    };

    
    for (int i = 0; i < 17; i++) {
        float x = leftBuildings[i * 4];
        float z = leftBuildings[i * 4 + 1];
        int styleIndex = (int)leftBuildings[i * 4 + 2];
        float heightMultiplier = leftBuildings[i * 4 + 3];

        BuildingType style = buildingStyles[styleIndex];
        float height = style.minHeight + (style.maxHeight - style.minHeight) * (heightMultiplier / 300.0f);

        drawDetailedBuilding(x, z, style, height);
    }

    
    for (int i = 0; i < 17; i++) {
        float x = rightBuildings[i * 4];
        float z = rightBuildings[i * 4 + 1];
        int styleIndex = (int)rightBuildings[i * 4 + 2];
        float heightMultiplier = rightBuildings[i * 4 + 3];

        BuildingType style = buildingStyles[styleIndex];
        float height = style.minHeight + (style.maxHeight - style.minHeight) * (heightMultiplier / 300.0f);

        drawDetailedBuilding(x, z, style, height);
    }
}


void drawDistantBuildings() {
    
    float distantLeft[] = { -400.0f, -1200.0f, 180.0f, 0.5f, 0.55f, 0.65f };
    float distantRight[] = { 400.0f, -1100.0f, 220.0f, 0.55f, 0.6f, 0.7f };
    
    
    for (int i = 0; i < 2; i++) {
        float* building = (i == 0) ? distantLeft : distantRight;

        glPushMatrix();
        glTranslatef(building[0], building[2] * 0.5f, building[1]);

        // main body
        setMaterial(building[3], building[4], building[5], 50.0f, 0.9f);
        drawBox(120.0f, building[2], 100.0f);

        // glass line
        glEnable(GL_BLEND);
        setMaterial(0.3f, 0.6f, 0.9f, 60.0f, 0.2f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 50.1f);
        drawBox(115.0f, building[2] * 0.95f, 1.0f);
        glPopMatrix();
        glDisable(GL_BLEND);

       
        glDisable(GL_LIGHTING);
        glColor3f(0.2f, 0.8f, 1.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 50.2f);
        glBegin(GL_LINES);

        // line
        for (int j = -50; j <= 50; j += 25) {
            glVertex3f(j, -building[2] * 0.45f, 0.0f);
            glVertex3f(j, building[2] * 0.45f, 0.0f);
        }

        glEnd();
        glPopMatrix();
        glEnable(GL_LIGHTING);

        glPopMatrix();
    }
}

void drawDistantRoadBuildings() {
  
    float deepBuildings[] = {
        // left buildings
        -280.0f, -1000.0f, 1, 220.0f,   // x, z, style index, height multiplier
        -320.0f, -1100.0f, 0, 250.0f,
        -300.0f, -1200.0f, 2, 200.0f,
        -280.0f, -1300.0f, 3, 180.0f,

        // right buildings
        280.0f,  -1050.0f, 3, 240.0f,
        320.0f,  -1150.0f, 2, 220.0f,
        300.0f,  -1250.0f, 1, 190.0f,
        280.0f,  -1350.0f, 0, 170.0f
    };

   
    for (int i = 0; i < 8; i++) {
        float x = deepBuildings[i * 4];
        float z = deepBuildings[i * 4 + 1];
        int styleIndex = (int)deepBuildings[i * 4 + 2];
        float heightMultiplier = deepBuildings[i * 4 + 3];

        BuildingType style = buildingStyles[styleIndex];
        float height = style.minHeight * 0.8f + (style.maxHeight * 0.8f - style.minHeight * 0.8f) * (heightMultiplier / 300.0f);

        
        glPushMatrix();
        glTranslatef(x, height * 0.5f, z);

        
        setMaterial(style.r, style.g, style.b, 70.0f, 0.9f);
        drawBox(style.width * 0.9f, height, style.depth * 0.9f);

        // glass line
        glEnable(GL_BLEND);
        setMaterial(0.4f, 0.7f, 0.9f, 80.0f, 0.4f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, style.depth * 0.45f + 0.1f);
        drawBox(style.width * 0.85f, height * 0.95f, 1.0f);
        glPopMatrix();
        glDisable(GL_BLEND);

        glPopMatrix();
    }
}

void drawMainBuilding(float x, float z,
    float width = 150.0f, float height = 300.0f, float depth = 150.0f) {
    // body
    glPushMatrix();
    glTranslatef(x, height * 0.5f, z);
    setMaterial(0.5f, 0.5f, 0.6f, 80.0f, 1.0f);
    drawBox(width, height, depth);
    glPopMatrix();

    //glass
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPushMatrix();
    glTranslatef(x, height * 0.5f, z);
    setMaterial(0.4f, 0.7f, 0.9f, 90.0f, 0.5f);
    drawBox(width + 5.0f, height + 5.0f, depth + 5.0f); 
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // glass line
    glDisable(GL_LIGHTING);
    glColor4f(0.2f, 0.8f, 1.0f, 0.5f);
    glPushMatrix();
    glTranslatef(x, height * 0.5f, z);
    glBegin(GL_LINES);
    float halfWidth = width * 0.5f;
    for (int i = -static_cast<int>(halfWidth * 0.9f); i <= static_cast<int>(halfWidth * 0.9f); i += 20) {
        glVertex3f(i, -height * 0.5f, depth * 0.5f);
        glVertex3f(i, height * 0.5f, depth * 0.5f);
    }
    glEnd();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawSideBuilding(float x, float z,
    float length=120, float height=300, float depth=120,
    float r=0.5, float g=0.6, float b=0.8) {
    glPushMatrix();
    glTranslatef(x, height * 0.5f, z); 
    setMaterial(r, g, b, 90.0f, 0.9f);

    drawBox(length, height, depth);

    // glass line
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.8f, 1.0f); 
    glBegin(GL_LINES);

    // line spacing
    float halflength = length * 0.5f;
    float lineSpacing = length / 8.0f;
    for (float i = -halflength * 0.8f; i <= halflength * 0.8f; i += lineSpacing) {
        glVertex3f(i, -height * 0.5f, depth * 0.5f);
        glVertex3f(i, height * 0.5f, depth * 0.5f);
    }

    glEnd();
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void setupBuildingNeonLights() {
    glEnable(GL_LIGHT3);
    glEnable(GL_LIGHT4);
    glEnable(GL_LIGHT5);

   
    GLfloat blue[] = { 0.2f, 0.6f, 1.0f, 1.0f };
    GLfloat purple[] = { 0.6f, 0.3f, 1.0f, 1.0f };
    GLfloat cyan[] = { 0.0f, 0.8f, 0.8f, 1.0f };

    // middle buildings
    GLfloat pos3[] = { 0.0f, 300.0f, -800.0f, 1.0f }; 
    glLightfv(GL_LIGHT3, GL_POSITION, pos3);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, blue);
    glLightfv(GL_LIGHT3, GL_SPECULAR, blue);

    // left buildings
    GLfloat pos4[] = { -220.0f, 180.0f, -700.0f, 1.0f };
    glLightfv(GL_LIGHT4, GL_POSITION, pos4);
    glLightfv(GL_LIGHT4, GL_DIFFUSE, purple);
    glLightfv(GL_LIGHT4, GL_SPECULAR, purple);

    // right buildings
    GLfloat pos5[] = { 250.0f, 400.0f, -700.0f, 1.0f };
    glLightfv(GL_LIGHT5, GL_POSITION, pos5);
    glLightfv(GL_LIGHT5, GL_DIFFUSE, cyan);
    glLightfv(GL_LIGHT5, GL_SPECULAR, cyan);
}


// ====================== Air Rail System ======================

void drawTrainStation(float stationZ = -1200.0f) {
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, stationZ);
    glScalef(2, 2.5, 1);


    // station main parts

    // platform roofing 
    setMaterial(0.8f, 0.85f, 0.9f, 70.0f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 85.0f, 0.0f);
    drawBox(200.0f, 10.0f, 120.0f); 
    glPopMatrix();

    // support
    setMaterial(0.6f, 0.65f, 0.7f, 60.0f);
    float supportPositions[] = { -80.0f, -40.0f, 0.0f, 40.0f, 80.0f };
    for (int i = 0; i < 5; i++) {
        if (i != 2){ 
        glPushMatrix();
        glTranslatef(supportPositions[i], 42.5f, 0.0f);
        drawBox(8.0f, 85.0f, 8.0f); 
        glPopMatrix(); 
        }
    }

    // platform floor
    setMaterial(0.4f, 0.45f, 0.5f, 50.0f);
    glPushMatrix();
    glTranslatef(65.0f, 15.0f, 0.0f);
    drawBox(60.0f, 30.0f, 100.0f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-65.0f, 15.0f, 0.0f);
    drawBox(60.0f, 30.0f, 100.0f); 
    glPopMatrix();

    // hollow area
    // left
    glPushMatrix();
    glTranslatef(-65.0f, 25.0f, 0.0f);
    drawBox(25.0f, 40.0f, 120.0f); 
    glPopMatrix();

    // right 
    glPushMatrix();
    glTranslatef(65.0f, 25.0f, 0.0f);
    drawBox(25.0f, 40.0f, 120.0f);
    glPopMatrix();

    // road passage
    setMaterial(0.15f, 0.15f, 0.18f, 30.0f);
    glPushMatrix();
    glTranslatef(0.0f, -10.0f, 0.0f);
    drawBox(100.0f, 20.0f, 80.0f); 
    glPopMatrix();

   
    // light
    setMaterial(1.0f, 1.0f, 0.9f, 100.0f);
    float lightPositions[] = { -60.0f, -20.0f, 20.0f, 60.0f };
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(lightPositions[i], 75.0f, 0.0f);
        glutSolidSphere(3.0f, 12, 12); 
        glPopMatrix();
    }

    // screen
    setMaterial(0.1f, 0.8f, 0.3f, 90.0f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, 65.0f, -45.0f);
    drawBox(60.0f, 12.0f, 2.0f); 

    // scanning line
    static float cursorPos = -30.0f; 
    cursorPos += 0.5f; 

    
    if (cursorPos > 30.0f) {
        cursorPos = -30.0f;
    }

    glDisable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f); 

    
    glBegin(GL_QUADS);
    glVertex3f(cursorPos - 2.5f, -4.5f, 1.1f);    
    glVertex3f(cursorPos + 2.5f, -4.5f, 1.1f);    
    glVertex3f(cursorPos + 2.5f, 4.5f, 1.1f);   
    glVertex3f(cursorPos - 2.5f, 4.5f, 1.1f);     
    glEnd();

    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(0.8f, 1.0f, 1.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex3f(cursorPos - 2.0f, -4.0f, 1.05f);   
    glVertex3f(cursorPos + 2.0f, -4.0f, 1.05f);
    glVertex3f(cursorPos + 2.0f, 4.0f, 1.05f);
    glVertex3f(cursorPos - 2.0f, 4.0f, 1.05f);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    glPopMatrix();

    glDisable(GL_BLEND);
    glPopMatrix();
}
void drawSingleRail(float startX, float startY, float startZ, float lengthZ) {
    // Track base
    setMaterial(0.7f, 0.72f, 0.75f, 80.0f);
    glPushMatrix();
    glTranslatef(startX, startY, startZ);
    drawBox(14.0f, 8.0f, lengthZ);
    glPopMatrix();

    // light strip on track
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    setMaterial(0.0f, 0.8f, 1.0f, 120.0f, 0.8f);
    glPushMatrix();
    glTranslatef(startX - 7.5f, startY, startZ);
    glScalef(1.0f, 8.0f, lengthZ);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(startX + 7.5f, startY, startZ); 
    glScalef(1.0f, 8.0f, lengthZ);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();
    
    setMaterial(0.3f, 0.9f, 1.0f, 100.0f, 0.6f);
    glPushMatrix();
    glTranslatef(startX, startY - 5.0f, startZ);
    glScalef(14.0f, 1.0f, lengthZ);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
}



void drawParallelRailPair() {
    float railY = 120.0f;
    float lengthZ = 4000.0f;
    float leftX = -130.0f;
    float rightX = 130.0f;
    float startZ = -1000.0f;

    
    drawSingleRail(leftX, railY, startZ, lengthZ);
    drawSingleRail(rightX, railY, startZ, lengthZ);
}

// ====================== Maglev Train ======================
float trainPositionZ = -2000.0f; // initial positon
float trainSpeed = 10.0f;         
float train1Z = -2000.0f;        // first train initial position
float train2Z = -2000.0f;        // second train initial position£¨not active£©     
bool train2Active = false;       // activation status

void drawCapsuleTrainCar(float length, float radius, bool isHead) {
    
    glPushMatrix();

    // train main body
    setMaterial(0.75f, 0.75f, 0.78f, 80.0f); 
    GLUquadric* quad = gluNewQuadric();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -length * 0.5f);
    gluCylinder(quad, radius, radius, length, 40, 12);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, length * 0.5f);
    glutSolidSphere(radius, 30, 30);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -length * 0.5f);
    glRotatef(180, 0, 1, 0);
    glutSolidSphere(radius, 30, 30);
    glPopMatrix();

    
    setMaterial(0.3f, 0.7f, 1.0f, 100.0f, 0.7f); 
    glPushMatrix();
    glTranslatef(0.0f, radius * 0.2f, 0.0f);
    glScalef(radius * 1.6f, radius * 0.6f, length * 0.8f);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    
    setMaterial(0.4f, 0.8f, 1.0f, 120.0f, 0.7f);

    glPushMatrix();
    glTranslatef(0.0f, -radius * 1.2f, 0.0f); 
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(radius * 0.12f, radius * 1.1f, 12, 24);
    glPopMatrix();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    
    if (isHead) {
        setMaterial(1.0f, 0.9f, 0.6f, 120.0f); 
        glPushMatrix();
        glTranslatef(0.0f, radius * 0.1f, length * 0.55f);
        glutSolidSphere(radius * 0.2f, 12, 12);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(radius * 0.3f, radius * 0.0f, length * 0.55f);
        glutSolidSphere(radius * 0.1f, 12, 12);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-radius * 0.3f, radius * 0.0f, length * 0.55f);
        glutSolidSphere(radius * 0.1f, 12, 12);
        glPopMatrix();
    }

    gluDeleteQuadric(quad);
    glPopMatrix();
}




void drawSingleTrainSet(float x, float y, float baseZ) {
    glPushMatrix();
    glTranslatef(x, y, baseZ);

    float carLength = 40.0f;
    float radius = 12.0f; 
    float gap = 15.0f;    

    // three carriages
    drawCapsuleTrainCar(carLength, radius, true); // head
    glTranslatef(0.0f, 0.0f, -carLength - gap);
    drawCapsuleTrainCar(carLength, radius, false); // middle
    glTranslatef(0.0f, 0.0f, -carLength - gap);
    drawCapsuleTrainCar(carLength, radius, false); // tail

    glPopMatrix();
}

void drawMaglevTrains(float x, float y) {
    // first train
    drawSingleTrainSet(x, y, train1Z);

    // second train£¨draw after activation£©
    if (train2Active) drawSingleTrainSet(x, y, train2Z);
}


void updateTrain(int value) {
    
    // first train movement
    train1Z += trainSpeed;
    if (train1Z > 500.0f) train1Z = -2000.0f; 

    // second train delayed movement
    if (!train2Active && train1Z > -750.0f) {
        train2Active = true;
        train2Z = -2000.0f; // second train start point
    }

    // second train movement
    if (train2Active) {
        train2Z += trainSpeed;
        if (train2Z > 500.0f) train2Z = -2000.0f; 
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateTrain, 0); // 60FPS
}

// ====================== Drone ======================
struct Drone {
    float x, y, z;           // position
    float offset;            // current vertical offset
    float minY, maxY;        // upper and lower limits
    bool movingUp;           // moving direction status
    float propellerAngle;    // propeller ratation angle
    int lightID;             // OpenGL light ID 
};


Drone drones[] = {
    {  0.0f, 230.0f, -200.0f, 0.0f, 200.0f, 260.0f, true, 0.0f },
    { 160.0f,190.0f, -200.0f, 0.0f, 170.0f, 210.0f, false, 0.0f },
    {-80.0f, 218.0f, -100.0f, 0.0f, 200.0f, 220.0f, true, 0.0f },
    { 40.0f, 219.0f,  -60.0f, 0.0f, 200.0f, 230.0f, false, 0.0f }
};
int droneCount = sizeof(drones) / sizeof(Drone);

//initialize light number
void initDrones() {
    for (int i = 0; i < droneCount; i++) {
        drones[i].lightID = GL_LIGHT4 + i; // allocate from light4
    }
}
void setupDroneLight(const Drone& d) {
    glEnable(GL_LIGHTING);
    glEnable(d.lightID);

    
    GLfloat lightColor[] = { 0.3f, 0.7f, 1.0f, 1.0f };
    GLfloat lightPos[] = { d.x, d.y - 3.0f, d.z, 1.0f }; 

    glLightfv(d.lightID, GL_POSITION, lightPos);
    glLightfv(d.lightID, GL_DIFFUSE, lightColor);
    glLightfv(d.lightID, GL_SPECULAR, lightColor);

    glLightf(d.lightID, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(d.lightID, GL_LINEAR_ATTENUATION, 0.02f);
}

void updateDrones() {
    for (int i = 0; i < droneCount; i++) {
        drones[i].propellerAngle += 25.0f;
        if (drones[i].propellerAngle > 360.0f)
            drones[i].propellerAngle -= 360.0f;

        if (drones[i].movingUp)
            drones[i].y += 0.2f;
        else
            drones[i].y -= 0.2f;

        if (drones[i].y > drones[i].maxY) drones[i].movingUp = false;
        if (drones[i].y < drones[i].minY) drones[i].movingUp = true;
    }
}

void updateDronesTimer(int value) {
    updateDrones();          
    glutPostRedisplay();     
    glutTimerFunc(16, updateDronesTimer, 0);  
}

void drawDrone(const Drone& d) {
    glPushMatrix();
    glTranslatef(d.x, d.y, d.z);
    glScalef(12.0f, 12.0f, 12.0f);

    
    glEnable(GL_LIGHTING);
    glEnable(d.lightID);

    GLfloat mat_ambient[] = { 0.3f, 0.35f, 0.4f, 1.0f };
    GLfloat mat_diffuse[] = { 0.7f, 0.8f, 0.9f, 1.0f };
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_emission[] = { 0.05f, 0.1f, 0.2f, 1.0f };
    GLfloat mat_shininess = 128.0f;  
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    // main body
    glPushMatrix();
    glScalef(0.7, 0.5, 1.5);
    glutSolidCube(0.7);
    glPopMatrix();

    // cross arms
    float sqrt2 = std::sqrt(2.0f) / 2;
    float rackLength = 1.0f;
    float angles[4] = { 45, -45, 135, -135 };
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glRotatef(angles[i], 0, 1, 0);
        glutSolidCylinder(0.08, rackLength, 20, 20);
        glPopMatrix();
    }

    // vertical arm
    float pos[4][2] = { {sqrt2, sqrt2}, {-sqrt2, sqrt2}, {-sqrt2, -sqrt2}, {sqrt2, -sqrt2} };
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(pos[i][0], 0.15f, pos[i][1]);
        glRotatef(90, 1, 0, 0);
        glutSolidCylinder(0.08, 0.3, 20, 20);
        glPopMatrix();
    }

    // propeller
    GLfloat blade_ambient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
    GLfloat blade_diffuse[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat blade_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat blade_shininess[] = { 60.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, blade_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blade_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, blade_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, blade_shininess);

    float armOffset = sqrt2;
    float bladeLength = 0.6f;
    float bladeWidth = 0.08f;
    float bladeThickness = 0.01f;
    for (int i = 0; i < 4; i++) {
        float px = (i == 0 || i == 3) ? armOffset : -armOffset;
        float pz = (i == 0 || i == 1) ? armOffset : -armOffset;
        glPushMatrix();
        glTranslatef(px, 0.25f, pz);
        glRotatef(d.propellerAngle, 0, 1, 0);
        glPushMatrix(); glScalef(bladeLength, bladeThickness, bladeWidth); glutSolidCube(1.0); glPopMatrix();
        glPushMatrix(); glScalef(bladeWidth, bladeThickness, bladeLength); glutSolidCube(1.0); glPopMatrix();
        glPopMatrix();
    }

    // ring of light
    GLfloat lightColor[] = { 0.0f, 0.7f, 1.0f, 1.0f }; 
    GLfloat lightPos[] = { 0.0f, -0.6f, 0.0f, 1.0f };  
    glLightfv(d.lightID, GL_POSITION, lightPos);
    glLightfv(d.lightID, GL_DIFFUSE, lightColor);
    glLightfv(d.lightID, GL_SPECULAR, lightColor);
    glLightf(d.lightID, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(d.lightID, GL_LINEAR_ATTENUATION, 0.01f);

    glPopMatrix();
}

// ======================  Robot ======================
void drawCuteRobot(float x, float y, float z, float bodyTilt = 0.0f) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(bodyTilt, 0.0f, 0.0f, 1.0f);

    // local using materials
    auto setMat = [](float r, float g, float b, float shin) {
        GLfloat amb[] = { 0.2f * r, 0.2f * g, 0.2f * b, 1.0f };
        GLfloat diff[] = { r, g, b, 1.0f };
        GLfloat spec[] = { 0.9f, 0.9f, 0.95f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
        glMaterialf(GL_FRONT, GL_SHININESS, shin);
        };

    // bottom roller
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);

    setMat(0.55f, 0.6f, 0.68f, 60.0f); 
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f); 
    glutSolidCylinder(4.0f, 2.0f, 24, 8);
    glPopMatrix();

    setMat(0.95f, 0.95f, 1.0f, 100.0f);
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCylinder(1.2f, 2.4f, 16, 6);
    glPopMatrix();

    glPopMatrix(); 

    // body
    glPushMatrix();
    glTranslatef(0.0f, 10.0f, 0.0f);
    setMat(0.95f, 0.95f, 0.98f, 80.0f); 
    glPushMatrix();
    glScalef(1.0f, 1.2f, 0.85f);
    glutSolidSphere(5.0f, 28, 28);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.6f, 0.8f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    for (int i = -3; i <= 3; i += 2) {
        glVertex3f(i, -3.0f, 4.3f);
        glVertex3f(i, 3.0f, 4.3f);
    }
    glEnd();
    glEnable(GL_LIGHTING);

    glPopMatrix(); 

    // head
    glPushMatrix();
    glTranslatef(0.0f, 17.0f, 0.0f);
    setMat(0.98f, 0.98f, 0.98f, 120.0f);
    glutSolidSphere(3.5f, 26, 26);

    // face
    glDisable(GL_LIGHTING);
    // eyes
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-1.15f, 0.9f, 3.2f); 
    glutSolidSphere(0.85f, 16, 16);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.15f, 0.9f, 3.2f);
    glutSolidSphere(0.85f, 16, 16);
    glPopMatrix();

    // eyes highlight
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.9f, 1.25f, 3.6f);
    glutSolidSphere(0.28f, 8, 8);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.4f, 1.25f, 3.6f);
    glutSolidSphere(0.28f, 8, 8);
    glPopMatrix();

    // rouge
    glColor3f(1.0f, 0.72f, 0.78f);
    glPushMatrix();
    glTranslatef(-1.9f, 0.05f, 3.25f);
    glutSolidSphere(0.62f, 12, 12);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.9f, 0.05f, 3.25f);
    glutSolidSphere(0.62f, 12, 12);
    glPopMatrix();

    // mouth
    glColor3f(0.05f, 0.05f, 0.1f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-1.4f, -0.7f, 3.4f);
    glVertex3f(-0.8f, -1.0f, 3.4f);
    glVertex3f(0.0f, -1.15f, 3.4f);
    glVertex3f(0.8f, -1.0f, 3.4f);
    glVertex3f(1.4f, -0.7f, 3.4f);
    glEnd();

    glEnable(GL_LIGHTING);
    glPopMatrix(); 

    // arm
    glPushMatrix();
    glTranslatef(0.0f, 12.0f, 0.0f);
    setMat(0.86f, 0.88f, 0.95f, 90.0f);

    // left arm
    {
        glPushMatrix();
        glTranslatef(-5.0f, 0.0f, 0.0f);  // left shoulder

        // shoulder joint
        setMat(0.95f, 0.95f, 0.98f, 100.0f);
        glutSolidSphere(1.0f, 16, 16);

        // upper arm
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);  

        glPushMatrix();
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);  
        glutSolidCylinder(0.8f, 3.5f, 20, 6);
        glPopMatrix();

        // move to elbow joint
        glTranslatef(-3.5f, 0.0f, 0.0f);  
        setMat(0.92f, 0.92f, 0.95f, 90.0f);
        glutSolidSphere(0.75f, 14, 14);

        // lower arm
        glRotatef(-20.0f, 0.0f, 0.0f, 1.0f);  

        glPushMatrix();
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);  
        glutSolidCylinder(0.6f, 3.0f, 16, 6);
        glPopMatrix();

        // hands
        glTranslatef(-3.0f, 0.0f, 0.0f);  
        setMat(0.98f, 0.98f, 1.0f, 110.0f);
        glutSolidSphere(0.55f, 14, 14);

        glPopMatrix(); 
    }

    // right arm
    {
        glPushMatrix();
        glTranslatef(5.0f, 0.0f, 0.0f);  // right shoulder

        // shoulder joint
        setMat(0.95f, 0.95f, 0.98f, 100.0f);
        glutSolidSphere(1.0f, 16, 16);

        // upper arm
        glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);  

        glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glutSolidCylinder(0.8f, 3.5f, 20, 6);
        glPopMatrix();

        // move to elbow joint
        glTranslatef(3.5f, 0.0f, 0.0f); 
        setMat(0.92f, 0.92f, 0.95f, 90.0f);
        glutSolidSphere(0.75f, 14, 14);

        // lower arm
        glRotatef(20.0f, 0.0f, 0.0f, 1.0f);  

        glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glutSolidCylinder(0.6f, 3.0f, 16, 6);
        glPopMatrix();

        // hands
        glTranslatef(3.0f, 0.0f, 0.0f);  
        setMat(0.98f, 0.98f, 1.0f, 110.0f);
        glutSolidSphere(0.55f, 14, 14);

        glPopMatrix(); 
    }

    glPopMatrix(); 

    // light
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    setMat(0.45f, 0.85f, 1.0f, 80.0f);
    glPushMatrix();
    glTranslatef(0.0f, 10.0f, 0.0f);
    glScalef(1.1f, 1.25f, 1.0f);
    glutSolidSphere(5.5f, 16, 16);
    glPopMatrix();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    // light
    GLfloat robotLightPos[] = { x, y + 10.0f, z, 1.0f };
    GLfloat robotLightColor[] = { 0.82f, 0.88f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT7, GL_POSITION, robotLightPos);
    glLightfv(GL_LIGHT7, GL_DIFFUSE, robotLightColor);
    glLightfv(GL_LIGHT7, GL_SPECULAR, robotLightColor);
    glLightf(GL_LIGHT7, GL_CONSTANT_ATTENUATION, 0.3f);
    glLightf(GL_LIGHT7, GL_LINEAR_ATTENUATION, 0.05f);

    glPopMatrix(); //root end
}

void drawRobots() {
    static float robotTime = 0.0f;
    robotTime += 0.02f;

    struct RobotParams {
        float x;           // x position
        float currentZ;    // current z position
        float speed;       // speed
        float phaseOffset; // used for animation 
        bool active;       // activation status
    };

    static RobotParams robots[] = {
        // right side
        {130.0f, -1500.0f, 3.0f, 0.0f, true},      
        {135.0f, -1200.0f, 3.5f, 1.5f, true},      
        {140.0f, -800.0f, 4.0f, 3.0f, true},      
        {145.0f, -500.0f, 4.5f, 4.5f, true},      

        // left side
        {-130.0f, -1300.0f, 3.5f, 2.0f, true},     
        {-135.0f, -1000.0f, 4.0f, 3.5f, true},     
        {-145.0f, -600.0f, 4.5f, 5.0f, true},     
        {-140.0f, -300.0f, 5.0f, 6.5f, true}       
    };

    float startZ = -1500.0f;  // start position
    float endZ = 500.0f;      // end position

    // update robots
    for (int i = 0; i < 8; i++) {
        RobotParams& r = robots[i];

        if (r.active) {
            r.currentZ += r.speed;

            if (r.currentZ > endZ) {
                r.currentZ = startZ;
            }

            float tilt = sin(robotTime * 3.0f + r.phaseOffset) * 8.0f;

            drawCuteRobot(r.x, 0.0f, r.currentZ, tilt);
        }
    }
}

// ====================== Lighting ======================
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    GLfloat globalAmb[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

    // Main light
    GLfloat lightPos0[] = { -300.0f, 600.0f, 200.0f, 0.0f };
    GLfloat lightCol0[] = { 1.0f, 0.95f, 0.9f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol0);

    // blue bmbient light
    GLfloat lightPos1[] = { 300.0f, 150.0f, -300.0f, 1.0f };
    GLfloat lightCol1[] = { 0.3f, 0.5f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightCol1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightCol1);

    // train follows light
    GLfloat lightPos2[] = { 0.0f, 140.0f, trainPositionZ, 1.0f };  
    GLfloat lightCol2[] = { 0.8f, 0.9f, 1.0f, 1.0f };  
    glLightfv(GL_LIGHT2, GL_POSITION, lightPos2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightCol2);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lightCol2);
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.01f);

    // drone light
    glEnable(GL_LIGHT6);
    GLfloat upLightPos[] = { 0.0f, 10.0f, 0.0f, 1.0f }; 
    GLfloat upLightDir[] = { 0.0f, 1.0f, 0.0f };        
    GLfloat upLightColor[] = { 0.5f, 0.6f, 0.8f, 1.0f }; 

    glLightfv(GL_LIGHT6, GL_POSITION, upLightPos);
    glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, upLightDir);
    glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, 70.0f);  
    glLightf(GL_LIGHT6, GL_SPOT_EXPONENT, 10.0f); 
    glLightfv(GL_LIGHT6, GL_DIFFUSE, upLightColor);
    glLightfv(GL_LIGHT6, GL_SPECULAR, upLightColor);
    glLightf(GL_LIGHT6, GL_CONSTANT_ATTENUATION, 0.3f);
    glLightf(GL_LIGHT6, GL_LINEAR_ATTENUATION, 0.01f);
}
// ====================== Camera Boundaries ======================
float CAM_MIN_X = -250.0f;
float CAM_MAX_X = 280.0f;
float CAM_MIN_Y = 20.0f;
float CAM_MAX_Y = 300.0f;
float CAM_MIN_Z = -320.0f;
float CAM_MAX_Z = 380.0f;

// ====================== Keyboard Interaction ======================
void keyboard(unsigned char key, int x, int y) {
    float moveSpeed = 15.0f;

    switch (key) {
    case 27: // ESC to exit
        exit(0);
        break;
    case 'r': case 'R': // reset view
        camX = CAM_DEFAULT_X;
        camY = CAM_DEFAULT_Y;
        camZ = CAM_DEFAULT_Z;

        lookX = LOOK_DEFAULT_X;
        lookY = LOOK_DEFAULT_Y;
        lookZ = LOOK_DEFAULT_Z;

        camYaw = YAW_DEFAULT;
        camPitch = PITCH_DEFAULT;
        break;

    // move camera position 
    case 'w': case 'W': // forward
        if (camZ > CAM_MIN_Z) {
            camZ -= moveSpeed;
        }
        break;
    case 's': case 'S': // backworad
        if (camZ < CAM_MAX_Z) {
            camZ += moveSpeed;
        }
        break;
    case 'a': case 'A': // left
        if (camX > CAM_MIN_X) {
            camX -= moveSpeed;
        }
        break;
    case 'd': case 'D': // right
        if (camX < CAM_MAX_X) {
            camX += moveSpeed;
        }
        break;

    case ' ': // rise
        if (camY < CAM_MAX_Y) {
            camY += moveSpeed;
        }
        break;
     }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    float moveSpeed = 15.0f;
    float radYaw = camYaw * M_PI / 180.0f;
    float radPitch = camPitch * M_PI / 180.0f;
    float dirX = cos(radPitch) * sin(radYaw);
    float dirY = sin(radPitch);
    float dirZ = -cos(radPitch) * cos(radYaw);
    
    switch (key) {
    case GLUT_KEY_CTRL_L: case GLUT_KEY_CTRL_R: // Ctrl key downward
        if (camY > CAM_MIN_Y) {
            camY -= moveSpeed;
        }
        break;

    case GLUT_KEY_LEFT:
        camYaw -= 2.0f;
        if (camYaw < YAW_MIN) camYaw = YAW_MIN;
        break;

    case GLUT_KEY_RIGHT:
        camYaw += 2.0f;
        if (camYaw > YAW_MAX) camYaw = YAW_MAX;
        break;

    case GLUT_KEY_UP:
        camPitch += 2.0f;
        if (camPitch > PITCH_MAX) camPitch = PITCH_MAX;   
        break;

    case GLUT_KEY_DOWN:
        camPitch -= 2.0f;
        if (camPitch < PITCH_MIN) camPitch = PITCH_MIN;   
        break;
    }
    glutPostRedisplay();
}


// ====================== Rendering System ======================
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radYaw = camYaw * M_PI / 180.0f;
    float radPitch = camPitch * M_PI / 180.0f;
    float dirX = cos(radPitch) * sin(radYaw);
    float dirY = sin(radPitch);
    float dirZ = -cos(radPitch) * cos(radYaw);

    // update lookup point
    lookX = camX + dirX * 100.0f;   // 100 represents looking forward 100 units
    lookY = camY + dirY * 100.0f;
    lookZ = camZ + dirZ * 100.0f;

    gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, 0, 1, 0);
    setupLighting();

    drawSky();
    drawGround();
    
    // ground light strip
    drawGroundNeonStrip(150.0f, 1.5f, -800.0f, 2500.0f);  
    drawGroundNeonStrip(-150.0f, 1.5f, -800.0f, 2500.0f); 

    // road system
    drawRoadSystem();
    drawStreetLights();
    
    // tree 
    drawGreenBelts();

    // station and track system
    drawTrainStation(-1200.0f);  
    drawParallelRailPair();
    
    // building
    drawSideBuilding(250.0f, -700.0f, 120.0f, 400.0f,120.0f, 0.6f, 0.7f, 0.9f);
    drawSideBuilding(-220.0f, -780.0f, 90.0f, 180.0f, 120.0f);
    drawBuildingClusters();      
    drawDistantRoadBuildings();     
    drawFuturisticVehicles();           

    // holographic billboard 
    drawHolographicBillboard(200.0f, 0.0f, -400.0f);
    drawHolographicBillboard(-180.0f, 0.0f, -600.0f);
    drawHolographicBillboard(-180.0f, 0.0f, -200.0f);

    // robot syatem
    drawRobots(); 

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //train system
    drawMaglevTrains(130.0f, 155.0f);
    drawMaglevTrains(-130.0f, 155.0f);

    // drone
    for (int i = 0; i < droneCount; i++) {
        drawDrone(drones[i]);
    }

    // light system
    setupLighting();
    setupBuildingNeonLights();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, (float)w / (float)h, 1.0, 5000.0);
    glMatrixMode(GL_MODELVIEW);
}

void initGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.85f, 0.9f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
}


// ====================== Main ======================
#include <direct.h>
int main(int argc, char** argv) {
    char buf[256];
    _getcwd(buf, 256);
    printf("Current working directory: %s\n", buf);//find where to put the texture file
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1200, 700);
    glutCreateWindow("Futuristic Metropolis");

    initGL();
    initTextures();  
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutKeyboardFunc(keyboard);      //registering callbacks
    glutSpecialFunc(specialKeys);    

    glutTimerFunc(100, updateTrain, 0); // animation after 100ms
    glutTimerFunc(16, updateDronesTimer, 0);
    glutMainLoop();
    return 0;
}
