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