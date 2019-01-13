#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#define GAME_TERMINATE -1
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

// ALLEGRO Variables
ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_BITMAP *image = NULL;
ALLEGRO_BITMAP *image2 = NULL;
ALLEGRO_BITMAP *image3 = NULL;
ALLEGRO_BITMAP *background = NULL;
ALLEGRO_KEYBOARD_STATE keyState ;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_TIMER *timer2 = NULL;
ALLEGRO_TIMER *timer3 = NULL;
ALLEGRO_TIMER *timer_shooter = NULL;
ALLEGRO_TIMER *timer_enemy = NULL;
ALLEGRO_SAMPLE *song=NULL;
ALLEGRO_FONT *font = NULL;

//Custom Definition
const char *title = "Final Project 107062136";
const float FPS = 60;
const int WIDTH = 400;
const int HEIGHT = 600;
typedef struct character
{
    int x;
    int y;
    ALLEGRO_BITMAP *image_path;

}Character;
typedef struct _bullet{
    int x;
    int y;
    int vx;
    int vy;
    bool hidden;
    double time_stamp;
    struct _bullet* next;
};

typedef struct _bullet bullet;

bullet* shooter_head = NULL;
bullet* enemy_head = NULL;
Character character1;
Character character2;
Character character3;

int imageWidth = 0;
int imageHeight = 0;
int draw = 0;
int done = 0;
int window = 1;
bool judge_next_window = false;
bool ture = true; //true: appear, false: disappear
bool next = false; //true: trigger
bool dir = true; //true: left, false: right
bool* keys;
int lives = 3;
int score = 0;
int enemylife = 400;
bool judge_replay = false;
bool judge_exit = false;
int graze = 0;

void show_err_msg(int msg);
void game_init();
void game_begin();
int process_event();
int game_run();
void game_destroy();
void create_bullet(bullet**, int, int);
bullet* check_bullet(bullet*);
void free_bullet(bullet*);

int main(int argc, char *argv[]) {
    int msg = 0;

    game_init();
    game_begin();

    while (msg != GAME_TERMINATE) {
        msg = game_run();
        if (msg == GAME_TERMINATE)
            printf("Game Over\n");
    }
    free_bullet(shooter_head);
    game_destroy();
    return 0;
}

void show_err_msg(int msg) {
    fprintf(stderr, "unexpected msg: %d\n", msg);
    game_destroy();
    exit(9);
}

void game_init() {
    if (!al_init()) {
        show_err_msg(-1);
    }
    if(!al_install_audio()){
        fprintf(stderr, "failed to initialize audio!\n");
        show_err_msg(-2);
    }
    if(!al_init_acodec_addon()){
        fprintf(stderr, "failed to initialize audio codecs!\n");
        show_err_msg(-3);
    }
    if (!al_reserve_samples(1)){
        fprintf(stderr, "failed to reserve samples!\n");
        show_err_msg(-4);
    }
    // Create display
    display = al_create_display(WIDTH, HEIGHT + 50);
    event_queue = al_create_event_queue();
    if (display == NULL || event_queue == NULL) {
        show_err_msg(-5);
    }
    // Initialize Allegro settings
    al_set_window_position(display, 0, 0);
    al_set_window_title(display, title);
    al_init_primitives_addon();
    al_install_keyboard();
    al_install_audio();
    al_init_image_addon();
    al_init_acodec_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    keys = (bool*) malloc(4 * sizeof(bool));
    memset(keys, false, 4 * sizeof(bool));
    // Register event
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
}

void game_begin() {
    // Load sound
    song = al_load_sample( "Big Chungus - Piano Tutorial - CG5(amped).wav" );
    if (!song){
        printf( "Audio clip sample not loaded!\n" );
        show_err_msg(-6);
    }
    // Loop the song until the display closes
    al_play_sample(song, 1.0, 0.0,1.0,ALLEGRO_PLAYMODE_LOOP,NULL);
    al_clear_to_color(al_map_rgb(0,0,0));
    ALLEGRO_BITMAP * chungus = al_load_bitmap("bigchungus_cover.jpg");
    // Load and draw text
    font = al_load_ttf_font("pirulen.ttf",40,0);
    al_draw_bitmap(chungus,WIDTH / 2.0 - 150, HEIGHT / 2.0 - 200, 0);
    al_draw_text(font, al_map_rgb(255,255,255), WIDTH/2, 10, ALLEGRO_ALIGN_CENTRE, "SHOOT THAT");
    al_draw_text(font, al_map_rgb(255,255,255), WIDTH/2, 60, ALLEGRO_ALIGN_CENTRE, "CHUNKY BOI!");
    font = al_load_ttf_font("pirulen.ttf",12,0);
    al_draw_text(font, al_map_rgb(255,255,255), WIDTH/2, HEIGHT/2+260 , ALLEGRO_ALIGN_CENTRE, "Press 'Enter' to start");
    al_draw_rectangle(WIDTH/2-150, 550, WIDTH/2+150, 590, al_map_rgb(255, 255, 255), 0);
    al_flip_display();
    al_destroy_bitmap(chungus);
}

int process_event(){
    // Request the event
    ALLEGRO_EVENT event;
    al_wait_for_event(event_queue, &event);

    // Our setting for controlling animation
    if(event.timer.source == timer){
        if(character2.x < 0) dir = false;
        else if(character2.x > WIDTH/2 +100) dir = true;

        if(dir) character2.x -= 10;
        else character2.x += 10;
    }
    if(event.timer.source == timer2){
        ture = false;
        next = true;
    }
    if(event.timer.source == timer3){
        if(next) next = false;
        else ture = true;
    }
    if(event.timer.source == timer_shooter){
        create_bullet(&shooter_head, character1.x +10.5, character1.y);
        shooter_head = check_bullet(shooter_head);
        if(shooter_head != NULL){
            bullet* temp = shooter_head;
            double now = al_get_time();
            while(temp->next != NULL && temp != NULL){
                temp->y -= 50*(now - temp->time_stamp);
                temp =  temp->next;
            }
            temp->y -= 50*(now - temp->time_stamp);
        }
    }
    if(event.timer.source == timer_enemy){
        create_bullet(&enemy_head, character2.x + 47, 200);
        enemy_head = check_bullet(enemy_head);
        if(enemy_head != NULL){
            bullet* tempein = enemy_head;
            //double now = al_get_time();
            while(tempein->next != NULL && tempein != NULL){
                tempein->x = tempein->x + tempein->vx;
                tempein->y = tempein->y + tempein->vy;
                tempein =  tempein->next;
            }
            tempein->x = tempein->x + tempein->vx;
            tempein->y = tempein->y + tempein->vy;
        }
    }
    // Keyboard
    if(event.type == ALLEGRO_EVENT_KEY_DOWN) {
        switch(event.keyboard.keycode) {
            case ALLEGRO_KEY_W:
                keys[UP] = true;
                break;
            case ALLEGRO_KEY_S:
                keys[DOWN] = true;
                break;
            case ALLEGRO_KEY_D:
                keys[RIGHT] = true;
                break;
            case ALLEGRO_KEY_A:
                keys[LEFT] = true;
                break;
        }
    }
    else if(event.type == ALLEGRO_EVENT_KEY_UP)
    {
        switch(event.keyboard.keycode)
        {
            // Control
            case ALLEGRO_KEY_W:
                keys[UP] = false;
                break;
            case ALLEGRO_KEY_S:
                keys[DOWN] = false;
                break;
            case ALLEGRO_KEY_A:
                keys[LEFT] = false;
                break;
            case ALLEGRO_KEY_D:
                keys[RIGHT] = false;
                break;

            // For Start Menu
            case ALLEGRO_KEY_ENTER:
                judge_next_window = true;
                break;
            //temp
            case ALLEGRO_KEY_N:
                window = 3;
                break;

        }
        //for final
        if(window == 3){
            switch(event.keyboard.keycode){
                case ALLEGRO_KEY_F:
                    judge_exit = true;
                    break;
                case ALLEGRO_KEY_R:
                    judge_replay = true;
                    break;
            }
        }
    }

    // Shutdown our program
    else if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        return GAME_TERMINATE;

    if(character1.y >= 200) character1.y -= keys[UP] * 10;
    if(character1.y <= HEIGHT / 2 + 260) character1.y += keys[DOWN] * 10;
    if(character1.x >= 10) character1.x -= keys[LEFT] * 10;
    if(character1.x <= WIDTH / 2 + 160) character1.x += keys[RIGHT] * 10;

    //enemy injure
    bullet* tempe = shooter_head;
    if(tempe != NULL){
        while(tempe->next != NULL){
            if(!tempe->hidden){
                if(tempe->y <= 200 && tempe->x - character2.x <= 94.4 && tempe->x - character2.x >= 0){
                    enemylife = enemylife - 0.0001;
                    score += 16;
                    tempe->hidden = true;
                }
            }
            tempe = tempe->next;
        }
        if(!tempe->hidden){
            if(tempe->y <= 200 && tempe->x - character2.x <= 94.4 && tempe->x - character2.x >= 0){
                enemylife = enemylife - 0.0001;
                score += 16;
                tempe->hidden = true;
            }
        }
    }
    //shooter injure and graze
    tempe = enemy_head;
    if(tempe != NULL){
        while(tempe->next != NULL){
            if(!tempe->hidden){
                double dis = sqrt((tempe->x - character1.x - 15.5) * (tempe->x - character1.x - 15.5) + (tempe->y - character1.y - 15.5) * (tempe->y - character1.y - 15.5));
                if(dis <= 10){
                    lives --;
                    bullet* in = enemy_head;
                    while(in->next != NULL){
                        in->hidden = true;
                        in = in->next;
                    }
                    in->hidden = true;
                }
                else if(dis <= 20.73){
                    score += 20;
                    graze++;
                }
            }
            tempe = tempe->next;
        }
        if(!tempe->hidden){
            double dis = sqrt((tempe->x - character1.x - 15.5) * (tempe->x - character1.x - 15.5) + (tempe->y - character1.y-15.5) * (tempe->y - character1.y-15.5));
            if(dis <= 10){
                lives --;
                bullet* in = enemy_head;
                while(in->next != NULL){
                    in->hidden = true;
                    in = in->next;
                }
                in->hidden = true;
            }
            else if(dis <= 20.73){
                score += 20;
                graze++;
            }
        }
    }
    //bullets hitting
    /*tempe = enemy_head;
    bullet* temps = shooter_head;
    if(tempe != NULL && temps != NULL){
        while(tempe->next != NULL){
        temps = shooter_head;
        while(temps->next!= NULL){
            if(abs(tempe->x - temps->x - 5) <= 10 && abs(tempe->y - temps->y - 5) <= 10){
                tempe->hidden = true;
                temps->hidden = true;
            }
            temps = temps->next;
        }
        if(abs(tempe->x - temps->x - 5) <= 10 && abs(tempe->y - temps->y - 5) <= 10){
            tempe->hidden = true;
            temps->hidden = true;
        }
        tempe = tempe->next;
    }
    if(abs(tempe->x - temps->x - 5) <= 10 && abs(tempe->y - temps->y - 5) <= 10){
        tempe->hidden = true;
        temps->hidden = true;
    }

    }*/

    return 0;
}

int game_run() {
    int error = 0;
    // First window(Menu)
    if(window == 1){
        if (!al_is_event_queue_empty(event_queue)) {
            error = process_event();
            if(judge_next_window) {
                window = 2;
                judge_next_window = FALSE;
                // Setting Character
                character1.x = WIDTH / 2;
                character1.y = HEIGHT / 2 + 150;
                character2.x = WIDTH - 94;
                character2.y = HEIGHT / 2 - 280;
                character1.image_path = al_load_bitmap("shooter.jpg");
                character2.image_path = al_load_bitmap("bigchunguswhiteleft.jpg");
                character3.image_path = al_load_bitmap("bigchunguswhiteright.jpg");
                al_clear_to_color(al_map_rgb(0,0,0));
                al_stop_samples();
                song = al_load_sample("Big Chungus (song)(ex).wav");
                //Initialize Timer
                timer  = al_create_timer(1.0/15.0);
                timer2  = al_create_timer(1.0);
                timer3  = al_create_timer(1.0/10.0);
                timer_shooter = al_create_timer(1.0/15.0);
                timer_enemy = al_create_timer(1.0/15.0);
                al_register_event_source(event_queue, al_get_timer_event_source(timer)) ;
                al_register_event_source(event_queue, al_get_timer_event_source(timer2)) ;
                al_register_event_source(event_queue, al_get_timer_event_source(timer3)) ;
                al_register_event_source(event_queue, al_get_timer_event_source(timer_shooter)) ;
                al_register_event_source(event_queue, al_get_timer_event_source(timer_enemy)) ;
                al_start_timer(timer);
                al_start_timer(timer2);
                al_start_timer(timer3);
                al_start_timer(timer_shooter);
                al_start_timer(timer_enemy);
                al_play_sample(song, 1.0, 0.0,1.0,ALLEGRO_PLAYMODE_LOOP,NULL);
            }
        }
    }
    // Second window(Main Game)
    else if(window == 2){
        // Change Image for animation
        al_clear_to_color(al_map_rgb(0,0,0));
        if(ture){
            al_draw_bitmap(character1.image_path, character1.x, character1.y, 0);
            al_draw_filled_circle(character1.x + 15.5, character1.y + 15.5, 5,al_map_rgb(255,0,0));
        }
        if(dir) al_draw_bitmap(character2.image_path, character2.x, character2.y, 0);
        else al_draw_bitmap(character3.image_path, character2.x, character2.y, 0);
        //draw bullets
        bullet* temp = shooter_head;
        if(temp != NULL){
            while(temp->next != NULL && temp != NULL){
                if(temp->hidden == false) al_draw_filled_rectangle(temp->x,temp->y,temp->x + 10, temp->y +10,al_map_rgb(255,255,255));
                temp = temp->next;
            }
        }
        temp = enemy_head;
        if(temp != NULL){
            while(temp->next != NULL && temp != NULL){
                if(temp->hidden == false) al_draw_filled_circle(temp->x, temp->y, 5, al_map_rgb(200,200,200));
                temp = temp->next;
            }
        }
        //draw life and score and enemy life
        ALLEGRO_FONT* font_add = al_load_ttf_font("pirulen.ttf",25,0);
        al_draw_textf(font_add, al_map_rgb(255,255,255), 0, HEIGHT, ALLEGRO_ALIGN_LEFT, "LIVES: %d",lives);
        al_draw_textf(font_add, al_map_rgb(255,255,255), WIDTH/2 - 25, HEIGHT, ALLEGRO_ALIGN_LEFT, "SCORE: %d",score);
        al_draw_filled_rectangle(0,HEIGHT +35, enemylife, HEIGHT +45,al_map_rgb(255,255,255));
        al_flip_display();
        al_clear_to_color(al_map_rgb(0,0,0));
        al_destroy_font(font_add);
        if(lives <= 0 || enemylife <= 0) window = 3;
        // Listening for new event
        if (!al_is_event_queue_empty(event_queue)) {
            error = process_event();
        }
    }
    else if(window == 3){
        bullet* temp1 = shooter_head;
        while(temp1->next != NULL){
            temp1->hidden = true;
            temp1 = temp1->next;
        }
        temp1->hidden = true;
        temp1 = enemy_head;
        while(temp1->next != NULL){
            temp1->hidden = true;
            temp1 = temp1->next;
        }
        temp1->hidden = true;
        ALLEGRO_FONT * font_final = al_load_ttf_font("pirulen.ttf",40,0);
        if(lives == 0) al_draw_text(font_final, al_map_rgb(255,255,255), WIDTH/2, 60, ALLEGRO_ALIGN_CENTRE, "YOU LOSE!");
        else if(enemylife == 0) al_draw_text(font_final, al_map_rgb(255,255,255), WIDTH/2, 60, ALLEGRO_ALIGN_CENTRE, "YOU WIN!");
        al_draw_textf(font_final, al_map_rgb(255,255,255), WIDTH/2, 110, ALLEGRO_ALIGN_CENTRE, "SCORE: %d", score);
        al_draw_textf(font_final, al_map_rgb(255,255,255), WIDTH/2, 160, ALLEGRO_ALIGN_CENTRE, "GRAZE: %d", graze);
        al_destroy_font(font_final);
        ALLEGRO_FONT *font_final2 = al_load_ttf_font("pirulen.ttf",12,0);
        al_draw_text(font_final2, al_map_rgb(255,255,255), WIDTH/2, HEIGHT/2+260 , ALLEGRO_ALIGN_CENTRE, "Press 'F' to pay respect and exit");
        al_draw_rectangle(WIDTH/2-160, 550, WIDTH/2+160, 590, al_map_rgb(255, 255, 255), 0);
        al_draw_text(font_final2, al_map_rgb(255,255,255), WIDTH/2, HEIGHT/2+210 , ALLEGRO_ALIGN_CENTRE, "Press 'R' to restart");
        al_draw_rectangle(WIDTH/2-150, 500, WIDTH/2+150, 540, al_map_rgb(255, 255, 255), 0);
        al_flip_display();
        al_destroy_font(font_final2);
        if(judge_exit) return GAME_TERMINATE;
        if(judge_replay) {
            window = 2;
            judge_replay = false;
            lives = 3;
            enemylife = 400;
            score = 0;
            graze = 0;
        }
        //listen event
        if (!al_is_event_queue_empty(event_queue)) {
            error = process_event();
        }
    }
    return error;
}

void game_destroy() {
    // Make sure you destroy all things
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_timer(timer2);
    al_destroy_bitmap(image);
    al_destroy_sample(song);
}

void create_bullet(bullet ** head, int x, int y)
{
    srand(time(NULL));
    if(*head == NULL){
        *head = (bullet*)malloc(sizeof(bullet));
        (*head)->x = x;
        (*head)->y = y;
        (*head)->hidden = false;
        (*head)->time_stamp = al_get_time();
        (*head)->vy = rand()%10 + 1;
        if(rand()%2) (*head)->vx = rand()%10 + 1;
        else (*head)->vx = -(rand()%10 + 1);
        (*head)->next = NULL;
    }
    else{
        bullet* temp = *head;
        while(temp->next != NULL) temp = temp -> next;
        bullet* newnode = (bullet*)malloc(sizeof(bullet));
        temp->next = newnode;
        newnode->x = x;
        newnode->y = y;
        newnode->hidden = false;
        newnode->time_stamp = al_get_time();
        newnode->vy = rand()%10 + 1;
        if(rand()%2) newnode->vx = rand()%10 + 1;
        else newnode->vx = -(rand()%10 + 1);
        newnode->next = NULL;
    }
}

bullet* check_bullet(bullet* head)
{
    bullet* temp;
    if(head == NULL) return NULL;

    else if(head->y <= 0 || head->y >= HEIGHT ){
        if(head->next != NULL){
            temp = head->next;
            free(head);
            return check_bullet(temp);
        }
        else{
            free(head);
            return NULL;
        }
    }
    else{
        temp = head->next;
        head->next = check_bullet(temp);
        return head;
    }
}
void free_bullet(bullet* head)
{
    if(head != NULL){
        if(head->next != NULL) free_bullet(head->next);
        free(head);
    }
}
