/*
 * project2.c
 */


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include "../mylib/initShader.h"
#include "../mylib/linear_alg.h"

#define PI (3.141592653589793)
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

typedef struct
{
    GLfloat x;
    GLfloat y;
} vec2;

typedef enum
 {
    FLYING_TO_START = 0, 
    FLYING_AROUND, 
    FLYING_DOWN, 
    WALK_FORWARD, 
    TURN_LEFT, 
    TURN_RIGHT,
} state;

state currentState = FLYING_TO_START;

void add_cube(vec4, vec4, vec4, vec4, vec4, vec4, vec4, vec4, int, mat4);
void add_texture(vec2, vec2, vec2, vec2, int);

int num_vertices = 10944;

vec4 vertices[10944];
vec2 tex_coords[10944];

int isAnimating = 0;
            

mat4 model_view = {{1, 0, 0, 0},
                    {0, 1, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1}};
mat4 projection = {{1, 0, 0, 0},
                    {0, 1, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1}};
                    
GLuint model_view_location;
GLuint projection_location;
GLuint texture_location;

typedef struct {
    int north;
    int east;
    int south;
    int west;
} cell;



cell cells[8][8];
vec2 cell_visit[81];
int cursor = 0;

void maze(int left, int right, int bottom, int top) {
    srand(time(NULL));
    if((right - left) <= 1 || (top - bottom) <= 1) return;
    int row_low = bottom + 1;
    int row_high = top - 1;
    int col_low = left + 1;
    int col_high = right - 1;
    int r = (rand() % (row_high - row_low + 1)) + row_low;
    int c = (rand() % (col_high - col_low + 1)) + col_low;
    //generate walls
    for(int i = bottom; i < top; i++) {
        cells[i][c-1].east = 1;
        cells[i][c].west = 1;
    }
    for(int i = left; i < right; i++) {
        cells[r-1][i].south = 1;
        cells[r][i].north = 1;
    }
    //remove walls
    bool direction[4] = {false, false, false, false};
    for(int i = 0; i < 3; i++) {
        int temp;
        do{
            temp = rand() % 4;
        }while(direction[temp]);
        direction[temp] = true;
    }
    if(direction[0]) {  // remove one of the walls in the north
        int temp = rand() % (r - bottom) + bottom;
        cells[temp][c-1].east = 0;
        cells[temp][c].west = 0;
    }
    if(direction[1]) {  // remove one of the walls from south
        int temp = rand() % (top - r) + r;
        cells[temp][c-1].east = 0;
        cells[temp][c].west = 0;
    }
    if(direction[2]) {  // remove one of the walls from west
        int temp = rand() % (c - left) + left;
        cells[r-1][temp].south = 0;
        cells[r][temp].north = 0;
    }
    if(direction[3]) {  // remove one of the walls from east
        int temp = rand() % (right - c) + c;
        cells[r-1][temp].south = 0;
        cells[r][temp].north = 0;
    }

    //recursively generate maze for four subsets
    maze(left, c, bottom, r);   //top-left
    maze(c, right, bottom, r);  //top-right
    maze(left, c, r, top);      //bottom-left
    maze(c, right, r, top);     //bottom-right

}

void init_maze() {
    for(int i = 0; i < 8; i++) {
        cells[0][i].north = 1;
        cells[i][0].west = 1;
        cells[7][i].south = 1;
        cells[i][7].east = 1;
    }
    cells[0][0].west = 0;
    cells[7][7].east = 0;
    maze(0, 8, 0, 8);
}
void print_maze(){
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(cells[i][j].north) printf("*--");
            else printf("*  ");
        }
        printf("*\n");
        for(int j = 0; j < 8; j++) {
            if(cells[i][j].west) printf("|  ");
            else printf("   ");
        }
        if(cells[i][7].east) printf("|\n");
        else printf(" \n");
    }
    printf("*--*--*--*--*--*--*--*--*\n");
}

void check_cells(){
    printf("checking cells...\n");
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            if(cells[i][j].north){
                if(i - 1 >= 0){
                    if(!cells[i-1][j].south)
                        printf("error\n");
                }
            }
            if(cells[i][j].south){
                if(i+1 <= 7){
                    if(!cells[i+1][j].north)
                        printf("error\n");
                }
            }
            if(cells[i][j].west){
                if(j-1 >=0){
                    if(!cells[i][j-1].east)
                        printf("error\n");
                }
            }
            if(cells[i][j].east){
                if(j+1 <= 7){
                    if(!cells[i][j+1].west)
                        printf("error\n");
                }
            }
        }
    }
}

//first 9*9*36 = 2916 vertices are poles
void generate_maze_pole() {
    vec4 blu = {-1, -0.5, -1, 1};
    vec4 bld = {-1, -0.5, 1, 1};
    vec4 bru = {1, -0.5, -1, 1};
    vec4 brd = {1, -0.5, 1, 1};
        
    vec4 tlu = {-1, 0.5, -1, 1};
    vec4 tld = {-1, 0.5, 1, 1};
    vec4 tru = {1, 0.5, -1, 1};
    vec4 trd = {1, 0.5, 1, 1};

    vec2 lu = {0.5, 0};
    vec2 ld = {0.5, 0.5};
    vec2 ru = {1, 0};
    vec2 rd = {1, 0.5};

    mat4 scale = scaling_m(0.1, 1, 0.1);

    blu = m_v_mult(scale, blu);
    bld = m_v_mult(scale, bld);
    bru = m_v_mult(scale, bru);
    brd = m_v_mult(scale, brd);
    tlu = m_v_mult(scale, tlu);
    tld = m_v_mult(scale, tld);
    tru = m_v_mult(scale, tru);
    trd = m_v_mult(scale, trd);
    int start = 0;
    for(int i = 0; i < 81; i++) {
        int x_coord = (i % 9) - 4;
        int z_coord = (i / 9) - 4;
        mat4 trans_m = translation_m(x_coord, 0, z_coord);
        add_cube(blu, bld, bru, brd, tlu, tld, tru, trd, start, trans_m);
        //add texture
        add_texture(lu, ld, ru, rd, start);
        start += 36;
    }
}


// 79 walls ==> 79 * 36 = 2844 vertices
void generate_maze_walls() {
    vec4 blu = {-1, -0.5, -1, 1};
    vec4 bld = {-1, -0.5, 1, 1};
    vec4 bru = {1, -0.5, -1, 1};
    vec4 brd = {1, -0.5, 1, 1};
        
    vec4 tlu = {-1, 0.5, -1, 1};
    vec4 tld = {-1, 0.5, 1, 1};
    vec4 tru = {1, 0.5, -1, 1};
    vec4 trd = {1, 0.5, 1, 1};

    vec2 lu = {0, 0};
    vec2 ld = {0, 0.5};
    vec2 ru = {0.5, 0};
    vec2 rd = {0.5, 0.5};

    mat4 scale_h = scaling_m(0.4, 1, 0.1);
    mat4 scale_v = scaling_m(0.1, 1, 0.4);

    vec4 blu_h = m_v_mult(scale_h, blu);
    vec4 bld_h = m_v_mult(scale_h, bld);
    vec4 bru_h = m_v_mult(scale_h, bru);
    vec4 brd_h = m_v_mult(scale_h, brd);
    vec4 tlu_h = m_v_mult(scale_h, tlu);
    vec4 tld_h = m_v_mult(scale_h, tld);
    vec4 tru_h = m_v_mult(scale_h, tru);
    vec4 trd_h = m_v_mult(scale_h, trd);

    vec4 blu_v = m_v_mult(scale_v, blu);
    vec4 bld_v = m_v_mult(scale_v, bld);
    vec4 bru_v = m_v_mult(scale_v, bru);
    vec4 brd_v = m_v_mult(scale_v, brd);
    vec4 tlu_v = m_v_mult(scale_v, tlu);
    vec4 tld_v = m_v_mult(scale_v, tld);
    vec4 tru_v = m_v_mult(scale_v, tru);
    vec4 trd_v = m_v_mult(scale_v, trd);
    int start = 2916;           // after the poles
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            mat4 trans_m;
            if(cells[i][j].north) {
                double x_coord = -3.5 + j;
                double z_coord = -4 + i;
                trans_m = translation_m(x_coord, 0, z_coord);
                add_cube(blu_h, bld_h, bru_h, brd_h, tlu_h, tld_h, tru_h, trd_h, start, trans_m);
                add_texture(lu, ld, ru, rd, start);
                start += 36;
            }
            if(cells[i][j].west) {
                double x_coord = -4 + j;
                double z_coord = -3.5 + i;
                trans_m = translation_m(x_coord, 0, z_coord);
                add_cube(blu_v, bld_v, bru_v, brd_v, tlu_v, tld_v, tru_v, trd_v, start, trans_m);
                add_texture(lu, ld, ru, rd, start);
                start += 36;
            }
        }
    }
    for(int i = 0; i < 8; i++) {
        if(cells[7][i].south) {
            double x_coord = -3.5 + i;
            mat4 trans_m = translation_m(x_coord, 0, 4);
            add_cube(blu_h, bld_h, bru_h, brd_h, tlu_h, tld_h, tru_h, trd_h, start, trans_m);
            add_texture(lu, ld, ru, rd, start);
            start += 36;
        }
        if(cells[i][7].east) {
            double z_coord = -3.5 + i;
            mat4 trans_m = translation_m(4, 0, z_coord);
            add_cube(blu_v, bld_v, bru_v, brd_v, tlu_v, tld_v, tru_v, trd_v, start, trans_m);
            add_texture(lu, ld, ru, rd, start);
            start += 36;
        }
    }
}

void generate_maze_floors() {
    vec4 blu = {-0.5, -0.5, -0.5, 1};
    vec4 bld = {-0.5, -0.5, 0.5, 1};
    vec4 bru = {0.5, -0.5, -0.5, 1};
    vec4 brd = {0.5, -0.5, 0.5, 1};
        
    vec4 tlu = {-0.5, 0.5, -0.5, 1};
    vec4 tld = {-0.5, 0.5, 0.5, 1};
    vec4 tru = {0.5, 0.5, -0.5, 1};
    vec4 trd = {0.5, 0.5, 0.5, 1};

    vec2 lu = {0, 0.5};
    vec2 ld = {0, 1};
    vec2 ru = {0.5, 0.5};
    vec2 rd = {0.5, 1};

    mat4 scale_m = scaling_m(1, 0.2, 1);

    blu = m_v_mult(scale_m, blu);
    bld = m_v_mult(scale_m, bld);
    bru = m_v_mult(scale_m, bru);
    brd = m_v_mult(scale_m, brd);
    tlu = m_v_mult(scale_m, tlu);
    tld = m_v_mult(scale_m, tld);
    tru = m_v_mult(scale_m, tru);
    trd = m_v_mult(scale_m, trd);

    int start = 5760;   // after the poles and the walls
    /*
    mat4 trans_m = translation_m(0, -0.6, 0);
    add_cube(blu, bld, bru, brd, tlu, tld, tru, trd, start, trans_m);
    add_texture(lu, ld, ru, rd, start);
    start += 36;
    */
    // 12 * 12 * 36 = 5184 vertices
    for(int i = 0; i < 12; i++){
        for(int j = 0; j < 12; j++) {
            double x_coord = -5.5 + j;
            double z_coord = -5.5 + i;
            mat4 trans_m = translation_m(x_coord, -0.6, z_coord);
            add_cube(blu, bld, bru, brd, tlu, tld, tru, trd, start, trans_m);
            add_texture(lu, ld, ru, rd, start);
            start += 36;
        }
    }
    printf("start is now %d\n", start);
}

void setup() {
    init_maze();
    print_maze();
    generate_maze_pole();
    generate_maze_walls();
    generate_maze_floors();
    check_cells();
    model_view = look_at(0, 4, 0, 0, 0, 0, 0, 0, -1);
    projection = perspective(-1.3, 1.3, -1.3, 1.3, -0.9, -15);
}



void init(void)
{
    int width = 800;
    int height = 800;
    GLubyte my_texels[width][height][3];

    FILE *fp = fopen("p2texture04.raw", "r");
    fread(my_texels, width * height * 3, 1, fp);
    fclose(fp);

    setup();

    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLuint mytex[1];
    glGenTextures(1, mytex);
    glBindTexture(GL_TEXTURE_2D, mytex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, my_texels);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    int param;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &param);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(tex_coords), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(tex_coords), tex_coords);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0 + sizeof(vertices));


    model_view_location = glGetUniformLocation(program, "model_view_matrix");
    projection_location = glGetUniformLocation(program, "projection_matrix");
    texture_location = glGetUniformLocation(program, "texture");
    glUniform1i(texture_location, 0);

    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);

    glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat *) &model_view);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat *) &projection);
    
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    if(key == 'q')
        glutLeaveMainLoop();
    if(key == 's')
        isAnimating = 1;;
    //glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, 512, 512);
}

/*
void adjust() {
    int t;
    printf("1. adjust eye point\n2. adjust perspective\n");
    printf("enter a number: ");
    scanf("%d", &t);
    if(t == 1){
        float eyex, eyey, eyez, atx, aty, atz, upx, upy, upz;
        printf("    eye: "); 
        scanf("%f, %f, %f", &eyex, &eyey, &eyez);
        printf("    at : "); 
        scanf("%f, %f, %f", &atx, &aty, &atz);
        printf("    up : "); 
        scanf("%f, %f, %f", &upx, &upy, &upz);
        model_view = look_at(eyex, eyey, eyez, atx, aty, atz, upx, upy, upz);
    }else{
        float left, right, bottom, top, near, far;
        printf("    perspective: "); scanf("%f, %f, %f, %f, %f, %f", &left, &right, &bottom, &top, &near, &far);
        projection = perspective(left, right, bottom, top, near, far);
    }
}
*/

double cur_x;
double cur_z;
double cur_y;
double x_step = 0;
double z_step = 0;
bool turned_right = false;
bool if_forward = false;
bool shortest_path = false;

void record(int r, int c){
    if(cursor <= 1){
        vec2 temp = {r, c};
        cell_visit[cursor++] = temp;
        return;
    }
    if(cell_visit[cursor-2].x == r && cell_visit[cursor-2].y == c){
        cursor -= 1;
    }else{
        vec2 temp = {r, c};
        cell_visit[cursor++] = temp;
    }
}

void finish_solve() {
    printf("You have solved the maze\n");
    for(int i = 0; i < cursor; i++){
        printf("(%f, %f)\n", cell_visit[i].x, cell_visit[i].y);
    }
}

bool is_turnback = false;
int turn_back_num = 0;
void determine_pt(){
    if(is_turnback) {
        if(turn_back_num == 2){
            shortest_path = true;
            is_turnback = false;
            currentState = WALK_FORWARD;
            return;
        }
        currentState = TURN_LEFT;
        turn_back_num++;
        return;
    }
    int r = round(cur_z + 3.5);
    int c = round(cur_x + 3.5);
    cell cur = cells[r][c];
    if(!shortest_path){
        if(r == 7 && c == 8){   //finish solving the maze
            finish_solve();
            is_turnback = true;
            currentState = TURN_LEFT;
            turn_back_num++;
            return;
        }else if(r < 0 || r > 7 || c < 0 || c > 7){
            currentState = WALK_FORWARD;
            return;
        }

        if(if_forward) {
            record(r, c);
            if_forward = false;
        }
        if(turned_right){
            currentState = WALK_FORWARD;
            turned_right = false;
            return;
        }
        if(x_step > 0){ //currently go east
            if (cur.south){
                if(cur.east)
                    currentState = TURN_LEFT;
                else
                    currentState = WALK_FORWARD;
            }else{  // turn right
                currentState = TURN_RIGHT;
            }
        }
        else if (x_step < 0)    //currently go west
        {
            if(cur.north){
                if(cur.west)
                    currentState = TURN_LEFT;
                else{
                    currentState = WALK_FORWARD;
                }
            }
            else{
                currentState = TURN_RIGHT;
            }
        }
        else if (z_step > 0)    // currently go south
        {
            if(cur.west){
                if(cur.south){

                    currentState = TURN_LEFT;
                }
                else{
                    currentState = WALK_FORWARD;
                }
            }else{
                currentState = TURN_RIGHT;
            }

        }else                   // currently go north
        {
            if(cur.east){
                if(cur.north)
                    currentState = TURN_LEFT;
                else{
                    currentState = WALK_FORWARD;
                }
            }else{
                currentState = TURN_RIGHT;
            }
        }
    }
    else{ //shortest path
        //printf("now it is at %d, %d\n", r, c);
        //printf("x_step is %f, z_Step is %f\n\n", x_step, z_step);
        if(r == 0 && c == -1){
            printf("Finish!\n");
            isAnimating = 0;
            return;
        }
        if(if_forward){
            if_forward = false;
            cursor--;
            int next_r, next_c;
            if(cursor == 0){
                next_r = 0;
                next_c = -1;
            }else{
                next_r = cell_visit[cursor - 1].x;
                next_c = cell_visit[cursor - 1].y;
            }
                
            //printf("next is %d, %d\n", next_r, next_c);
            if (next_r > r) //should go south
            {   
                if(x_step > 0)
                    currentState = TURN_RIGHT;
                else if(x_step < 0)
                    currentState = TURN_LEFT;
                else if(z_step > 0)
                    currentState = WALK_FORWARD;
                else{
                    printf("some error\n");
                    isAnimating = 0;
                }
            }
            else if (next_r < r){ // go north
                if(x_step > 0)
                    currentState = TURN_LEFT;
                else if(x_step < 0)
                    currentState = TURN_RIGHT;
                else if(z_step < 0)
                    currentState = WALK_FORWARD;
                else{
                    printf("some error\n");
                    isAnimating = 0;
                }
            }
            else if (next_c > c) { // go east
                if(x_step > 0)
                    currentState = WALK_FORWARD;
                else if(z_step < 0)
                    currentState = TURN_RIGHT;
                else if(z_step > 0)
                    currentState = TURN_LEFT;
                else{
                    printf("some error\n");
                    isAnimating = 0;
                }
            }
            else {  // go west
                if(x_step < 0)
                    currentState = WALK_FORWARD;
                else if(z_step < 0)
                    currentState = TURN_LEFT;
                else if(z_step > 0)
                    currentState = TURN_RIGHT;
                else{
                    printf("some error\n");
                    isAnimating = 0;
                }
            }
        }
        else{
            currentState = WALK_FORWARD;
        }
        
    }        
}



double cur_turn = 0;
double turn_angle = 0;
int turn_step = 150;
double flying_angle = 0;
double fly_r = 0;
int down_time = 0;
double down_steps = 400;
int steps_per_unit = 150;
int steps = 0;

void idle(void) {
    if(isAnimating) {
        if(currentState == FLYING_TO_START){
            float xz_dis = -4.7 / down_steps * down_time;
            float y_dis = 4 - 2 / down_steps * down_time;
            model_view = look_at(xz_dis, y_dis, xz_dis, 0, 0, 0, 0, 1, 0);
            down_time++;
            if(down_time > down_steps){
                //model_view = look_at(-4.7, 2, -4.7, 0, 0, 0, 0, 1, 0);
                fly_r = sqrt(pow(4.7, 2) + pow(4.7, 2));
                currentState = FLYING_AROUND;
                down_time = 0;
            }    
        }
        else if (currentState == FLYING_AROUND){
            cur_x = cos(-0.75*PI - flying_angle) * fly_r;
            cur_z = sin(-0.75*PI - flying_angle) * fly_r;
            model_view = look_at(cur_x, 2, cur_z, 0, 0, 0, 0, 1, 0);
            flying_angle += PI / 800;
            if(flying_angle >= 2 * PI)
                currentState = FLYING_DOWN;
        }
        else if (currentState == FLYING_DOWN){
            model_view = look_at(-4.7-(1.2/down_steps)*down_time, 1 - 0.7 / down_steps * down_time, -4.7+1.2/down_steps*down_time, 3.5/down_steps * down_time, 0.3/down_steps*down_time, -3.5/down_steps*down_time, 0, 1, 0);
            double lens = 1.3 - 1.1 / down_steps * down_time;
            projection = perspective(-lens, lens, -lens, lens, -1+down_time*0.7/down_steps, -15);
            down_time ++;
            if(down_time > down_steps){
                currentState = WALK_FORWARD;
                cur_x = -5.5;
                cur_z = -3.5;
                cur_y = 0.3;
                x_step = (double) 1 / steps_per_unit;
                turn_angle = PI / 2 / turn_step;
                //projection = perspective(-0.2, 0.2, -0.2, 0.2, -0.3, -10);
            }
        }
        else if (currentState == WALK_FORWARD) {
            //if(steps==0)
            //    printf("now it is going front\n");
            steps++;
            cur_x += x_step;
            cur_z += z_step;
            model_view = look_at(cur_x, cur_y, cur_z, cur_x+x_step, cur_y, cur_z+z_step, 0, 1, 0);
            if(steps == steps_per_unit){
                steps = 0;
                if_forward = true;
                determine_pt();
            }
        }
        else if (currentState == TURN_LEFT) {
            //if(cur_turn == 0)
            //    printf("it is now turning left\n");
            cur_turn ++;
            vec4 eye = {cur_x, cur_y, cur_z, 1};
            vec4 direction = {x_step, 0, z_step, 0};
            mat4 rotate = rotate_y_m(turn_angle * cur_turn);
            direction = m_v_mult(rotate, direction);
            vec4 at = vec_add(eye, direction);
            model_view = look_at(cur_x, cur_y, cur_z, at.x, at.y, at.z, 0, 1, 0);
            if(cur_turn == turn_step){
                if(z_step == 0){
                    z_step = -x_step;
                    x_step = 0;
                }
                else{
                    x_step = z_step;
                    z_step = 0;
                }
                cur_turn = 0;
                determine_pt();
                
            }    
        }
        else if (currentState == TURN_RIGHT) {
            //if(cur_turn == 0)
            //    printf("it is now turning right\n");
            cur_turn ++;
            vec4 eye = {cur_x, cur_y, cur_z, 1};
            vec4 direction = {x_step, 0, z_step, 0};
            mat4 rotate = rotate_y_m(- turn_angle * cur_turn);
            direction = m_v_mult(rotate, direction);
            vec4 at = vec_add(eye, direction);
            model_view = look_at(cur_x, cur_y, cur_z, at.x, at.y, at.z, 0, 1, 0);
            if(cur_turn == turn_step){
                turned_right = true;
                if(z_step == 0){
                    z_step = x_step;
                    x_step = 0;
                }
                else{
                    x_step = -z_step;
                    z_step = 0;
                }
                cur_turn = 0;
                determine_pt();
            }
        }
        glutPostRedisplay();
    }else {
        //adjust();
        //glutPostRedisplay();
    }
}


int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(100,100);
    glutCreateWindow("maze-solver");
    glewInit();
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glutMainLoop();

    return 0;
}

void add_cube(vec4 blu, vec4 bld, vec4 bru, vec4 brd, vec4 tlu, vec4 tld, vec4 tru, vec4 trd, int start, mat4 trans_m) {
    vertices[start+0] = m_v_mult(trans_m, blu);
    vertices[start+1] = m_v_mult(trans_m, bld);
    vertices[start+2] = m_v_mult(trans_m, bru);
    vertices[start+3] = m_v_mult(trans_m, bld);
    vertices[start+4] = m_v_mult(trans_m, brd);
    vertices[start+5] = m_v_mult(trans_m, bru);

    vertices[start+6] = m_v_mult(trans_m, tlu);
    vertices[start+7] = m_v_mult(trans_m, tld);
    vertices[start+8] = m_v_mult(trans_m, tru);
    vertices[start+9] = m_v_mult(trans_m, tld);
    vertices[start+10] = m_v_mult(trans_m, trd);
    vertices[start+11] = m_v_mult(trans_m, tru);

    vertices[start+12] = m_v_mult(trans_m, tld);
    vertices[start+13] = m_v_mult(trans_m, bld);
    vertices[start+14] = m_v_mult(trans_m, trd);
    vertices[start+15] = m_v_mult(trans_m, bld);
    vertices[start+16] = m_v_mult(trans_m, brd);
    vertices[start+17] = m_v_mult(trans_m, trd);

    vertices[start+18] = m_v_mult(trans_m, trd);
    vertices[start+19] = m_v_mult(trans_m, brd);
    vertices[start+20] = m_v_mult(trans_m, tru);
    vertices[start+21] = m_v_mult(trans_m, brd);
    vertices[start+22] = m_v_mult(trans_m, bru);
    vertices[start+23] = m_v_mult(trans_m, tru);

    vertices[start+24] = m_v_mult(trans_m, tru);
    vertices[start+25] = m_v_mult(trans_m, bru);
    vertices[start+26] = m_v_mult(trans_m, tlu);
    vertices[start+27] = m_v_mult(trans_m, bru);
    vertices[start+28] = m_v_mult(trans_m, blu);
    vertices[start+29] = m_v_mult(trans_m, tlu);

    vertices[start+30] = m_v_mult(trans_m, tlu);
    vertices[start+31] = m_v_mult(trans_m, blu);
    vertices[start+32] = m_v_mult(trans_m, tld);
    vertices[start+33] = m_v_mult(trans_m, blu);
    vertices[start+34] = m_v_mult(trans_m, bld);
    vertices[start+35] = m_v_mult(trans_m, tld);
}

void add_texture(vec2 lu, vec2 ld, vec2 ru, vec2 rd, int start) {
    for(int j = 0; j < 6; j++) {
        tex_coords[start + 6*j +0] = lu;
        tex_coords[start + 6*j +1] = ld;
        tex_coords[start + 6*j +2] = ru;
        tex_coords[start + 6*j +3] = ld;
        tex_coords[start + 6*j +4] = rd;
        tex_coords[start + 6*j +5] = ru;
    }
}