#include <iostream>
#include <cmath>
#include <string.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "class.h"

const int FPS = 120;
const char FONT_CALIBRI[] = "font/Calibri.ttf";

int al_game_container(Game Container);

template<typename T>
T as_element_initialize(float x, float y, float W, float H, float velX, float velY, int dirX, int dirY, const char so[], const char id[] = "@", int parallax = 1) {
    T Container;

    Container.pos.initialize(x, y, velX, velY, dirX, dirY);
    Container.sprites.W = W;
    Container.sprites.H = H;
    Container.pos.id = id;
    Container.sprites.parallax = parallax;
    Container.sprites.source = al_load_bitmap(so);

    return Container;
}

template<typename T>
void as_element_drawer(T Container) {
    al_draw_bitmap(Container.sprites.source, Container.pos.x, Container.pos.y, 0);
    if(Container.sprites.parallax && Container.pos.x + Container.sprites.W < WIDTH)
        al_draw_bitmap(Container.sprites.source, Container.pos.x + Container.sprites.W, 0, 0);
}

template<typename T>
void as_element_drawer_animation(T Sprite) {
    int fx = (Sprite.curFrame % Sprite.animationColums) * Sprite.spriteW;
    int fy = (Sprite.curFrame / Sprite.animationColums) * Sprite.spriteH;
    al_draw_bitmap_region(Sprite.sprites.source, fx, fy, Sprite.spriteW, Sprite.spriteH, Sprite.pos.x, Sprite.pos.y, 0);
}

struct BackgroundContainer {
    Background  Backgrounds[10];
    int         size = 0;

    void addBackground(Background B) { Backgrounds[size++] = B; }
    void destroyAll() {
        for(int i = 0; i < size; i++) {
            Backgrounds[i].sprites.destroy();
        }
    }
    void stopAllExcept(const char *ex[], int s) {
        for(int i = 0; i < size; i++){
            int isExcepted = 0;
            for(int j = 0; j < s; j++) {
                if(Backgrounds[i].pos.id.compare(ex[j]) == 0) {
                    isExcepted = 1;
                    break;
                }
            }
            if(!isExcepted && Backgrounds[i].pos.dirX != 0)
                Backgrounds[i].pos.stopScrolling();
        }
    }
    void restartAll() {
        for(int i = 0; i < size; i++) {
            Backgrounds[i].pos.startScrolling();
        }
    }
    void resetPosition(const char *reset[], int s) {
        for(int i = 0; i < size; i++) {
            for(int j = 0; j < s; j++) {
                if(Backgrounds[i].pos.id.compare(reset[j]) == 0) {
                    Backgrounds[i].pos.resetPosition();
                    break;
                }
            }
        }
    }
    void drawBackground(const char *draw[], int s) {
        for(int i = 0; i < size; i++) {
            for(int j = 0; j < s; j++) {
                if(Backgrounds[i].pos.id.compare(draw[j]) == 0) {
                    as_element_drawer<Background>(Backgrounds[i]);
                    break;
                }
            }
        }
    }
};

void as_background_updater(Game Container, BackgroundContainer &BC) {
    int speedometers = Container.currentSpeed;
    
    if(Container.isScoreAdded())
        speedometers += pow(Container.acceleration * (Container.score / Container.acceleration_perscore), 2);

    for(int i = 0; i < BC.size; i++) {
        Background BG = BC.Backgrounds[i];
        BC.Backgrounds[i].pos.x += BG.pos.velX * BG.pos.dirX * speedometers;

        if(BG.sprites.parallax && BG.pos.x + BG.sprites.W <= 0)
            BC.Backgrounds[i].pos.x = 0;
    }
}
void as_branch_updater(Game Container, Branch BC[]) {
    int speedometers = Container.currentSpeed;

    if(Container.isScoreAdded())
        speedometers += Container.score / Container.acceleration_perscore * Container.acceleration;

    for(int i = 0; i < MAX_BRANCH; i++) {
        if(BC[i].live) {
            BC[i].pos.x += BC[i].pos.velX * BC[i].pos.dirX * speedometers;

            if(BC[i].pos.x + BC[i].sprites.W < 0 - BC[i].sprites.W)
                BC[i].removeElement();
        }
    }
}
void as_boost_updater(Game Container, Branch &BC) {
    int speedometers = Container.currentSpeed;

    if(Container.isScoreAdded())
        speedometers += Container.score / Container.acceleration_perscore * Container.acceleration;

    if(BC.live) {
        BC.pos.x += BC.pos.velX * BC.pos.dirX * speedometers;
        if(BC.pos.x + BC.sprites.W < 0 - BC.sprites.W)
            BC.removeElement();
    }
}
void as_branch_drawer(Branch BC[]) {
    int lastLive = 0;
    int lastLive_index = 0;
    float noLife = 1;
    for(int i = 0; i < MAX_BRANCH; i++) {
        if(BC[i].live) {
            as_element_drawer<Branch>(BC[i]);
            noLife = 0;
            lastLive_index = i;
            lastLive = BC[i].pos.x;
        }
    }
    if(noLife) {
        // jika ga ada yang hidup, hidupkan di index 0
        BC[0].drawBranch();
    } else {
        // ada yang live, cek lastLive
        // telah berjalan berapa lama gitu
        if(WIDTH - lastLive > 600 + rand()%100) {
            if(lastLive_index == 2)
                BC[0].drawBranch();
            else BC[lastLive_index + 1].drawBranch();
        }
    }
}
void as_boost_drawer(Branch &BC) {
    if(BC.live) {
        as_element_drawer<Branch>(BC);
    } else {
        if(rand()%rand()%500 == 0) {
            BC.drawBoost();
        }
    }
}

int main(int argc, char **argv) {

    srand(time(NULL));
    
    Game PappuPakia; // create game container

    PappuPakia.state = 1;
    PappuPakia.score = 0;
    PappuPakia.currentSpeed = 1;
    PappuPakia.acceleration = 0.02;
    PappuPakia.acceleration_perscore = 2;
    PappuPakia.score_increment = 1;
    PappuPakia.score_multiplexer = 1;

    al_game_container(PappuPakia);

    return 1;
}

void al_production_resourec_state() {
    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    al_append_path_component(path, "resource");
    al_change_directory(al_path_cstr(path, '/'));  // change the working directory
    al_destroy_path(path);
}

void al_initialize_plugin() {
    al_init_image_addon(); // image addon
    al_init_font_addon(); // font
    al_init_ttf_addon(); // font type .ttf
    al_install_audio(); // audio addon
    al_init_acodec_addon(); // audio codec, to open .ogg and etc
    al_install_keyboard(); // install keyboard input
}

int al_game_container(Game Container) {

    ALLEGRO_EVENT_QUEUE *eq;
    ALLEGRO_TIMER *timer;

    if(!al_init()) return -1; // if failed to initialize
    
    ALLEGRO_DISPLAY *display = al_create_display(WIDTH, HEIGHT); // create a display

    if(!display) return -1; // if display cannot

    al_initialize_plugin();
    al_production_resourec_state();

    BackgroundContainer background_container;
    // loading all background image and others resource
    background_container.addBackground(
        as_element_initialize<Background>(
            0, 0, 1000, 500, 0, 0, 0, 0, "bg_combined.png", "@_back"
        )
    );
    background_container.addBackground(
        as_element_initialize<Background>(
            0, 0, 1000, 500, 1.5, 0, -1, 0, "clouds.png", "@_clouds"
        )
    );
    background_container.addBackground(
        as_element_initialize<Background>(
            0, 0, 1000, 500, 1.875, 0, -1, 0, "back_trees.png", "@_backtrees"
        )
    );
    background_container.addBackground(
        as_element_initialize<Background>(
            0, 0, 1000, 500, 2.25, 0, -1, 0, "front_trees.png", "@_fronttrees"
        )
    );
    background_container.addBackground(
        as_element_initialize<Background>(
            0, 0, 1000, 500, 3, 0, -1, 0, "ground.png", "@_ground"
        )
    );
    background_container.addBackground(
        as_element_initialize<Background>(
            40, 335, 71, 119, 3, 0, -1, 0, "log.png", "@_log", 0
        )
    );
    background_container.addBackground(
        as_element_initialize<Background>(
            0, 0, 1000, 500, 0, 0, 0, 0, "splash.png", "@_splash"
        )
    );
    background_container.addBackground(
        as_element_initialize<Background>(
            0, 0, 1000, 500, 0, 0, 0, 0, "game_over.png", "@_gameover"
        )
    );
    background_container.addBackground(
        as_element_initialize<Background>(
            0, 0, 1000, 500, 0, 0, 0, 0, "key_up.png", "@_keyup"
        )
    );
    Pappu PappuConnect = as_element_initialize<Pappu>(40, 295, 60, 480, 0, 0, 0, 0, "pappu.png");
    PappuConnect.Container = &Container;

    Branch Branchs[MAX_BRANCH];
    for(int i = 0; i < MAX_BRANCH; i++) {
        Branchs[i] = as_element_initialize<Branch>(WIDTH + 10, 0, 31, 500, 3, 0, -1, 0, "branch.png", "@", 0);
    }

    // craete boost item
    Branch BoostItems;
    BoostItems = as_element_initialize<Branch>(WIDTH + 10, 0, 32, 32, 3, 0, -1, 0, "apple.png", "@_scoremux", 0);

    ALLEGRO_FONT *font18 = al_load_font(FONT_CALIBRI, 18, 0);

    // audio start
    al_reserve_samples(2);

    ALLEGRO_SAMPLE_INSTANCE *music_loop = al_create_sample_instance(al_load_sample("sound/loop.ogg"));
    ALLEGRO_SAMPLE_INSTANCE *on_jump = al_create_sample_instance(al_load_sample("sound/flap.ogg"));
    
    al_set_sample_instance_playmode(music_loop, ALLEGRO_PLAYMODE_LOOP);

    al_attach_sample_instance_to_mixer(music_loop, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(on_jump, al_get_default_mixer());
    // end audio setting

    eq = al_create_event_queue();
    timer = al_create_timer(1.0 / FPS);

    int timer_counter = 0;

    al_register_event_source(eq, al_get_keyboard_event_source());
    al_register_event_source(eq, al_get_timer_event_source(timer));

    al_play_sample_instance(music_loop);

    al_start_timer(timer);

    while(!Container.isExited()) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(eq, &ev);

        switch(ev.type) {
            case ALLEGRO_EVENT_KEY_DOWN:
                switch(ev.keyboard.keycode) {
                    case ALLEGRO_KEY_ESCAPE:
                        Container.exitGame(); // exit game
                    break;
                    case ALLEGRO_KEY_SPACE:
                        switch(Container.state) {
                            case 1: // waiting screen
                                srand(time(NULL));
                                Container.score_multiplexer = 1;
                                Container.state = 2; // make playing
                            break;
                            case 3: // game over
                                Container.state = 1; // back to waiting screen
                            break;
                        }
                    break;
                    case ALLEGRO_KEY_UP:
                        // only pull when playing
                        if(Container.isPlaying()) {
                            al_play_sample_instance(on_jump); // play sound
                            PappuConnect.pull(); // pull up
                        }
                    break;
                }
            break;
            case ALLEGRO_EVENT_TIMER:
                Container.redrawState(); // change draw state
                // if playing
                if(Container.isPlaying()) {
                    if(++timer_counter == FPS) {
                        Container.addScore(); // add score every second
                        timer_counter = 0;
                    }
                }
                const char *except[] = {"@_clouds"};
                const char *reset[] = {"@_log"};
                switch(Container.state) {
                    case 1:
                        // force bird animation
                        PappuConnect.onPullDelay = 30;
                        PappuConnect.forceAnimation = 0;

                        // stop all scrolling background except 
                
                        background_container.stopAllExcept(except, 1);
                        background_container.resetPosition(reset, 1);
                        // reset position
                        PappuConnect.pos.resetPosition();

                        BoostItems.pos.stopScrolling();
                        for(int i = 0; i < MAX_BRANCH; i++) {
                            Branchs[i].live = 0;
                            Branchs[i].pos.stopScrolling();
                        }
                    break;
                    case 2:
                        background_container.restartAll();
                        BoostItems.pos.startScrolling();
                        for(int i = 0; i < MAX_BRANCH; i++) {
                            Branchs[i].pos.startScrolling();
                        }
                        PappuConnect.updateGravity();
                    break;
                    case 3:
                        PappuConnect.forceAnimation = 0;

                        BoostItems.pos.stopScrolling();
                        background_container.stopAllExcept(except, 1);
                        for(int i = 0; i < MAX_BRANCH; i++) {
                            Branchs[i].pos.stopScrolling();
                        }
                        PappuConnect.updateGravity();
                    break;
                }

                as_background_updater(Container, background_container);
                as_branch_updater(Container, Branchs);
                as_boost_updater(Container, BoostItems);

                PappuConnect.animation();
                PappuConnect.checkOllication(Branchs, BoostItems);
            break;
        }

        if(Container.isRedraw() && al_is_event_queue_empty(eq)) {
            Container.redrawState(false);

            const char *draw1[] = {"@_back", "@_clouds", "@_backtrees", "@_fronttrees", "@_log"};
            const char *draw2[] = {"@_ground"};
            const char *drawSplash[] = {"@_splash", "@_keyup"};
            const char *drawGameOver[] = {"@_gameover"};

            background_container.drawBackground(draw1, 5);
            as_branch_drawer(Branchs);
            as_boost_drawer(BoostItems);
            background_container.drawBackground(draw2, 1);

            char mult[] = {""};

            switch(Container.state) {
                case 1:
                    background_container.drawBackground(drawSplash, 2);
                    al_draw_textf(font18, al_map_rgb(255, 255, 255), WIDTH/2, HEIGHT/2 + 60, ALLEGRO_ALIGN_CENTRE, "High Score: %d", (int)Container.highScore);
                break;
                case 2:
                    if(Container.score_multiplexer > 1) {
                        sprintf(mult, "x%d", (int)Container.score_multiplexer);
                    }
                    al_draw_textf(font18, al_map_rgb(255, 255, 255), 5, 5, 0, "Score: %d %s", (int)Container.score, mult);
                break;
                case 3:
                    background_container.drawBackground(drawGameOver, 1);
                break;
            }

            as_element_drawer_animation<Pappu>(PappuConnect);
            
            al_flip_display();
            al_clear_to_color(al_map_rgb(0, 0, 0));
        }
    }

    al_destroy_event_queue(eq);

    background_container.destroyAll();
    PappuConnect.sprites.destroy();

    for(int i = 0; i < MAX_BRANCH; i++) {
        Branchs[i].sprites.destroy();
    }

    al_destroy_font(font18);
    al_destroy_sample_instance(music_loop);
    al_destroy_timer(timer);
    al_destroy_display(display);

    return 0;
}