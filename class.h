#include <allegro5/allegro.h>
#include "window.h"

struct Element {
    // default element structure for every element on screen
    float           x = 0,
                    x_temp = 0;
    float           y = 0,
                    y_temp = 0;
    int             dirX,
                    dirX_temp;
    int             dirY,
                    dirY_temp;
    float           velX,
                    velY;
    std::string     id;

    void resetPosition() { // reset element position to initial
        x = x_temp;
        y = y_temp;
    }
    void setPosition(float tx, float ty) { // set position
        x = x + tx;
        y = y + ty;
    }
    void startScrolling() { // start scroll
        if(dirX_temp != 0) { // when have temp dirX
            dirX = dirX_temp;
            dirX_temp = 0;
        }
    }
    void stopScrolling() { // stop scroll
        if(dirX != 0) { // when have dirX
            dirX_temp = dirX;
            dirX = 0;
        }
    }
    void setVelocity(float tvelX, float tvelY, int tdirX, int tdirY) { // set velocity
        velX = tvelX;
        velY = tvelY;
        dirX = tdirX;
        dirY = tdirY;
    }
    void initialize(float tx, float ty, float tvelX = 0, float tvelY = 0, int tdirX = 0, int tdirY = 0) { // iniitalize element
        x_temp = tx;
        y_temp = ty;
        dirX_temp = 0;
        dirY_temp = 0;
        setPosition(tx, ty);
        setVelocity(tvelX, tvelY, tdirX, tdirY);
    }
};

struct Image {
    float           W,
                    H;
    int             parallax;
    ALLEGRO_BITMAP  *source;

    void destroy() { al_destroy_bitmap(source); }
};

struct Game {
    int     state;
    float   score,
            highScore = 0;
    float   currentSpeed,
            acceleration,
            acceleration_perscore;
    int     elapsed_before = 0,
            elapsed = 0;
    float   score_increment = 1,
            score_multiplexer = 1;
    bool    isWindowRedraw = false,
            exitedProcess = false;

    int isScoreAdded() { return elapsed_before != elapsed; }
    bool isExited() { return exitedProcess == true; }
    void exitGame() { exitedProcess = true; }
    bool isRedraw() { return isWindowRedraw; }
    bool isPlaying() { return state == 2; }
    void redrawState(bool state = true) {
        isWindowRedraw = state; // just change state for current redraw function
    }
    void stateGameover() {
        state = 3;
        if(score > highScore) highScore = score;
        score = 0;
    }
    void addScore() {
        elapsed_before = elapsed;
        elapsed++;
        score += score_increment * score_multiplexer;
    }
};

struct Background {
    Image       sprites;
    Element     pos;
};

struct Branch {
    int         live = 0;

    void setLive() { live = 1; }
    void removeElement() { live = 0; }
    void draw() { setLive(); pos.x = WIDTH + 10; }
    void drawBranch() {
        draw();
        if(rand()%2 == 0)
            pos.y = 0 - rand()%200 - 100;
        else
            pos.y = 0 + rand()%200 + 100;
    }
    void drawBoost() {
        draw();
        pos.y = rand()%(HEIGHT-200) + 100;
    }

    Image       sprites;
    Element     pos;
};

struct Pappu {
    Game    *Container;

    int     spriteW = 60, // variabel sprite size
            spriteH = 60;  
    int     curFrame = 0, // variabel animasi
            maxFrame = 7,
            frameCount = 0,
            frameDelay = 5,
            animationRow = 8,
            animationColums = 1,
            animationDirection = 1;
    int     forceAnimation = 4; // force animasi aja
    int     onPull = 0, // variabel gravity
            pullWeightUp = 2,
            onPullTimer = 0,
            onPullDelay = 30;
    int     gravity = 2;

    void animation() {
        if(!forceAnimation) { // fungsi animasinya pokoknya
            if(++frameCount >= frameDelay) {
                curFrame += animationDirection;
                if(curFrame >= maxFrame) {
                    curFrame = 0;
                } else if(curFrame <= 0) {
                    curFrame = maxFrame;
                }
                frameCount = 0;
            }
        } else curFrame = forceAnimation % maxFrame; // ini lock animasi

        if(onPull && ++onPullTimer < onPullDelay && onPullTimer%2 == 0) // pokonya gitu :v
            pos.y -= pullWeightUp;
        else if(onPullTimer >= onPullDelay) { // hahwhahwha, lupa w
            onPull = 0;
            forceAnimation = 4;
            onPullTimer = 0;
        }
    }
    bool ollicationCheck(Element A, Image B) {
        if(!(pos.x + spriteW < A.x || A.x + B.W < pos.x || pos.y + spriteW < A.y || A.y + B.H < pos.y) || pos.y + spriteH < 0 || pos.y > HEIGHT) {
            return true;
        } else return false;
    }
    void checkOllication(Branch Branchs[], Branch &BoostItem) {
        for(int i = 0; i < 3; i++) {
            if(Branchs[i].live) {
                if(ollicationCheck(Branchs[i].pos, Branchs[i].sprites)) {
                    (*Container).stateGameover(); // kalo nyentuh ya mati deng
                }
            }
        }
        if(BoostItem.live) {
            if(ollicationCheck(BoostItem.pos, BoostItem.sprites)) {
                BoostItem.removeElement();
                (*Container).score_multiplexer++;
            }
        }
    }
    void pull() {
        onPull = 1;
        forceAnimation = 0;
        onPullTimer = 0;
    }
    void updateGravity() {
        if(!onPull)
            pos.y += gravity;
        if((*Container).state == 3) { // burung falldown
            pos.y += gravity * 2; // gravity 2x
            onPullDelay = 600; // just set 600/120 second
        }
    }

    Image   sprites;
    Element pos;
};