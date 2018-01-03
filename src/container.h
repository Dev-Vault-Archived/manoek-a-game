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