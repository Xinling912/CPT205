#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

int WINDOW_WIDTH = 1000;
int WINDOW_HEIGHT = 700;

// Program states
enum ProgramState { SINGLE_BUTTERFLY, MULTIPLE_BUTTERFLIES, SHOW_MESSAGE };
ProgramState currentState = SINGLE_BUTTERFLY;

// Repositioning control flags
bool repositioning = false;
bool prevWasShowMessage = false;

// Text fade-in effect
float textAlpha = 0.0f;
bool messageFullyShown = false;

// Animation variables
float wheelAngle = 0.0f;
float butterflyFloatOffset = 0.0f;
float butterflyFloatSpeed = 0.05f;
float birdJumpPhase = 0.0f;
float birdJumpSpeed = 0.1f;

// Data structures for graphical objects
struct Balloon {
    float x, y;
    float radius;
    float color[3];
    float targetY;
    float speed;
    bool reachedTarget;
    bool active;
};
std::vector<Balloon> balloons;

struct Bird {
    float x, y;
    float wingAngle;
    float jumpOffset;
    float tShirtColor[3];
};
Bird bird;

struct Cloud {
    float xRatio;
    float yRatio;
    float scale;
    float speed;
    float offset;
    float animationPhase;
};
std::vector<Cloud> clouds;

struct Bush {
    float x, y;
    float w, h;
    float horizontalScale;
    float color[3];
};
std::vector<Bush> bushes;

struct Butterfly {
    float x, y;
    float scale;
    float wingAngle;
    float wingSpeed;
    float targetX, targetY;
    float color1[3];
    float color2[3];
    bool reachedTarget;
};
std::vector<Butterfly> butterflies;

float animationProgress = 0.0f;
const float ANIMATION_DURATION = 2.0f;

// Utility functions
void setColor(float c[3], float r, float g, float b) {
    c[0] = r; c[1] = g; c[2] = b;
}

void randomizeButterflyColor(Butterfly& b) {
    int palette = rand() % 7;
    switch (palette) {
    case 0: setColor(b.color1, 0.95f, 0.3f, 0.4f); setColor(b.color2, 1.0f, 0.7f, 0.8f); break;
    case 1: setColor(b.color1, 0.3f, 0.5f, 0.9f); setColor(b.color2, 0.6f, 0.8f, 1.0f); break;
    case 2: setColor(b.color1, 0.6f, 0.2f, 0.8f); setColor(b.color2, 0.9f, 0.6f, 1.0f); break;
    case 3: setColor(b.color1, 0.95f, 0.55f, 0.25f); setColor(b.color2, 1.0f, 0.8f, 0.6f); break;
    case 4: setColor(b.color1, 0.3f, 0.8f, 0.6f); setColor(b.color2, 0.6f, 1.0f, 0.8f); break;
    case 5: setColor(b.color1, 0.98f, 0.6f, 0.8f); setColor(b.color2, 1.0f, 0.85f, 0.9f); break;
    case 6: setColor(b.color1, 0.55f, 0.45f, 0.85f); setColor(b.color2, 0.8f, 0.75f, 1.0f); break;
    }
}

void initBird() {
    bird.x = WINDOW_WIDTH * 0.5f;
    bird.y = 180.0f;
    bird.wingAngle = 0.0f;
    bird.jumpOffset = 0.0f;
    setColor(bird.tShirtColor, 0.1f, 0.2f, 0.5f);
}

void initClouds() {
    clouds.clear();
    for (int i = 0; i < 4; i++) {
        Cloud c;
        c.xRatio = 0.2f + 0.25f * i;
        c.yRatio = 0.75f - 0.05f * i;
        c.scale = 1.0f + (rand() % 100) / 100.0f * 0.5f;
        c.speed = 0.3f + (rand() % 100) / 100.0f * 0.2f;
        c.offset = rand() % 360;
        c.animationPhase = c.offset;
        clouds.push_back(c);
    }
}

void initializeBushes() {
    bushes.clear();
    int numBushes = 12;
    float spacing = WINDOW_WIDTH / float(numBushes + 1);
    for (int i = 0; i < numBushes; i++) {
        Bush b;
        b.x = spacing * (i + 1);
        b.y = 120.0f;
        b.w = 35.0f + rand() % 40;
        b.h = 50.0f + rand() % 40;
        b.horizontalScale = 0.8f + (rand() % 40) / 100.0f;

        int colorType = rand() % 4;
        if (colorType == 0) setColor(b.color, 0.45f, 0.85f, 0.45f);
        else if (colorType == 1) setColor(b.color, 0.35f, 0.75f, 0.55f);
        else if (colorType == 2) setColor(b.color, 0.55f, 0.9f, 0.4f);
        else setColor(b.color, 0.4f, 0.8f, 0.5f);

        bushes.push_back(b);
    }
}

void initializePreclickScreen() {
    // Initialize single butterfly
    butterflies.clear();
    Butterfly b;
    b.x = WINDOW_WIDTH / 2.0f;
    b.y = WINDOW_HEIGHT / 2.0f;
    b.scale = 1.8f;
    b.wingAngle = 0.0f;
    b.wingSpeed = 0.25f;
    setColor(b.color1, 0.9f, 0.2f, 0.4f);
    setColor(b.color2, 1.0f, 0.6f, 0.8f);
    b.reachedTarget = true;
    butterflies.push_back(b);

    // Initialize balloons
    balloons.clear();
    int numBalloons = 6;
    float spacing = WINDOW_WIDTH / (numBalloons + 1);
    for (int i = 0; i < numBalloons; i++) {
        Balloon bl;
        bl.radius = 30.0f + rand() % 15;
        bl.x = (i + 1) * spacing + (rand() % 60 - 30);
        bl.y = 0 - bl.radius;
        bl.targetY = WINDOW_HEIGHT * (0.2f + 0.2f * ((rand() % 10) / 10.0f));
        bl.speed = 3.5f + static_cast<float>(rand()) / RAND_MAX * 1.0f;
        bl.reachedTarget = false;
        bl.active = true;

        int colorType = i % 5;
        if (colorType == 0) setColor(bl.color, 1.0f, 0.5f, 0.5f);
        else if (colorType == 1) setColor(bl.color, 1.0f, 0.8f, 0.4f);
        else if (colorType == 2) setColor(bl.color, 0.5f, 0.7f, 1.0f);
        else if (colorType == 3) setColor(bl.color, 0.7f, 1.0f, 0.7f);
        else setColor(bl.color, 0.9f, 0.6f, 1.0f);

        balloons.push_back(bl);
    }

    initBird();
    initClouds();
    initializeBushes();

    // Reset text state
    textAlpha = 0.0f;
    messageFullyShown = false;
}

void initializeMultipleButterflies() {
    butterflies.clear();
    for (int i = 0; i < 20; i++) {
        Butterfly b;
        b.x = WINDOW_WIDTH / 2.0f;
        b.y = WINDOW_HEIGHT / 2.0f;
        b.scale = 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
        b.wingAngle = 0.0f;
        b.wingSpeed = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.2f;
        randomizeButterflyColor(b);

        float angle = 2.0f * 3.14159f * i / 20;
        float distance = 280.0f + static_cast<float>(rand()) / RAND_MAX * 50.0f;
        b.targetX = b.x + cos(angle) * distance;
        b.targetY = b.y + sin(angle) * distance;
        b.reachedTarget = false;
        butterflies.push_back(b);
    }

    animationProgress = 0.0f;
    textAlpha = 0.0f;
    messageFullyShown = false;
}

// Drawing functions
void drawButterfly(const Butterfly& b) {
    glPushMatrix();
    glTranslatef(b.x, b.y + sin(butterflyFloatOffset) * 5.0f, 0.0f);
    glScalef(b.scale, b.scale, 1.0f);

    // Body
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(-2.0f, -8.0f);
    glVertex2f(2.0f, -8.0f);
    glVertex2f(2.0f, 8.0f);
    glVertex2f(-2.0f, 8.0f);
    glEnd();

    float wingOffset = sin(b.wingAngle) * 12.0f;

    // Left wing with gradient
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(b.color1[0], b.color1[1], b.color1[2]);
    glVertex2f(0.0f, 0.0f);
    glColor3f(b.color2[0], b.color2[1], b.color2[2]);
    glVertex2f(-30.0f - wingOffset, 20.0f);
    glVertex2f(-20.0f - wingOffset, 40.0f);
    glVertex2f(-5.0f, 25.0f);
    glEnd();

    // Right wing with gradient
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(b.color1[0], b.color1[1], b.color1[2]);
    glVertex2f(0.0f, 0.0f);
    glColor3f(b.color2[0], b.color2[1], b.color2[2]);
    glVertex2f(30.0f + wingOffset, 20.0f);
    glVertex2f(20.0f + wingOffset, 40.0f);
    glVertex2f(5.0f, 25.0f);
    glEnd();

    glPopMatrix();
}

void drawBushes() {
    for (const auto& b : bushes) {
        // Bush fill
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(b.color[0], b.color[1], b.color[2]);
        glVertex2f(b.x, b.y);
        for (int j = 0; j <= 180; j += 5) {
            float angle = j * 3.14159f / 180.0f;
            float x = b.x + cos(angle) * b.w * b.horizontalScale;
            float y = b.y + sin(angle) * b.h * 0.6f;
            glVertex2f(x, y);
        }
        glEnd();

        // Bush outline
        glColor3f(0.25f, 0.45f, 0.25f);
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= 180; j += 5) {
            float angle = j * 3.14159f / 180.0f;
            float x = b.x + cos(angle) * b.w * b.horizontalScale;
            float y = b.y + sin(angle) * b.h * 0.6f;
            glVertex2f(x, y);
        }
        glEnd();
    }
}

void drawSun() {
    glPushMatrix();
    float sunX = WINDOW_WIDTH * 0.1f;
    float sunY = WINDOW_HEIGHT * 0.85f;
    float sunR = 50.0f;

    // Sun glow with layered circles
    for (int i = 0; i < 40; i++) {
        float t = static_cast<float>(i) / 40;
        float r = sunR * (1.0f - t * 0.3f);
        float rColor = 1.0f;
        float gColor = 0.9f - t * 0.4f;
        float bColor = 0.3f + t * 0.2f;
        float alpha = 1.0f - t * 0.9f;

        glBegin(GL_TRIANGLE_FAN);
        glColor4f(rColor, gColor, bColor, alpha);
        glVertex2f(sunX, sunY);
        for (int j = 0; j <= 36; j++) {
            float angle = j * (2.0f * 3.14159f / 36.0f);
            glVertex2f(sunX + cos(angle) * r, sunY + sin(angle) * r);
        }
        glEnd();
    }

    // Sun rays
    glLineWidth(3.0f);
    glColor4f(1.0f, 0.9f, 0.3f, 0.5f);
    glBegin(GL_LINES);
    for (int i = 0; i < 12; i++) {
        float angle = 2.0f * 3.14159f * i / 12;
        float inner = sunR * 1.1f;
        float outer = (i % 2 == 0) ? sunR * 1.8f : sunR * 1.6f;
        glVertex2f(sunX + cos(angle) * inner, sunY + sin(angle) * inner);
        glVertex2f(sunX + cos(angle) * outer, sunY + sin(angle) * outer);
    }
    glEnd();
    glPopMatrix();
}

void drawBalloon(const Balloon& bl) {
    if (!bl.active) return;

    // Balloon string
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    glVertex2f(bl.x, bl.y - bl.radius);
    glVertex2f(bl.x, bl.y - bl.radius - 40);
    glEnd();

    // Balloon body
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(bl.color[0], bl.color[1], bl.color[2]);
    glVertex2f(bl.x, bl.y);
    for (int i = 0; i <= 30; i++) {
        float angle = 2.0f * 3.14159f * i / 30;
        glVertex2f(bl.x + cos(angle) * bl.radius, bl.y + sin(angle) * bl.radius);
    }
    glEnd();

    // Balloon highlight
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(bl.x - bl.radius * 0.4f, bl.y + bl.radius * 0.4f);
    for (int i = 0; i <= 15; i++) {
        float angle = 2.0f * 3.14159f * i / 15;
        glVertex2f(bl.x - bl.radius * 0.4f + cos(angle) * 6.0f,
            bl.y + bl.radius * 0.4f + sin(angle) * 6.0f);
    }
    glEnd();
}

void drawCenteredString(const std::string& text, float y, void* font) {
    float textWidth = 0;
    for (char c : text) textWidth += glutBitmapWidth(font, c);
    glRasterPos2f((WINDOW_WIDTH - textWidth) / 2.0f, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

void drawAnniversaryText() {
    glColor4f(0.1f, 0.3f, 0.6f, textAlpha);
    drawCenteredString("XJTLU 20th Anniversary!", WINDOW_HEIGHT / 2 + 20, GLUT_BITMAP_TIMES_ROMAN_24);
    drawCenteredString("2004 - 2024", WINDOW_HEIGHT / 2 - 20, GLUT_BITMAP_HELVETICA_18);
}

void drawSkyWheel(float baseX, float baseY, float scale, float angle) {
    glPushMatrix();
    glTranslatef(baseX, baseY, 0.0f);
    glScalef(scale, scale, 1.0f);

    // Base and supports
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-50, 0);
    glVertex2f(-50, 10);
    glVertex2f(50, 10);
    glVertex2f(50, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-30, 10); glVertex2f(0, 80);
    glVertex2f(30, 10);  glVertex2f(0, 80);
    glEnd();

    // Wheel rings
    float r1 = 45.0f, r2 = 50.0f;
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i += 10)
        glVertex2f(r1 * cos(i * 3.14159f / 180), r1 * sin(i * 3.14159f / 180) + 80);
    glEnd();

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i += 10)
        glVertex2f(r2 * cos(i * 3.14159f / 180), r2 * sin(i * 3.14159f / 180) + 80);
    glEnd();

    // Rotating cabins
    glPushMatrix();
    glTranslatef(0, 80, 0);
    glRotatef(angle, 0, 0, 1);

    for (int i = 0; i < 8; ++i) {
        float theta = i * 2 * 3.14159f / 8;
        float x = 50 * cos(theta);
        float y = 50 * sin(theta);

        // Spoke
        glBegin(GL_LINES);
        glVertex2f(0, 0);
        glVertex2f(x, y);
        glEnd();

        // Cabin with gradient color
        glColor3f(1.0f, 0.8f - 0.1f * i, 0.6f);
        glBegin(GL_POLYGON);
        for (int j = 0; j < 360; j += 30) {
            float cx = x + 4 * cos(j * 3.14159f / 180);
            float cy = y + 4 * sin(j * 3.14159f / 180);
            glVertex2f(cx, cy);
        }
        glEnd();

        glColor3f(1, 1, 1);
    }
    glPopMatrix();
    glPopMatrix();
}

void drawXJTLUBuilding(float baseX, float baseY, float scale) {
    glPushMatrix();
    glTranslatef(baseX, baseY, 0.0f);
    glScalef(scale, scale, 1.0f);

    glLineWidth(2.5f);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Main building structure
    glBegin(GL_LINE_LOOP);
    glVertex2f(165, 95);
    glVertex2f(165, 165);
    glVertex2f(140, 150);
    glVertex2f(115, 118);
    glVertex2f(105, 110);
    glVertex2f(105, 130);
    glVertex2f(140, 160);
    glVertex2f(165, 190);
    glVertex2f(165, 370);
    glVertex2f(105, 303);
    glVertex2f(105, 280);
    glVertex2f(95, 270);
    glVertex2f(85, 284);
    glVertex2f(63, 260);
    glVertex2f(77, 125);
    glVertex2f(82, 120);
    glVertex2f(85, 95);
    glEnd();

    // Architectural details and connections
    glBegin(GL_LINES);
    glVertex2f(88, 280); glVertex2f(107, 299);
    glVertex2f(165, 165); glVertex2f(250, 162);
    glVertex2f(250, 162); glVertex2f(370, 120);
    glVertex2f(370, 120); glVertex2f(369, 95);
    glVertex2f(265, 328); glVertex2f(285, 322);
    glVertex2f(365, 155); glVertex2f(364, 122);
    glEnd();

    // Additional structural elements
    glBegin(GL_LINE_LOOP);
    glVertex2f(165, 370);
    glVertex2f(265, 335);
    glVertex2f(262, 190);
    glVertex2f(257, 185);
    glVertex2f(165, 190);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(285, 328);
    glVertex2f(380, 298);
    glVertex2f(373, 160);
    glVertex2f(367, 154);
    glVertex2f(323, 162);
    glVertex2f(305, 180);
    glVertex2f(283, 220);
    glEnd();

    // Decorative lines
    glBegin(GL_LINES);
    glVertex2f(68, 223); glVertex2f(165, 322);
    glVertex2f(72, 182); glVertex2f(165, 274);
    glVertex2f(76, 140); glVertex2f(165, 226);
    glVertex2f(84, 102); glVertex2f(105, 122);
    glVertex2f(115, 95); glVertex2f(165, 130);
    glVertex2f(165, 322); glVertex2f(264, 294);
    glVertex2f(165, 274); glVertex2f(263, 253);
    glVertex2f(165, 226); glVertex2f(262, 212);
    glVertex2f(165, 130); glVertex2f(370, 109);
    glVertex2f(285, 290); glVertex2f(379, 265);
    glVertex2f(284, 250); glVertex2f(377, 231);
    glVertex2f(290, 208); glVertex2f(375, 191);
    glEnd();

    // Base platform
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 35);
    glVertex2f(0, 40);
    glVertex2f(55, 95);
    glVertex2f(395, 95);
    glVertex2f(395, 70);
    glVertex2f(55, 70);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(5, 0); glVertex2f(5, 38);
    glVertex2f(65, 0); glVertex2f(65, 70);
    glVertex2f(388, 0); glVertex2f(388, 70);
    glEnd();

    glPopMatrix();
}

void drawCloud(const Cloud& c) {
    glPushMatrix();

    float x = WINDOW_WIDTH * c.xRatio;
    float y = WINDOW_HEIGHT * c.yRatio;

    // Cloud floating animation
    float driftX = sin(c.animationPhase) * 25.0f;
    float driftY = cos(c.animationPhase * 1.5f) * 5.0f;

    glTranslatef(x + driftX, y + driftY, 0.0f);
    glScalef(c.scale, c.scale, 1.0f);

    // Cloud body (three overlapping circles)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    for (int i = 0; i <= 360; i += 10) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(cos(angle) * 60.0f, sin(angle) * 35.0f);
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 0; i <= 360; i += 10) {
        float a = i * 3.14159f / 180.0f;
        glVertex2f(-40 + cos(a) * 25, 5 + sin(a) * 20);
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 0; i <= 360; i += 10) {
        float a = i * 3.14159f / 180.0f;
        glVertex2f(40 + cos(a) * 25, 5 + sin(a) * 20);
    }
    glEnd();

    // Cloud outline
    glLineWidth(3.0f);
    glColor3f(0.7f, 0.85f, 1.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 360; i += 10) {
        float a = i * 3.14159f / 180.0f;
        glVertex2f(cos(a) * 60.0f, sin(a) * 35.0f);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 360; i += 10) {
        float a = i * 3.14159f / 180.0f;
        glVertex2f(-40 + cos(a) * 25, 5 + sin(a) * 20);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 360; i += 10) {
        float a = i * 3.14159f / 180.0f;
        glVertex2f(40 + cos(a) * 25, 5 + sin(a) * 20);
    }
    glEnd();

    glPopMatrix();
}

void drawBird() {
    glPushMatrix();
    glTranslatef(bird.x, bird.y + bird.jumpOffset, 0.0f);
    glScalef(2.0f, 2.0f, 1.0f);

    // Body (white circle)
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(0, 0);
    for (int i = 0; i <= 40; i++) {
        float angle = 2.0f * 3.14159f * i / 40;
        glVertex2f(cos(angle) * 18.0f, sin(angle) * 18.0f);
    }
    glEnd();

    // T-shirt
    glBegin(GL_QUADS);
    glColor3f(bird.tShirtColor[0], bird.tShirtColor[1], bird.tShirtColor[2]);
    glVertex2f(-18, -2);
    glVertex2f(18, -2);
    glVertex2f(16, -14);
    glVertex2f(-16, -14);
    glEnd();

    // Animated wings
    float wingFlap = sin(bird.wingAngle) * 8.0f;

    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(-12.0f, 0.0f);
    glVertex2f(-32.0f, -6.0f + wingFlap);
    glVertex2f(-32.0f, 10.0f + wingFlap);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(12.0f, 0.0f);
    glVertex2f(32.0f, -6.0f + wingFlap);
    glVertex2f(32.0f, 10.0f + wingFlap);
    glEnd();

    // Beak
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.6f, 0.1f);
    glVertex2f(-3, 2);
    glVertex2f(3, 2);
    glVertex2f(0, -2);
    glEnd();

    // Eyes
    glColor3f(0.0f, 0.0f, 0.0f);
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glVertex2f(-6, 6);
    glVertex2f(6, 6);
    glEnd();

    // Cheeks
    glColor3f(1.0f, 0.6f, 0.7f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(-10, 2);
    for (int i = 0; i <= 20; i++) {
        float angle = 2.0f * 3.14159f * i / 20;
        glVertex2f(-10 + cos(angle) * 3.0f, 2 + sin(angle) * 2.0f);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(10, 2);
    for (int i = 0; i <= 20; i++) {
        float angle = 2.0f * 3.14159f * i / 20;
        glVertex2f(10 + cos(angle) * 3.0f, 2 + sin(angle) * 2.0f);
    }
    glEnd();

    // Feet
    glColor3f(1.0f, 0.6f, 0.08f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(-6.0f, -14.0f); glVertex2f(-6.0f, -20.0f);
    glVertex2f(-6.0f, -20.0f); glVertex2f(-8.5f, -22.0f);
    glVertex2f(-6.0f, -20.0f); glVertex2f(-4.0f, -22.0f);
    glVertex2f(6.0f, -14.0f); glVertex2f(6.0f, -20.0f);
    glVertex2f(6.0f, -20.0f); glVertex2f(4.0f, -22.0f);
    glVertex2f(6.0f, -20.0f); glVertex2f(8.5f, -22.0f);
    glEnd();

    // Hair
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(0, 18);
    glVertex2f(0, 23);
    glVertex2f(3, 25);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex2f(0, 23);
    glVertex2f(-3, 26);
    glEnd();

    glPopMatrix();
}

void drawBackground() {
    // Sky gradient
    glBegin(GL_QUADS);
    glColor3f(0.6f, 0.85f, 1.0f);
    glVertex2f(0, WINDOW_HEIGHT);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glColor3f(0.3f, 0.65f, 0.95f);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(0, 0);
    glEnd();

    drawSun();

    // Grass gradient
    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.5f, 0.2f);
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glColor3f(0.5f, 0.8f, 0.5f);
    glVertex2f(WINDOW_WIDTH, 120);
    glVertex2f(0, 120);
    glEnd();

    drawBushes();

    for (const auto& c : clouds)
        drawCloud(c);

    drawXJTLUBuilding(WINDOW_WIDTH * 0.1f, 120, 1.0f);
    drawBird();
}

// Main rendering function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawBackground();
    drawSkyWheel(WINDOW_WIDTH * 0.75f, 120.0f, 1.5f, wheelAngle);
    for (const auto& bl : balloons) drawBalloon(bl);
    for (const auto& b : butterflies) drawButterfly(b);

    if (currentState == SHOW_MESSAGE) {
        textAlpha += 0.05f;
        if (textAlpha > 1.0f) textAlpha = 1.0f;
        drawAnniversaryText();
    }

    glutSwapBuffers();
}

// Animation timer function
void timer(int value) {
    // Bird animations
    bird.wingAngle += 0.3f;
    birdJumpPhase += birdJumpSpeed;
    if (birdJumpPhase > 2 * 3.14159f) birdJumpPhase -= 2 * 3.14159f;
    bird.jumpOffset = sin(birdJumpPhase) * 10.0f;

    // Ferris wheel rotation
    wheelAngle += 0.3f;
    if (wheelAngle > 360.0f) wheelAngle -= 360.0f;

    // Butterfly animations
    butterflyFloatOffset += butterflyFloatSpeed;
    if (butterflyFloatOffset > 2 * 3.14159f) butterflyFloatOffset -= 2 * 3.14159f;

    for (auto& b : butterflies) {
        b.wingAngle += b.wingSpeed;
    }

    // Cloud animations
    for (auto& c : clouds) {
        c.animationPhase += c.speed * 0.1f;
    }

    // Butterfly scattering logic
    if (currentState == MULTIPLE_BUTTERFLIES || repositioning) {
        for (auto& b : butterflies) {
            const float moveFactor = 0.08f;
            b.x += (b.targetX - b.x) * moveFactor;
            b.y += (b.targetY - b.y) * moveFactor;
            if (fabs(b.x - b.targetX) < 1.0f && fabs(b.y - b.targetY) < 1.0f)
                b.reachedTarget = true;
        }

        bool allReached = true;
        for (const auto& b : butterflies)
            if (!b.reachedTarget) { allReached = false; break; }

        if (!repositioning) {
            animationProgress += 0.02f;
            if (animationProgress > ANIMATION_DURATION) animationProgress = ANIMATION_DURATION;
            if (animationProgress > ANIMATION_DURATION * 0.3f) currentState = SHOW_MESSAGE;
        }
        else {
            if (allReached) {
                repositioning = false;
                if (prevWasShowMessage) {
                    messageFullyShown = true;
                    textAlpha = 1.0f;
                    currentState = SHOW_MESSAGE;
                }
                else {
                    currentState = MULTIPLE_BUTTERFLIES;
                }
            }
        }
    }

    // Text fade-in animation
    if (currentState == SHOW_MESSAGE && !messageFullyShown) {
        if (textAlpha < 1.0f)
            textAlpha += 0.05f;
        else {
            textAlpha = 1.0f;
            messageFullyShown = true;
        }
    }

    // Balloon animations
    for (auto& bl : balloons) {
        if (!bl.active) continue;
        if (!bl.reachedTarget && currentState == SINGLE_BUTTERFLY) {
            if (bl.y < bl.targetY)
                bl.y += bl.speed;
            else
                bl.reachedTarget = true;
        }
        else if (currentState != SINGLE_BUTTERFLY) {
            bl.y += bl.speed * 1.5f;
            if (bl.y - bl.radius > WINDOW_HEIGHT)
                bl.active = false;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(30, timer, 0);
}

// Mouse interaction handler
void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float clickX = static_cast<float>(x);
        float clickY = WINDOW_HEIGHT - static_cast<float>(y);

        if (currentState == SINGLE_BUTTERFLY) {
            Butterfly& b = butterflies[0];
            float dx = clickX - b.x;
            float dy = clickY - b.y;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance < 80.0f * b.scale) {
                currentState = MULTIPLE_BUTTERFLIES;
                initializeMultipleButterflies();
            }
        }
        else if (currentState == SHOW_MESSAGE && messageFullyShown) {
            Butterfly b;
            b.x = x;
            b.y = WINDOW_HEIGHT - y;
            b.scale = 0.6f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
            b.wingAngle = 0.0f;
            b.wingSpeed = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.2f;
            randomizeButterflyColor(b);
            b.targetX = b.x;
            b.targetY = b.y;
            b.reachedTarget = true;
            butterflies.push_back(b);
        }
    }
}

// Keyboard interaction handler
void keyboard(unsigned char key, int, int) {
    if (key == 27) exit(0);  // ESC to exit
    if (key == 'r' || key == 'R') {
        currentState = SINGLE_BUTTERFLY;
        initializePreclickScreen();
    }
    if (key == 'c' || key == 'C') {
        for (auto& b : butterflies)
            randomizeButterflyColor(b);
    }
}

// Window resize handler
void reshape(int w, int h) {
    if (h == 0) h = 1;
    WINDOW_WIDTH = w;
    WINDOW_HEIGHT = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Reposition objects based on new window size
    if (currentState == SINGLE_BUTTERFLY) {
        initializePreclickScreen();
        repositioning = false;
        prevWasShowMessage = false;
    }
    else if (currentState == MULTIPLE_BUTTERFLIES || currentState == SHOW_MESSAGE) {
        float centerX = WINDOW_WIDTH / 2.0f;
        float centerY = WINDOW_HEIGHT / 2.0f;
        prevWasShowMessage = (currentState == SHOW_MESSAGE);

        int total = butterflies.size();
        for (int i = 0; i < total; i++) {
            float angle = 2.0f * 3.14159f * i / total;
            float distance = 250.0f + static_cast<float>(rand()) / RAND_MAX * 70.0f;
            butterflies[i].targetX = centerX + cos(angle) * distance;
            butterflies[i].targetY = centerY + sin(angle) * distance;
            butterflies[i].reachedTarget = false;
        }
        repositioning = true;
    }

    // Adjust balloon positions
    if (currentState == SINGLE_BUTTERFLY) {
        int num = balloons.size();
        for (int i = 0; i < num; i++) {
            balloons[i].x = WINDOW_WIDTH * (0.15f + 0.14f * i);
            balloons[i].targetY = WINDOW_HEIGHT * (0.2f + 0.2f * ((rand() % 10) / 10.0f));
            if (!balloons[i].reachedTarget)
                balloons[i].y = std::min(balloons[i].y, balloons[i].targetY);
        }
    }

    bird.x = WINDOW_WIDTH * 0.5f;
    initializeBushes();
    glutPostRedisplay();
}

// Initialization function
void init() {
    glClearColor(0.9f, 0.95f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    srand((unsigned int)time(NULL));
    initializePreclickScreen();
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("XJTLU 20th Anniversary Greeting Card");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseClick);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(30, timer, 0);

    // User instructions
    printf("=== XJTLU 20th Anniversary Greeting Card ===\n");
    printf("Instructions:\n");
    printf("1. Click the butterfly to start animation\n");
    printf("2. After message shows, click anywhere to add butterflies\n");
    printf("3. Press 'C' to change butterfly colors\n");
    printf("4. Press 'R' to reset\n");
    printf("5. Press 'ESC' to exit\n");

    glutMainLoop();
    return 0;
}