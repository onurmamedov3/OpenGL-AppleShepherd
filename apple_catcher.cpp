#include <GL/freeglut.h>
#include <GL/glu.h>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
#include <sstream> 
#include <limits> 

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float BASKET_WIDTH = 150.0f;
const float BASKET_HEIGHT = 80.0f;
const float APPLE_RADIUS = 18.0f;
const float APPLE_SPEED = 4.0f;
const int MAX_FALLING_APPLES = 8;
const int WIN_SCORE = 20;
const int LOSE_LIMIT = 10;
const int CLOUD_COUNT = 3;
const float APPLE_SPAWN_INTERVAL = 1.2f;
const float MIN_APPLE_DISTANCE = 100.0f;
const float GROUND_LEVEL = 60.0f;
#ifndef M_PI
const float M_PI = 3.14159265358979323846f;
#endif

const int NUM_SHEEP = 2;
const float SHEEP_SPEED = 1.5f;
const float SHEEP_DETECT_RANGE = 250.0f;
const float SHEEP_EAT_DURATION = 1.2f;
const float SHEEP_REACH_THRESHOLD = 10.0f;
const float SHEEP_BODY_RADIUS_X = 25.0f; 
const float SHEEP_BODY_RADIUS_Y = 18.0f; 
const float SHEEP_HEAD_RADIUS = 10.0f;  

float basketX = WINDOW_WIDTH / 2 - BASKET_WIDTH / 2;
float basketY = GROUND_LEVEL;
int score = 0;
int missed = 0;
bool gameRunning = true;
bool gameOver = false;
bool gameWon = false;
float lastAppleSpawn = 0;
int activeFallingAppleCount = 0;
float lastTime = 0.0f; 

struct Apple {
    float x, y;
    bool active;
    bool fallen;
    bool targetedBySheep;
    float speed;
};

struct Cloud {
    float x, y;
    float speed;
    float scale; 
    float partSizes[5] = {0.12f, 0.15f, 0.19f, 0.15f, 0.12f}; 
};

enum class SheepState {
    WANDERING,
    MOVING_TO_APPLE,
    EATING
};

struct Sheep {
    float x, y;             
    float targetX;          
    float speed;
    SheepState state;
    int targetAppleIndex;   
    float eatTimer;         
    bool movingRight;       
    float bobOffset;        
    float scale;            
};

std::vector<Apple> apples(MAX_FALLING_APPLES + LOSE_LIMIT + 10); 
std::vector<Cloud> clouds(CLOUD_COUNT);
std::vector<Sheep> sheeps(NUM_SHEEP);

void drawFilledCircle(float centerX, float centerY, float radius) {
    const int numSegments = 32;
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
        for (int i = 0; i <= numSegments; i++) {
            float angle = 2.0f * M_PI * i / numSegments;
            glVertex2f(centerX + radius * cosf(angle), centerY + radius * sinf(angle));
        }
    glEnd();
}

void drawFilledEllipse(float cx, float cy, float rx, float ry) {
    const int num_segments = 40;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy); 
    for(int i = 0; i <= num_segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(num_segments); 
        float x = rx * cosf(theta); 
        float y = ry * sinf(theta); 
        glVertex2f(x + cx, y + cy); 
    }
    glEnd();
}

void drawSemiCircle(float cx, float cy, float r, int start_angle, int end_angle) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx,cy);
    for(int i = start_angle; i <= end_angle; i++) {
        float angle_rad = i * M_PI / 180.0f;
        glVertex2f(cx + cosf(angle_rad) * r, cy + sinf(angle_rad) * r);
    }
    glEnd();
}

void drawSun(float x, float y, float radius) {
    glColor3f(1.0f, 1.0f, 0.0f); 
    drawFilledCircle(x, y, radius);
    glColor3f(1.0f, 0.8f, 0.0f); 
    glLineWidth(2.0f);
    for (int i = 0; i < 12; ++i) {
        float angle = 2.0f * M_PI * i / 12.0f;
        float startX = x + radius * 1.1f * cosf(angle);
        float startY = y + radius * 1.1f * sinf(angle);
        float endX = x + radius * 1.5f * cosf(angle);
        float endY = y + radius * 1.5f * sinf(angle);
        glBegin(GL_LINES);
            glVertex2f(startX, startY); glVertex2f(endX, endY);
        glEnd();
    }
}

void drawFlower(float x, float y, float scale, float r_p, float g_p, float b_p, float r_c, float g_c, float b_c) {

    glColor3f(0.1f, 0.6f, 0.1f); 
    glLineWidth(3.0f * scale);
    glBegin(GL_LINES); glVertex2f(x, y); glVertex2f(x, y + 25.0f * scale); glEnd();

    glColor3f(r_c, g_c, b_c); 
    drawFilledCircle(x, y + 25.0f * scale, 8.0f * scale);

    glColor3f(r_p, g_p, b_p); 
    float petalRadius = 12.0f * scale, centerOffset = 8.0f * scale;
    for (int i = 0; i < 5; ++i) {
        float angle = 2.0f * M_PI * i / 5.0f + (M_PI / 10.0f); 
        drawFilledCircle(x + centerOffset * cosf(angle), y + 25.0f * scale + centerOffset * sinf(angle), petalRadius);
    }
}

void drawMushroom(float x, float y, float scale) {
    float stemW = 20.0f*scale, stemH = 30.0f*scale, capR = 25.0f*scale, capH = 20.0f*scale; 

    glColor3f(0.95f, 0.9f, 0.8f); 
    glBegin(GL_QUADS); glVertex2f(x-stemW/2,y); glVertex2f(x+stemW/2,y); glVertex2f(x+stemW/2,y+stemH); glVertex2f(x-stemW/2,y+stemH); glEnd();

    glColor3f(1.0f, 0.1f, 0.1f); 
    drawSemiCircle(x, y + stemH - capH / 2, capR, 0, 180); 

    glColor3f(1.0f, 1.0f, 1.0f); 
    drawFilledCircle(x, y + stemH + capH*0.2f, 5.0f*scale);
    drawFilledCircle(x - capR*0.5f, y + stemH, 4.0f*scale);
    drawFilledCircle(x + capR*0.5f, y + stemH, 4.5f*scale);
}

void drawCloud(const Cloud& cloud) {
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f); 

    drawFilledCircle(cloud.x - 0.2f*cloud.scale, cloud.y + 0.03f*cloud.scale, cloud.partSizes[0]*cloud.scale);
    drawFilledCircle(cloud.x - 0.1f*cloud.scale, cloud.y + 0.06f*cloud.scale, cloud.partSizes[1]*cloud.scale);
    drawFilledCircle(cloud.x                      , cloud.y + 0.08f*cloud.scale, cloud.partSizes[2]*cloud.scale);
    drawFilledCircle(cloud.x + 0.1f*cloud.scale, cloud.y + 0.06f*cloud.scale, cloud.partSizes[3]*cloud.scale);
    drawFilledCircle(cloud.x + 0.2f*cloud.scale, cloud.y + 0.03f*cloud.scale, cloud.partSizes[4]*cloud.scale);
}

void drawSheep(const Sheep& sheep) {

    float scale = sheep.scale;
    float bodyRadiusX = SHEEP_BODY_RADIUS_X * scale;
    float bodyRadiusY = SHEEP_BODY_RADIUS_Y * scale;
    float headRadius = SHEEP_HEAD_RADIUS * scale;
    float legLength = 15.0f * scale;
    float legWidth = 4.0f * scale; 

    float bodyBaseY = sheep.y; 
    float bobY = bodyBaseY + sheep.bobOffset; 

    float headXOffset = sheep.movingRight ? bodyRadiusX * 0.8f : -bodyRadiusX * 0.8f; 
    float headBaseY = bobY + bodyRadiusY * 0.5f; 

    glColor3f(0.2f, 0.2f, 0.2f); 

    glRectf(sheep.x - bodyRadiusX * 0.5f - legWidth/2, bodyBaseY - legLength,
            sheep.x - bodyRadiusX * 0.5f + legWidth/2, bodyBaseY);
    glRectf(sheep.x + bodyRadiusX * 0.5f - legWidth/2, bodyBaseY - legLength,
            sheep.x + bodyRadiusX * 0.5f + legWidth/2, bodyBaseY);

    glRectf(sheep.x - bodyRadiusX * 0.2f - legWidth/2, bodyBaseY - legLength,
            sheep.x - bodyRadiusX * 0.2f + legWidth/2, bodyBaseY);
    glRectf(sheep.x + bodyRadiusX * 0.2f - legWidth/2, bodyBaseY - legLength,
            sheep.x + bodyRadiusX * 0.2f + legWidth/2, bodyBaseY);

    glColor3f(0.95f, 0.95f, 0.95f); 
    drawFilledEllipse(sheep.x, bobY, bodyRadiusX, bodyRadiusY); 

    drawFilledCircle(sheep.x                      , bobY + bodyRadiusY * 0.8f, bodyRadiusX * 0.5f); 
    drawFilledCircle(sheep.x - bodyRadiusX * 0.6f, bobY + bodyRadiusY * 0.3f, bodyRadiusX * 0.4f); 
    drawFilledCircle(sheep.x + bodyRadiusX * 0.6f, bobY + bodyRadiusY * 0.3f, bodyRadiusX * 0.4f); 

    float currentHeadY = headBaseY;
    if (sheep.state == SheepState::EATING) {
        currentHeadY -= 10.0f * scale; 
    }

    glColor3f(0.85f, 0.85f, 0.85f); 
    drawFilledEllipse(sheep.x + headXOffset, currentHeadY, headRadius, headRadius * 0.8f); 

    glColor3f(0.1f, 0.1f, 0.1f); 
    float eyeXOffset = headRadius * 0.4f * (sheep.movingRight ? 1 : -1); 
    float eyeYOffset = headRadius * 0.2f;                                
    drawFilledCircle(sheep.x + headXOffset + eyeXOffset, currentHeadY + eyeYOffset, 1.5f * scale); 
}

void drawBasket() {

    glColor3f(0.75f, 0.55f, 0.35f); 
    glBegin(GL_QUADS);
        glVertex2f(basketX - 10, basketY);                     
        glVertex2f(basketX + BASKET_WIDTH + 10, basketY);      
        glVertex2f(basketX + BASKET_WIDTH - 20, basketY + BASKET_HEIGHT); 
        glVertex2f(basketX + 20, basketY + BASKET_HEIGHT);     
    glEnd();

    glColor3f(0.4f, 0.3f, 0.2f); 
    glLineWidth(5.0f);
    glBegin(GL_LINES);

        glVertex2f(basketX + 30, basketY + BASKET_HEIGHT);
        glVertex2f(basketX + 20, basketY + BASKET_HEIGHT + 30);

        glVertex2f(basketX + BASKET_WIDTH - 30, basketY + BASKET_HEIGHT);
        glVertex2f(basketX + BASKET_WIDTH - 20, basketY + BASKET_HEIGHT + 30);

        glVertex2f(basketX + 20, basketY + BASKET_HEIGHT + 30);
        glVertex2f(basketX + BASKET_WIDTH - 20, basketY + BASKET_HEIGHT + 30);
    glEnd();
}

void drawApple(float x, float y) {

    glColor3f(0.8f, 0.0f, 0.0f); 
    drawFilledCircle(x, y, APPLE_RADIUS);

    glColor3f(1.0f, 0.9f, 0.8f); 
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x - APPLE_RADIUS*0.4f, y + APPLE_RADIUS*0.4f); 
        for (int i = 0; i <= 12; ++i) { 
            float angle = M_PI*0.5f + (M_PI*i/12.0f);
            glVertex2f(x-APPLE_RADIUS*0.4f+cosf(angle)*APPLE_RADIUS*0.3f, y+APPLE_RADIUS*0.4f+sinf(angle)*APPLE_RADIUS*0.4f);
        }
    glEnd();

    glColor3f(0.35f, 0.2f, 0.1f); 
    glLineWidth(2.5f);
    glBegin(GL_LINES);
        glVertex2f(x, y+APPLE_RADIUS*0.9f); 
        glVertex2f(x+4, y+APPLE_RADIUS+8);  
    glEnd();
}

void drawGround() {
    glColor3f(0.2f, 0.6f, 0.2f); 
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(WINDOW_WIDTH, GROUND_LEVEL);
        glVertex2f(0, GROUND_LEVEL);
    glEnd();
}

void initGame() {
    srand(static_cast<unsigned>(time(0)));

    basketX = WINDOW_WIDTH / 2 - BASKET_WIDTH / 2;
    basketY = GROUND_LEVEL;

    score = 0;
    missed = 0;
    gameOver = false;
    gameWon = false;
    gameRunning = true;
    activeFallingAppleCount = 0;
    lastAppleSpawn = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; 
    lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; 

    for (auto& apple : apples) {
        apple.active = false;
        apple.fallen = false;
        apple.targetedBySheep = false;
    }

    clouds[0] = {100.0f, 500.0f, 0.3f, 80.0f, {0.12f, 0.15f, 0.19f, 0.15f, 0.12f}};
    clouds[1] = {300.0f, 450.0f, 0.4f, 120.0f, {0.12f, 0.15f, 0.19f, 0.15f, 0.12f}};
    clouds[2] = {600.0f, 480.0f, 0.5f, 160.0f, {0.12f, 0.15f, 0.19f, 0.15f, 0.12f}};

    for (int i = 0; i < NUM_SHEEP; ++i) {
        sheeps[i].x = (float)(rand() % (WINDOW_WIDTH - 100)) + 50.0f; 
        sheeps[i].y = GROUND_LEVEL + 10.0f; 
        sheeps[i].speed = SHEEP_SPEED * (0.8f + (rand() % 41) / 100.0f); 
        sheeps[i].state = SheepState::WANDERING;
        sheeps[i].targetAppleIndex = -1;
        sheeps[i].eatTimer = 0.0f;
        sheeps[i].targetX = (float)(rand() % (WINDOW_WIDTH - 100)) + 50.0f; 
        sheeps[i].movingRight = (sheeps[i].targetX > sheeps[i].x);
        sheeps[i].bobOffset = (float)(rand() % 5) - 2.0f; 
        sheeps[i].scale = 1.0f; 
    }
}

void update(int value) {
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    float deltaTime = currentTime - lastTime;

    if (deltaTime > 0.1f) deltaTime = 0.016f; 
    if (deltaTime <= 0) deltaTime = 0.016f; 

    if (gameRunning && !gameOver) {

        if (currentTime - lastAppleSpawn > APPLE_SPAWN_INTERVAL && activeFallingAppleCount < MAX_FALLING_APPLES) {
            int applesToSpawn = 1 + (rand() % 2); 
            for (int spawnAttempt = 0; spawnAttempt < applesToSpawn && activeFallingAppleCount < MAX_FALLING_APPLES; ++spawnAttempt) {

                int appleIndex = -1;
                for(int i = 0; i < apples.size(); ++i) {
                    if (!apples[i].active) {
                        appleIndex = i;
                        break;
                    }
                }
                if (appleIndex == -1) break; 

                float newX;
                bool positionValid = false;
                int attempts = 0;
                while (!positionValid && attempts < 10) {
                    positionValid = true;
                    newX = rand() % (WINDOW_WIDTH - 100) + 50; 
                    attempts++;

                    for (int i=0; i < apples.size(); ++i) {
                        if (i == appleIndex || !apples[i].active || apples[i].fallen) continue;
                        if (abs(apples[i].x - newX) < MIN_APPLE_DISTANCE) {
                            positionValid = false;
                            break;
                        }
                    }
                }

                if (positionValid) {
                    apples[appleIndex].x = newX;
                    apples[appleIndex].y = WINDOW_HEIGHT + APPLE_RADIUS; 
                    apples[appleIndex].speed = APPLE_SPEED * (0.9f + (rand() % 21)/100.0f); 
                    apples[appleIndex].active = true;
                    apples[appleIndex].fallen = false;
                    apples[appleIndex].targetedBySheep = false;
                    activeFallingAppleCount++;
                    lastAppleSpawn = currentTime; 
                }
            }
        }

        for (int i = 0; i < apples.size(); ++i) {
            auto& apple = apples[i];
            if (apple.active && !apple.fallen) { 
                apple.y -= apple.speed * (deltaTime / 0.016f); 

                if (apple.x + APPLE_RADIUS >= basketX - 10 &&             
                    apple.x - APPLE_RADIUS <= basketX + BASKET_WIDTH + 10 && 
                    apple.y - APPLE_RADIUS <= basketY + BASKET_HEIGHT &&     
                    apple.y + APPLE_RADIUS >= basketY)                       
                {
                    score++;
                    apple.active = false; 
                    activeFallingAppleCount--;

                    if(apple.targetedBySheep) {
                        for(auto& s : sheeps) {
                            if(s.targetAppleIndex == i) {
                                s.state = SheepState::WANDERING;
                                s.targetAppleIndex = -1;
                                s.targetX = (float)(rand() % (WINDOW_WIDTH - 100)) + 50.0f; 
                                s.movingRight = (s.targetX > s.x);
                            }
                        }
                    }

                    if (score >= WIN_SCORE) {
                         gameWon = true;
                         gameOver = true;
                    }
                }

                else if (apple.y - APPLE_RADIUS < GROUND_LEVEL) {
                    apple.y = GROUND_LEVEL + APPLE_RADIUS; 
                    apple.fallen = true; 
                    missed++;
                    activeFallingAppleCount--;
                    if (missed >= LOSE_LIMIT) {
                        gameOver = true;
                        gameWon = false;
                    }
                }
            }
        } 

        for (auto& sheep : sheeps) {

            sheep.bobOffset = sinf(currentTime * 3.0f + sheep.x * 0.1f) * 1.5f;

            switch (sheep.state) {
                case SheepState::WANDERING: {

                    int bestAppleIndex = -1;
                    float minDistanceSq = SHEEP_DETECT_RANGE * SHEEP_DETECT_RANGE; 

                    for (int i = 0; i < apples.size(); ++i) {

                        if (apples[i].active && apples[i].fallen && !apples[i].targetedBySheep) {
                            float dx = apples[i].x - sheep.x;
                            float dy = apples[i].y - sheep.y; 
                            float distSq = dx * dx + dy * dy;

                            if (distSq < minDistanceSq) {
                                minDistanceSq = distSq;
                                bestAppleIndex = i;
                            }
                        }
                    }

                    if (bestAppleIndex != -1) {
                        sheep.targetAppleIndex = bestAppleIndex;
                        apples[bestAppleIndex].targetedBySheep = true; 
                        sheep.state = SheepState::MOVING_TO_APPLE;

                    } else {

                        float dx = sheep.targetX - sheep.x;
                        if (abs(dx) < SHEEP_REACH_THRESHOLD) {

                            sheep.targetX = (float)(rand() % (WINDOW_WIDTH - 100)) + 50.0f;
                            sheep.movingRight = (sheep.targetX > sheep.x);
                        } else {

                            sheep.x += sheep.speed * (sheep.movingRight ? 1.0f : -1.0f) * (deltaTime / 0.016f);
                        }
                    }
                    break;
                } 

                case SheepState::MOVING_TO_APPLE: {

                    if (sheep.targetAppleIndex < 0 || sheep.targetAppleIndex >= apples.size() ||
                        !apples[sheep.targetAppleIndex].active || !apples[sheep.targetAppleIndex].fallen)
                    {

                        sheep.state = SheepState::WANDERING;
                         if(sheep.targetAppleIndex >= 0 && sheep.targetAppleIndex < apples.size()) {
                            apples[sheep.targetAppleIndex].targetedBySheep = false; 
                         }
                        sheep.targetAppleIndex = -1;
                        sheep.targetX = (float)(rand() % (WINDOW_WIDTH - 100)) + 50.0f; 
                        sheep.movingRight = (sheep.targetX > sheep.x);
                    } else {

                        float appleX = apples[sheep.targetAppleIndex].x;
                        float dx = appleX - sheep.x;
                        sheep.movingRight = (dx > 0); 

                        if (abs(dx) < SHEEP_REACH_THRESHOLD) {

                            sheep.state = SheepState::EATING;
                            sheep.eatTimer = SHEEP_EAT_DURATION;
                        } else {

                            sheep.x += sheep.speed * (sheep.movingRight ? 1.0f : -1.0f) * (deltaTime / 0.016f);
                        }
                    }
                    break;
                } 

                case SheepState::EATING: {

                    if (sheep.targetAppleIndex < 0 || sheep.targetAppleIndex >= apples.size() ||
                        !apples[sheep.targetAppleIndex].active || !apples[sheep.targetAppleIndex].fallen)
                    {

                        sheep.state = SheepState::WANDERING;
                        if(sheep.targetAppleIndex >= 0 && sheep.targetAppleIndex < apples.size()) {
                            apples[sheep.targetAppleIndex].targetedBySheep = false; 
                        }
                        sheep.targetAppleIndex = -1;
                        sheep.targetX = (float)(rand() % (WINDOW_WIDTH - 100)) + 50.0f;
                        sheep.movingRight = (sheep.targetX > sheep.x);
                    } else {

                        sheep.eatTimer -= deltaTime; 
                        if (sheep.eatTimer <= 0.0f) {

                            apples[sheep.targetAppleIndex].active = false; 
                            apples[sheep.targetAppleIndex].targetedBySheep = false; 

                            sheep.scale += 0.1f; 

                            const float MAX_SHEEP_SCALE = 2.5f;
                            if (sheep.scale > MAX_SHEEP_SCALE) {
                                sheep.scale = MAX_SHEEP_SCALE;
                            }

                            sheep.state = SheepState::WANDERING;
                            sheep.targetAppleIndex = -1;
                            sheep.targetX = (float)(rand() % (WINDOW_WIDTH - 100)) + 50.0f; 
                            sheep.movingRight = (sheep.targetX > sheep.x);
                        }
                    }
                    break;
                } 
            } 

            float boundaryX = SHEEP_BODY_RADIUS_X * sheep.scale; 
            if (sheep.x < boundaryX) {
                 sheep.x = boundaryX;

                 if (sheep.state == SheepState::WANDERING) {
                    sheep.targetX = (float)(rand() % (WINDOW_WIDTH / 2)) + (WINDOW_WIDTH / 2 - 50.0f); 
                    sheep.movingRight = true;
                 }
            }
            if (sheep.x > WINDOW_WIDTH - boundaryX) {
                sheep.x = WINDOW_WIDTH - boundaryX;

                 if (sheep.state == SheepState::WANDERING) {
                    sheep.targetX = (float)(rand() % (WINDOW_WIDTH / 2)) + 50.0f; 
                    sheep.movingRight = false;
                 }
            }

        } 

        for (auto& cloud : clouds) {
            cloud.x += cloud.speed * (deltaTime / 0.016f); 

            if (cloud.x - cloud.scale*0.2f > WINDOW_WIDTH) { 
                 cloud.x = -cloud.scale*1.5f; 
                 cloud.y = WINDOW_HEIGHT - 200 + rand()%150; 
            }

            cloud.y += sinf(currentTime * (0.4f + cloud.speed)) * 0.15f * (deltaTime / 0.016f);
        }

        lastTime = currentTime;

    } 

    glutPostRedisplay(); 
    glutTimerFunc(16, update, 0); 
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity(); 

    glBegin(GL_QUADS);
        glColor3f(0.53f, 0.81f, 0.92f); 
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.75f, 0.88f, 0.98f); 
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
    glEnd();

    drawSun(WINDOW_WIDTH - 100, WINDOW_HEIGHT - 100, 50); 
    for (const auto& cloud : clouds) drawCloud(cloud);
    drawGround();

    drawFlower(100, GROUND_LEVEL, 1.0f, 1.0f, 0.6f, 0.8f, 1.0f, 1.0f, 0.2f); 
    drawFlower(150, GROUND_LEVEL, 0.8f, 0.8f, 0.3f, 1.0f, 1.0f, 0.5f, 0.0f); 
    drawMushroom(WINDOW_WIDTH - 150, GROUND_LEVEL, 1.1f);                 
    drawFlower(WINDOW_WIDTH - 220, GROUND_LEVEL, 0.9f, 1.0f, 0.4f, 0.4f, 1.0f, 1.0f, 1.0f); 

    for (const auto& apple : apples) {
        if (apple.active) { 
            drawApple(apple.x, apple.y);
        }
    }

    for (const auto& sheep : sheeps) {
        drawSheep(sheep);
    }

    drawBasket();

    glColor3f(0.0f, 0.0f, 0.0f); 
    glRasterPos2f(10, WINDOW_HEIGHT - 30); 
    std::string scoreStr = "Apples: " + std::to_string(score) + "/" + std::to_string(WIN_SCORE) +
                           "    Missed: " + std::to_string(missed) + "/" + std::to_string(LOSE_LIMIT);
    for (char c : scoreStr) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

     if (gameOver || !gameRunning) {

        glColor4f(0.0f, 0.0f, 0.0f, 0.5f); 
        glBegin(GL_QUADS);
            glVertex2f(0,0); glVertex2f(WINDOW_WIDTH,0);
            glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glVertex2f(0,WINDOW_HEIGHT);
        glEnd();

        std::string message;
        if(gameOver) {
             glColor3f(1.0f, 1.0f, 0.0f); 
             message = gameWon ? "YOU WIN! Press R to Restart" : "GAME OVER! Press R to Restart";
        } else { 
             glColor3f(1.0f, 1.0f, 1.0f); 
             message = "PAUSED - Press P to Resume";
        }

        float textWidth = 0;
        for(char c:message) {
             textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        }
        glRasterPos2f(WINDOW_WIDTH/2.0f - textWidth/2.0f, WINDOW_HEIGHT/2.0f); 

        for (char c : message) {
             glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }

    glutSwapBuffers(); 
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: 
            exit(0);
            break;
        case 'r':
        case 'R':
            initGame(); 
            break;
        case 'p':
        case 'P':
            if (!gameOver) { 
                gameRunning = !gameRunning;
                 if (gameRunning) {

                    lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                 }
            }
            break;
    }
}

void specialKeyboard(int key, int x, int y) {
    if (gameRunning && !gameOver) { 
        float moveSpeedH = 25.0f; 
        float moveSpeedV = 20.0f; 
        float basketTopLimit = WINDOW_HEIGHT / 1.5f; 
        float basketBottomLimit = GROUND_LEVEL;      

        switch (key) {
            case GLUT_KEY_LEFT:
                basketX -= moveSpeedH;
                if (basketX < 0) basketX = 0; 
                break;
            case GLUT_KEY_RIGHT:
                basketX += moveSpeedH;

                if (basketX + BASKET_WIDTH + 10 > WINDOW_WIDTH) basketX = WINDOW_WIDTH - BASKET_WIDTH - 10;
                break;
            case GLUT_KEY_UP:
                basketY += moveSpeedV;

                if (basketY + BASKET_HEIGHT + 30 > basketTopLimit) basketY = basketTopLimit - BASKET_HEIGHT - 30;
                break;
            case GLUT_KEY_DOWN:
                basketY -= moveSpeedV;
                if (basketY < basketBottomLimit) basketY = basketBottomLimit; 
                break;
        }
    }
}

void reshape(int w, int h) {
    if (h == 0) h = 1; 
    glViewport(0, 0, w, h); 

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100); 
    glutCreateWindow("Apple Catcher Game - Sheep Grow!"); 

    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    initGame(); 

    glutDisplayFunc(display);      
    glutReshapeFunc(reshape);      
    glutKeyboardFunc(keyboard);    
    glutSpecialFunc(specialKeyboard); 
    glutTimerFunc(0, update, 0);   

    glutMainLoop(); 
    return 0;
}