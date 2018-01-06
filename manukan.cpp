#include "src/include.h"

int al_game_container(Game Container);

int as_get_speed(Game Container) {
    int speedometers = Container.currentSpeed;
    if(Container.isScoreAdded())
        speedometers += pow(Container.acceleration * (Container.score / Container.acceleration_perscore), 2);
    return speedometers;
}

void as_background_updater(Game Container, BackgroundContainer &BC) {
    for(int i = 0; i < BC.size; i++) {
        Background BG = BC.Backgrounds[i];
        BC.Backgrounds[i].pos.x += BG.pos.velX * BG.pos.dirX * as_get_speed(Container);
        if(BG.sprites.parallax && BG.pos.x + BG.sprites.W <= 0) BC.Backgrounds[i].pos.x = 0;
    }
}

void as_branch_updater(Game Container, Branch BC[]) {
    for(int i = 0; i < MAX_BRANCH; i++) {
        if(BC[i].live) {
            BC[i].pos.x += BC[i].pos.velX * BC[i].pos.dirX * as_get_speed(Container);
            if(BC[i].pos.x + BC[i].sprites.W < 0 - BC[i].sprites.W) BC[i].removeElement();
        }
    }
}

void as_boost_updater(Game Container, Branch &BC) {
    if(BC.live) {
        BC.pos.x += BC.pos.velX * BC.pos.dirX * as_get_speed(Container);
        if(BC.pos.x + BC.sprites.W < 0 - BC.sprites.W) BC.removeElement();
    }
}

void as_branch_drawer(Branch BC[]) {
    int     lastLive = 0, // cek kalo ada branch aktif
            lastLive_index = 0; // index berapa
    float   noLife = 1; // jika tidak ada yang live

    for(int i = 0; i < MAX_BRANCH; i++) {
        if(BC[i].live) {
            as_element_drawer<Branch>(BC[i]); // draw branch
            noLife = 0; lastLive_index = i; lastLive = BC[i].pos.x;
        }
    }
    if(noLife) BC[0].drawBranch(); // jika ga ada yang hidup, hidupkan di index 0
    else { // ada yang live, cek lastLive, telah berjalan berapa lama gitu
        if(WIDTH - lastLive > 600 + rand()%100) { // fungsi probabilitasnya
            if(lastLive_index == 2) BC[0].drawBranch(); // jika lasst index, maka draw pertama arraynya
            else BC[lastLive_index + 1].drawBranch(); // atau draw berikutnya
        }
    }
}

void as_boost_drawer(Branch &BC) {
    if(BC.live) as_element_drawer<Branch>(BC); // draw booster
    else
        if(rand()%rand()%1000 == 0) BC.drawBoost(); // dengan probability yang besaaar
}

int main(int argc, char **argv) {

    srand(time(NULL)); // seed dari rand()

    Game PappuPakia; // create game container
    PappuPakia.state = 4; // initialize state
    PappuPakia.score = 0; // first score
    PappuPakia.currentSpeed = 1; // first speed
    PappuPakia.acceleration = 0.02; // acceleration at first
    PappuPakia.acceleration_perscore = 6; // increase acceleration per score
    PappuPakia.score_increment = 1; // score increment
    PappuPakia.score_multiplexer = 1; // score multiplexer
    PappuPakia.soundActive = 1; // active sound or not

    al_game_container(PappuPakia); // pass game to container

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

    int timer_counter = 0, // timer counter helper for allegro
        game_running_time = 0; // running time (in second)

    const char *except[] = {"@_clouds"};
    const char *reset[] = {"@_log"};
    const char *draw1[] = {"@_back", "@_clouds", "@_backtrees", "@_fronttrees", "@_log"};
    const char *draw2[] = {"@_ground"};
    const char *drawSplash[] = {"@_splash", "@_keyup"};
    const char *drawGameOver[] = {"@_gameover"};
    const char *drawLoading[] = {"@_flash"};
    char mult[] = {""};

    ALLEGRO_EVENT_QUEUE *eq;
    ALLEGRO_TIMER *timer;

    if(!al_init()) return -1; // if failed to initialize

    ALLEGRO_DISPLAY *display = al_create_display(WIDTH, HEIGHT); // create a display

    if(!display) return -1; // if display cannot

    al_set_window_title(display, "Manukan Bird - Game of Student");
    al_initialize_plugin();
    al_production_resourec_state();

    BackgroundContainer background_container; // loading all background image and others resource
    background_container.addBackground(as_element_initialize<Background>(0, 0, 1000, 500, 0, 0, 0, 0, "bg_combined.png", "@_back"));
    background_container.addBackground(as_element_initialize<Background>(0, 0, 1000, 500, 1.5, 0, -1, 0, "clouds.png", "@_clouds"));
    background_container.addBackground(as_element_initialize<Background>(0, 0, 1000, 500, 1.875, 0, -1, 0, "back_trees.png", "@_backtrees"));
    background_container.addBackground(as_element_initialize<Background>(0, 0, 1000, 500, 2.25, 0, -1, 0, "front_trees.png", "@_fronttrees"));
    background_container.addBackground(as_element_initialize<Background>(0, 0, 1000, 500, 3, 0, -1, 0, "ground.png", "@_ground"));
    background_container.addBackground(as_element_initialize<Background>(40, 335, 71, 119, 3, 0, -1, 0, "log.png", "@_log", 0));
    background_container.addBackground(as_element_initialize<Background>(0, 0, 1000, 500, 0, 0, 0, 0, "splash.png", "@_splash"));
    background_container.addBackground(as_element_initialize<Background>(0, 0, 1000, 500, 0, 0, 0, 0, "game_over.png", "@_gameover"));
    background_container.addBackground(as_element_initialize<Background>(0, 0, 1000, 500, 0, 0, 0, 0, "key_up.png", "@_keyup"));
    background_container.addBackground(as_element_initialize<Background>(0, 0, 1000, 500, 0, 0, 0, 0, "plash.png", "@_flash", 0));

    Pappu PappuConnect = as_element_initialize<Pappu>(40, 295, 60, 480, 0, 0, 0, 0, "pappu.png");
    PappuConnect.Container = &Container;

    Branch Branchs[MAX_BRANCH];
    for(int i = 0; i < MAX_BRANCH;) // draw branch and create load images
        Branchs[i++] = as_element_initialize<Branch>(WIDTH + 10, 0, 31, 500, 3, 0, -1, 0, "branch.png", "@", 0);

    Branch BoostItems; // craete boost item and load
    BoostItems = as_element_initialize<Branch>(WIDTH + 10, 0, 32, 32, 3, 0, -1, 0, "coin.png", "@_scoremux", 0);
    
    al_reserve_samples(2); // audio start

    ALLEGRO_SAMPLE_INSTANCE *music_loop = al_create_sample_instance(al_load_sample("sound/loop.ogg"));
    ALLEGRO_SAMPLE_INSTANCE *on_jump = al_create_sample_instance(al_load_sample("sound/flap.ogg"));

    al_set_sample_instance_playmode(music_loop, ALLEGRO_PLAYMODE_LOOP);

    al_attach_sample_instance_to_mixer(music_loop, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(on_jump, al_get_default_mixer()); // end audio setting
    
    ALLEGRO_FONT *font18 = al_load_font(FONT_CALIBRI, 18, 0);

    eq = al_create_event_queue();
    timer = al_create_timer(1.0 / FPS);

    al_register_event_source(eq, al_get_keyboard_event_source());
    al_register_event_source(eq, al_get_display_event_source(display));
    al_register_event_source(eq, al_get_timer_event_source(timer));

    if(Container.soundActive) al_play_sample_instance(music_loop);

    al_start_timer(timer);

    while(!Container.isExited()) {
        ALLEGRO_EVENT EVENT;
        al_wait_for_event(eq, &EVENT);

        switch(EVENT.type) {
            case ALLEGRO_EVENT_KEY_DOWN:
                switch(EVENT.keyboard.keycode) {
                    case ALLEGRO_KEY_ESCAPE: Container.exitGame(); // exit game
                        break;
                    case ALLEGRO_KEY_SPACE:
                        switch(Container.state) {
                            case 1: // waiting screen
                                srand(time(NULL)); // re-seed seed for rand()
                                Container.score_multiplexer = 1; // reset score
                                Container.state = 2; // make playing
                                break;
                            case 3: Container.state = 1; // back to waiting screen
                                break;
                        }
                        break;
                    case ALLEGRO_KEY_UP:
                        if(Container.isPlaying()) { // only pull when playing
                            if(Container.soundActive) al_play_sample_instance(on_jump); // play sound
                            PappuConnect.pull(); // pull up
                        }
                        break;
                }
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE: Container.exitGame(); //close window when click X icon
                break;
            case ALLEGRO_EVENT_TIMER:
                Container.redrawState(); // change draw state
                if(Container.isPlaying()) { // if playing, update score
                    if(++timer_counter%FPS == 0) {
                        Container.addScore(); // add score every second
                        timer_counter = 0;
                    }
                }
                
                game_running_time++; // add timer every timer

                switch(Container.state) {
                    case 1:
                        PappuConnect.onPullDelay = 30;
                        PappuConnect.forceAnimation = 0; // force bird animation

                        background_container.stopAllExcept(except, 1); // stop all scrolling background except 
                        background_container.resetPosition(reset, 1);
                        
                        PappuConnect.pos.resetPosition(); // reset position

                        BoostItems.live = 0; // set live to 0 all boost and stop scrolling
                        BoostItems.pos.stopScrolling();
                        for(int i = 0; i < MAX_BRANCH; i++) {
                            Branchs[i].live = 0; // also for branch and stop scrolling
                            Branchs[i].pos.stopScrolling();
                        }
                        break;
                    case 2:
                        background_container.restartAll(); // restart all background container speed
                        BoostItems.pos.startScrolling(); // also for boost
                        for(int i = 0; i < MAX_BRANCH; i++) // for branch also
                            Branchs[i].pos.startScrolling();

                        PappuConnect.updateGravity(); // update gravity
                        break;
                    case 3:
                        PappuConnect.forceAnimation = 0; // force animation to not
                        BoostItems.pos.stopScrolling();
                        background_container.stopAllExcept(except, 1);
                        for(int i = 0; i < MAX_BRANCH; i++)
                            Branchs[i].pos.stopScrolling();

                        PappuConnect.updateGravity();
                        break;
                    case 4:
                        PappuConnect.forceAnimation = 0;
                        PappuConnect.waitingPosition();
                        if(game_running_time%(FPS*5) == 0) Container.state = 1;
                        break;
                }

                as_background_updater(Container, background_container);
                as_branch_updater(Container, Branchs);
                as_boost_updater(Container, BoostItems);

                PappuConnect.animation();
                if(Container.state == 2) PappuConnect.checkOllication(Branchs, BoostItems);
                break;
        }

        if(Container.isRedraw() && al_is_event_queue_empty(eq)) {
            Container.redrawState(false);

            if(Container.state != 4) {
                background_container.drawBackground(draw1, 5);
                as_branch_drawer(Branchs);
                as_boost_drawer(BoostItems);
                background_container.drawBackground(draw2, 1);
            }

            switch(Container.state) {
                case 1:
                    background_container.drawBackground(drawSplash, 2);
                    al_draw_textf(font18, al_map_rgb(255, 255, 255), WIDTH/2, HEIGHT/2 + 60, ALLEGRO_ALIGN_CENTRE, "High Score: %d", (int)Container.highScore);
                    break;
                case 2:
                    if(Container.score_multiplexer > 1) sprintf(mult, "x%d", (int)Container.score_multiplexer);
                    
                    al_draw_textf(font18, al_map_rgb(255, 255, 255), 5, 5, 0, "Score: %d %s", (int)Container.score, Container.score_multiplexer > 1 ? mult : "");
                    break;
                case 3:
                    background_container.drawBackground(drawGameOver, 1);
                    al_draw_textf(font18, al_map_rgb(255, 255, 255), WIDTH/2, HEIGHT/2 + 60, ALLEGRO_ALIGN_CENTRE, "Your Score: %d", (int)Container.last_score);
                    break;
                case 4: background_container.drawBackground(drawLoading, 1);
                    break;
            }

            as_element_drawer_animation<Pappu>(PappuConnect);

            al_flip_display();
            al_clear_to_color(al_map_rgb(0, 0, 0));
        }
    }

    // destroy all allegro sourec from memory
    al_destroy_event_queue(eq);
    al_destroy_font(font18);
    al_destroy_sample_instance(music_loop);
    al_destroy_timer(timer);
    al_destroy_display(display);

    background_container.destroyAll();
    PappuConnect.sprites.destroy();
    for(int i = 0; i < MAX_BRANCH; i++) Branchs[i].sprites.destroy();

    return 0;
}