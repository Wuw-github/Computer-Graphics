/*
 * project3.c
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


void add_texture(vec2, vec2, vec2, vec2, int);

int num_vertices = 18366;

vec4 vertices[18366];
vec4 sphere_vertices[1080];
vec4 colors[18366];
vec2 tex_coords[18366];
vec4 normal[18366];

vec2 tex_lu[16] = {{0.01, 0.01}, {0.26, 0.01}, {0.51, 0.01}, {0.76, 0.01}, 
                    {0.01, 0.26}, {0.26, 0.26}, {0.51, 0.26}, {0.76, 0.26},
                    {0.01, 0.511}, {0.26, 0.511}, {0.51, 0.511}, {0.76, 0.511},
                    {0.01, 0.7625}, {0.26, 0.7625}, {0.51, 0.7625}, {0.76, 0.7625}};
double size[16] = {0.23, 0.23, 0.23, 0.23,
                    0.23, 0.23, 0.23, 0.23,
                    0.23, 0.23, 0.23, 0.23, 
                    0.2275, 0.2275, 0.2275, 0.2275};

int isAnimating = 0;

vec4 ball_position[16];
            

mat4 model_view = {{1, 0, 0, 0},
                    {0, 1, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1}};
mat4 projection = {{1, 0, 0, 0},
                    {0, 1, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1}};

mat4 ctms[18];

vec4 light_position = {0, 1, 0, 1};

float eyex = 0;
float eyey = 2;
float eyez = 5;
float upx = 0, upy = 1, upz = 0;

                    
GLuint model_view_location;
GLuint projection_location;
GLuint ctm_location;
GLuint texture_location;
GLuint use_texture_location;
GLuint light_position_location;
GLuint is_shadow_location;


int layer = 4;
// 1080 vertices per sphere
void generate_sphere(double r) {
    vec4 top = {0, r, 0, 1};
    vec4 bottom = {0, -r, 0, 1};
    double top_dis = r / 40;
    double dis = (r - top_dis) / layer;
    vec4 circles[2*layer+1][20];
    double angle = 2 * PI / 20;

    for(int j = 0; j < 2*layer+1; j++){
        double height = (r-top_dis) - dis * j;
        double cur_r = sqrt(pow(r, 2) - pow(height, 2));
        for(int i = 0; i < 20; i++){
            vec4 temp = {cur_r*cos(angle*i), height, cur_r*sin(angle*i), 1};
            circles[j][i] = temp;
        }
    }
    int n = 0;

    for(int i = 0; i < 20; i++){
        sphere_vertices[n++] = top;
        sphere_vertices[n++] = circles[0][(i+1) % 20];
        sphere_vertices[n++] = circles[0][i];
    }
    for(int i = 0; i < 2*layer; i++){
        for(int j = 0; j < 20; j++){
            sphere_vertices[n++] = circles[i][j];
            sphere_vertices[n++] = circles[i+1][(j+1) % 20];
            sphere_vertices[n++] = circles[i+1][j];
            sphere_vertices[n++] = circles[i][j];
            sphere_vertices[n++] = circles[i][(j+1) % 20];
            sphere_vertices[n++] = circles[i+1][(j+1) % 20];
        }
    }

    for(int i = 0; i < 20; i++){
        sphere_vertices[n++] = bottom;
        sphere_vertices[n++] = circles[2*layer][i];
        sphere_vertices[n++] = circles[2*layer][(i+1) % 20];
    }
    mat4 rot = rotate_x_m(PI / 2);
    for(int i = 0; i < 1080; i++){
        sphere_vertices[i] = m_v_mult(rot, sphere_vertices[i]);
    }
}


void setup() {
    generate_sphere(0.1);
    int num = 0;
    // ball vertices
    mat4 rotate = rotate_x_m(-PI / 2);
    for(int i = 0; i < 16; i++) {
        int row = i / 4;
        int col = i % 4;
        double z = -0.3 + row * 0.2;
        double x = -0.3 + col * 0.2;
        mat4 trans = translation_m(x, 0.1, z);
        vec4 pos = {x, 0.1, z, 1};
        ball_position[i] = pos;
        double tex_start_x = tex_lu[i].x;
        double tex_start_y = tex_lu[i].y;
        double size_x = 0.23;
        double size_y = size[i];
        for(int j = 0; j < 540; j++) {
            vec4 t_normal = sphere_vertices[j];
            t_normal = m_v_mult(rotate, t_normal);
            t_normal.w = 0.0;
            t_normal = vec_normal(t_normal);
            double s_x = sphere_vertices[j].x;
            double s_y = sphere_vertices[j].y;
            double tex_x = tex_start_x + (s_x + 0.1) / 0.2 * size_x;
            double tex_y = tex_start_y + (1 - (s_y + 0.1) / 0.2) * size_y;
            vec2 tex = {tex_x, tex_y};
            vertices[num] = m_v_mult(trans, m_v_mult(rotate, sphere_vertices[j]));
            tex_coords[num] = tex;
            normal[num] = t_normal;
            num++;
        }
        for(int j = 540; j < 1080; j++) {
            vec4 t_normal = sphere_vertices[j];
            t_normal = m_v_mult(rotate, t_normal);
            t_normal.w = 0.0;
            t_normal = vec_normal(t_normal);
            double s_x = sphere_vertices[j].x;
            double s_y = sphere_vertices[j].y;
            double tex_x = tex_start_x + size_x - (s_x + 0.1) / 0.2 * size_x;
            double tex_y = tex_start_y + (1 - (s_y + 0.1) / 0.2) * size_y;
            vec2 tex = {tex_x, tex_y};
            vertices[num] = m_v_mult(trans, m_v_mult(rotate, sphere_vertices[j]));
            tex_coords[num] = tex;
            normal[num] = t_normal;
            num++;
        }

    }
    // light 
    generate_sphere(0.05);
    mat4 trans = translation_m(light_position.x, light_position.y, light_position.z);
    vec2 bogus_tex = {0, 0};
    for(int i = 0; i < 1080; i++) {
        vec4 t_normal = sphere_vertices[i];
        t_normal = vec_scalar_mult(-1, t_normal);
        t_normal.w = 0.0;
        t_normal = vec_normal(t_normal);
        tex_coords[num] = bogus_tex;
        vertices[num] = m_v_mult(trans, sphere_vertices[i]);
        normal[num] = t_normal;
        num++;
    }
    // table
    vec4 lu = {-5, 0, -5, 1};
    vec4 lb = {-5, 0, 5, 1};
    vec4 ru = {5, 0, -5, 1};
    vec4 rb = {5, 0, 5, 1};
    vec4 t_normal = {0, 1, 0, 0};
    for(int i = 0; i < 6; i++){
        tex_coords[num+i] = bogus_tex;
        normal[num+i] = t_normal;
    }
    vertices[num++] = lu;
    vertices[num++] = lb;
    vertices[num++] = ru;
    vertices[num++] = lb;
    vertices[num++] = rb;
    vertices[num++] = ru;

    // add color to all coordinates
    num = 0;
    // all balls are bogus color
    for(int i = 0; i < num_vertices - 1086; i++) {
        vec4 color = {1, 1, 1, 1.0};
        colors[num++] = color;
    }
    // light is white
    for(int i = 0; i < 1080; i++) {
        vec4 color = {1, 1, 1, 1};
        colors[num++] = color;
    }
    // table is green
    for(int i = 0; i < 6; i++) {
        vec4 color = {0, 1, 0, 1};
        colors[num++] = color;
    }

    for(int i = 0; i < 18; i++) {
        mat4 id = {{1, 0, 0, 0},
                    {0, 1, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1}};
        ctms[i] = id;
    }
}


void init(void)
{
    int width = 512;
    int height = 512;
    GLubyte my_texels[width][height][3];

    FILE *fp = fopen("pb_512_512.raw", "r");
    fread(my_texels, width * height * 3, 1, fp);
    fclose(fp);

    setup();

    model_view = look_at(eyex, eyey, eyez, 0, 0, 0, upx, upy, upz);
    projection = perspective(-0.1, 0.1, -0.1, 0.1, -0.5, -20);

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors) + sizeof(tex_coords) + sizeof(normal), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), sizeof(tex_coords), tex_coords);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors) + sizeof(tex_coords), sizeof(normal), normal);


    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) sizeof(vertices));

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0 + (sizeof(vertices) + sizeof(colors)));
    
    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0 + (sizeof(vertices) + sizeof(colors) + sizeof(tex_coords)));



    model_view_location = glGetUniformLocation(program, "model_view_matrix");
    projection_location = glGetUniformLocation(program, "projection_matrix");
    ctm_location = glGetUniformLocation(program, "ctm");
    use_texture_location = glGetUniformLocation(program, "use_texture");
    light_position_location = glGetUniformLocation(program, "light_position");
    is_shadow_location = glGetUniformLocation(program, "is_shadow");
    
    /*
    texture_location = glGetUniformLocation(program, "texture");
    glUniform1i(texture_location, 0);
    */
    
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

    glUniform4fv(light_position_location, 1, (GLfloat *) &light_position);

    glUniform1i(is_shadow_location, 0);
    //draw the balls
    glUniform1i(use_texture_location, 1);
    for(int i = 0; i < 16; i++) {
        glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctms[i]);
        glDrawArrays(GL_TRIANGLES, 1080 * i, 1080);
    }
    //draw light and table
    glUniform1i(use_texture_location, 0);
    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctms[16]);
    glDrawArrays(GL_TRIANGLES, 1080 * 16, 1080);
    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctms[17]);
    glDrawArrays(GL_TRIANGLES, 1080 * 17, 6);
    //draw shadow
    glUniform1i(is_shadow_location, 1);
    for(int i = 0; i < 16; i++) {
        glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctms[i]);
        glDrawArrays(GL_TRIANGLES, 1080 * i, 1080);
    }

    glutSwapBuffers();
}

mat4 calculate_rotation_m(vec4 v1, vec4 v2, float r_angle, vec4 r, int flag){
    vec4 r_v = vec_cross_product(v1, v2);
    //rotation vector
    if (flag == 0)
        r_v = vec_normal(r_v);
    else
        r_v = vec_normal(r);

    float d = sqrt(pow(r_v.y, 2) + pow(r_v.z, 2));
    mat4 M;
    if(d != 0){
        mat4 R_x = {{1, 0, 0, 0},
                    {0, r_v.z / d, r_v.y / d, 0},
                    {0, -r_v.y / d, r_v.z / d, 0},
                    {0, 0, 0, 1}};
        mat4 R_y = {{d, 0, r_v.x, 0},
                    {0, 1, 0, 0},
                    {-r_v.x, 0, d, 0},
                    {0, 0, 0, 1}};
        mat4 R_z = rotate_z_m(r_angle);
        M = m_multiplication(R_y, R_x);
        M = m_multiplication(R_z, M);
        M = m_multiplication(m_transpose(R_y), M);
        M = m_multiplication(m_transpose(R_x), M);
    }
    else{
        M = rotate_x_m(r_v.x * r_angle);
    }
    
    return M;
}
bool is_line_up = false;
bool follow_ball = false;
int ball_followed;
void keyboard(unsigned char key, int mousex, int mousey)
{
    if(key == 'q')
        glutLeaveMainLoop();
    if(key == 's'){
        if(!is_line_up){
            isAnimating = 1;
        }else{
            isAnimating = 2;
        }
        
    }
    // change light location
    if(key == 't'){
        mat4 trans = translation_m(0.1, 0, 0);
        ctms[16] = m_multiplication(trans, ctms[16]);
        light_position.x += 0.1;
    }
    if(key == 'y'){
        mat4 trans = translation_m(0, 0.1, 0);
        ctms[16] = m_multiplication(trans, ctms[16]);
        light_position.y += 0.1;
    }
    if(key == 'u'){
        mat4 trans = translation_m(0, 0, 0.1);
        ctms[16] = m_multiplication(trans, ctms[16]);
        light_position.z += 0.1;
    }
    if(key == 'i'){
        mat4 trans = translation_m(-0.1, 0, 0);
        ctms[16] = m_multiplication(trans, ctms[16]);
        light_position.x -= 0.1;
    }
    if(key == 'o'){
        mat4 trans = translation_m(0, -0.1, 0);
        ctms[16] = m_multiplication(trans, ctms[16]);
        light_position.y -= 0.1;
    }
    if(key == 'p'){
        mat4 trans = translation_m(0, 0, -0.1);
        ctms[16] = m_multiplication(trans, ctms[16]);
        light_position.z -= 0.1;

    }

    //change eye location
    vec4 temp = {eyex, eyey, eyez, 0};
    if(key == 'h'){
        temp = vec_normal(temp);
        temp = vec_scalar_mult(0.1, temp);
        eyex += temp.x;
        eyey += temp.y;
        eyez += temp.z;
        model_view = look_at(eyex, eyey, eyez, 0, 0, 0, upx, upy, upz);
    }
    if(key == 'j'){
        temp = vec_normal(temp);
        temp = vec_scalar_mult(0.1, temp);
        eyex -= temp.x;
        eyey -= temp.y;
        eyez -= temp.z;
        model_view = look_at(eyex, eyey, eyez, 0, 0, 0, upx, upy, upz);
    }
    if(key == 'k'){ // left
        mat4 rotate = rotate_y_m(-2 * PI / 90);
        temp = m_v_mult(rotate, temp);
        eyex = temp.x;
        eyey = temp.y;
        eyez = temp.z;
        model_view = look_at(eyex, eyey, eyez, 0, 0, 0, upx, upy, upz);
    }
    if(key == 'l'){
        mat4 rotate = rotate_y_m(2 * PI / 90);
        temp = m_v_mult(rotate, temp);
        eyex = temp.x;
        eyey = temp.y;
        eyez = temp.z;
        model_view = look_at(eyex, eyey, eyez, 0, 0, 0, upx, upy, upz);
    }
    if(key == 'n'){
        vec4 v1 = {eyex, eyey, eyez, 0};
        vec4 v2 = {upx, upy, upz, 0};
        float angle = 2 * PI / 90;
        mat4 M = calculate_rotation_m(v1, v2, angle, v1, 0);
        vec4 t = m_v_mult(M, temp);
        if(t.z == 0) {
            angle = 2 * PI / 90 + 2 * PI /180;
            mat4 M = calculate_rotation_m(v1, v2, angle, v1, 0);
            t = m_v_mult(M, temp);
        }
        eyex = t.x;
        eyey = t.y;
        eyez = t.z;
        model_view = look_at(eyex, eyey, eyez, 0, 0, 0, upx, upy, upz);
    }
    if(key == 'm'){
        vec4 v1 = {eyex, eyey, eyez, 0};
        vec4 v2 = {upx, upy, upz, 0};
        float angle = 2 * PI / 90;
        mat4 M = calculate_rotation_m(v1, v2, -angle, v1, 0);
        vec4 t = m_v_mult(M, temp);
        if(t.z == 0) {
            angle = 2 * PI / 90 + 2 * PI /180;
            mat4 M = calculate_rotation_m(v1, v2, -angle, v1, 0);
            t = m_v_mult(M, temp);
        }
        eyex = t.x;
        eyey = t.y;
        eyez = t.z;
        model_view = look_at(eyex, eyey, eyez, 0, 0, 0, upx, upy, upz);
    }
    if(key =='1') {
        ball_followed = 0;
        follow_ball = true;
    }
    if(key =='2') {
        ball_followed = 1;
        follow_ball = true;
    }
    if(key =='3') {
        ball_followed = 2;
        follow_ball = true;
    }
    if(key =='4') {
        ball_followed = 3;
        follow_ball = true;
    }
    if(key =='5') {
        ball_followed = 4;
        follow_ball = true;
    }
    if(key =='6') {
        ball_followed = 5;
        follow_ball = true;
    }
    if(key =='7') {
        ball_followed = 6;
        follow_ball = true;
    }
    if(key =='8') {
        ball_followed = 7;
        follow_ball = true;
    }
    if(key =='9') {
        ball_followed = 8;
        follow_ball = true;
    }
    if(key =='a') {
        ball_followed = 9;
        follow_ball = true;
    }
    if(key =='b') {
        ball_followed = 10;
        follow_ball = true;
    }
    if(key =='c') {
        ball_followed = 11;
        follow_ball = true;
    }
    if(key =='d') {
        ball_followed = 12;
        follow_ball = true;
    }
    if(key =='e') {
        ball_followed = 13;
        follow_ball = true;
    }
    if(key =='f') {
        ball_followed = 14;
        follow_ball = true;
    }
    if(key == '0'){
        follow_ball = false;
        model_view = look_at(eyex, eyey, eyez, 0, 0, 0, upx, upy, upz);
    }
    glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, 512, 512);
}

int step_1 = 600;
int cur_step = 0;
int cur_ball = 0;
float x_step = 0;
float z_step = 0;
float des_z = 0;
mat4 rot;

int indicate = 0;
int rotate_step = 0;
int rotate_iteration = 800; // how many steps for one ball to rotate one circle
float d = 2;
void idle(void) {
    if(isAnimating == 0)
        return;
    if(isAnimating == 1){
        vec4 cur_pos = ball_position[cur_ball];
        if(cur_step == 0){
            des_z = -3.0 + 0.2 * cur_ball;
            x_step = (0 - cur_pos.x) / step_1;
            z_step = (des_z - cur_pos.z) / step_1;

            vec4 about = {x_step, 0, z_step, 0};
            about = vec_normal(about);
            vec4 y = {0, 1, 0, 0};
            float dis_move = sqrt(x_step * x_step + z_step * z_step);
            float angle = dis_move / 0.1;
            vec4 temp = {0, 0, 0, 0};
            rot = calculate_rotation_m(y, about, angle, temp, 0);
        }
        mat4 trans_origin = translation_m(-cur_pos.x, -cur_pos.y, -cur_pos.z);
        mat4 trans_back = translation_m(cur_pos.x, cur_pos.y, cur_pos.z);
        mat4 trans_f = translation_m(x_step, 0, z_step);
        mat4 M = m_multiplication(rot, trans_origin);
        M = m_multiplication(trans_back, M);
        M = m_multiplication(trans_f, M);
        ctms[cur_ball] = m_multiplication(M, ctms[cur_ball]);
        ball_position[cur_ball].x += x_step;
        ball_position[cur_ball].z += z_step;
        cur_step++;
        if(cur_step == step_1){
            vec4 t = {0, 0.1, des_z, 1};
            ball_position[cur_ball] = t;
            cur_step = 0;
            cur_ball = cur_ball + 1;
            step_1 -= step_1 / 15;
            if(cur_ball == 16){
                isAnimating = 0;
                is_line_up = true;
            }
        }
    }
    if(isAnimating == 2){
        for(int i = 0; i < 15; i++){
            //int i = 14;
            vec4 cur_pos = ball_position[i];

            float r = fabs(cur_pos.z);
            float angle;
            if(r == 0)
                angle = 0;
            else
                angle = 2 * PI / (rotate_iteration + 50 * (15 - i));
            mat4 rot_y = rotate_y_m(angle);
            vec4 about = {-cur_pos.x, -cur_pos.y, -cur_pos.z, 0};
            float dis_per_it = angle * r;   // distance of moving per iteration
            angle = dis_per_it / 0.1;
            vec4 temp1 = {1, 0, 0, 0};
            vec4 temp2 = {0, 0, 1, 0};
            mat4 rot_self = calculate_rotation_m(temp1, temp2, angle, about, 1);
            mat4 trans_origin = translation_m(-cur_pos.x, -cur_pos.y, -cur_pos.z);
            mat4 trans_back = translation_m(cur_pos.x, cur_pos.y, cur_pos.z);
            mat4 M = m_multiplication(rot_self, trans_origin);
            M = m_multiplication(trans_back, M);
            M = m_multiplication(rot_y, M);
            ctms[i] = m_multiplication(M, ctms[i]);
            vec4 temp = m_v_mult(rot_y, cur_pos);
            ball_position[i] = temp;
            if(follow_ball) {
                vec4 ball_pos = ball_position[ball_followed];
                float ball_r = sqrt(ball_pos.x * ball_pos.x + ball_pos.z * ball_pos.z);
                float view_r = sqrt(ball_r * ball_r + d * d);
                float theta_ball_view = atan(d / ball_r);
                float theta_ball = atan(ball_pos.z / ball_pos.x);
                if(ball_pos.x < 0)
                    theta_ball += PI;
                if(ball_pos.x == 0 && ball_pos.z > 0)
                    theta_ball = PI;
                else if(ball_pos.x == 0 && ball_pos.z < 0)
                    theta_ball = -PI;
                float theta_view = theta_ball + theta_ball_view;
                float view_x = view_r * cos(theta_view);
                float view_z = view_r * sin(theta_view);
                model_view = look_at(view_x, 0.1, view_z, ball_pos.x, ball_pos.y, ball_pos.z, 0, 1, 0);
            }
        }
    }
    glutPostRedisplay();
}


int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Project3");
    glewInit();
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glutMainLoop();

    return 0;
}
