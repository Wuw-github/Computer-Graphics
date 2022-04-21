/*
 * project1.c
 *
 * An OpenGL source code template.
 */


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>
#include <math.h>

#include "../mylib/initShader.h"
#include "../mylib/linear_alg.h"

#define PI (3.141592653589793)
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

int num_t = 60;
int num_vertices = 19200;

vec4 vertices[19200];

vec4 colors[19200];

vec4 circles[160][20];

vec4 circles_color[160][20];
            
mat4 ctm = {{1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}};

GLuint ctm_location;




void generate_torus(double d, double r){
    float angle = 2 * PI / 20;
    //create the first circle
    for(int i = 0; i < 20; i++){
        vec4 t = {r*cos(angle * i) + d, r*sin(angle * i), 0, 1};
        circles[0][i] = t;
    }
    //create following circle
    angle = 2 * PI / 40;
    mat4 r_m = rotate_y_m(-angle);
    for(int i = 1; i < 40; i++){
        for(int j = 0; j < 20; j++){
            circles[i][j] = m_v_mult(r_m, circles[i-1][j]);
        }
    }

    int n = 0;
    for(int i = 0; i < 40; i++){
        for(int j = 0; j < 20; j++){
            vertices[n++] = circles[i][j];
            vertices[n++] = circles[i][(j+1) % 20];
            vertices[n++] = circles[(i+1) % 40][j];
            vertices[n++] = circles[(i+1) % 40][j];
            vertices[n++] = circles[i][(j+1) % 20];
            vertices[n++] = circles[(i+1) % 40][(j+1) % 20];
        }
    }

    for(int i = 0; i < 1600; i++){
        GLfloat cx = (rand() % 10000) / 10000.0;
        GLfloat cy = (rand() % 10000) / 10000.0;
        GLfloat cz = (rand() % 10000) / 10000.0;
        vec4 color = {cx, cy, cz, 1.0};
        colors[i*3] = color;
        colors[i*3+1] = color;
        colors[i*3+2] = color;
    }
}

void generate_spring(double d, double r){
    float angle = 2 * PI / 20;
    //create the first circle
    for(int i = 0; i < 20; i++){
        vec4 t = {r*cos(angle * i) + d, r*sin(angle * i) - 0.7, 0, 1};
        circles[0][i] = t;
    }
    //create following circle
    angle = 2 * PI / 40;
    mat4 r_m = rotate_y_m(-angle);

    for(int i = 1; i < 160; i++){
        for(int j = 0; j < 20; j++){
            circles[i][j] = m_v_mult(r_m, circles[i-1][j]);
            circles[i][j].y += 0.008;
        }
    }

    int n = 0;
    for(int i = 0; i < 159; i++){
        for(int j = 0; j < 20; j++){
            vertices[n++] = circles[i][j];
            vertices[n++] = circles[i][(j+1) % 20];
            vertices[n++] = circles[(i+1) % 160][j];
            vertices[n++] = circles[(i+1) % 160][j];
            vertices[n++] = circles[i][(j+1) % 20];
            vertices[n++] = circles[(i+1) % 160][(j+1) % 20];
        }
    }

    //create the end circle
    float x = (circles[0][0].x + circles[0][10].x) / 2;
    float y = (circles[0][0].y + circles[0][10].y) / 2;
    float z = (circles[0][0].z + circles[0][10].z) / 2;

    vec4 center_b = {x, y, z, 1};
    for(int i = 0; i < 20; i++){
        vertices[n++] = circles[0][i];
        vertices[n++] = center_b;
        vertices[n++] = circles[0][(i+1) % 20];
    }

    //create the top circle
    x = (circles[159][0].x + circles[159][10].x) / 2;
    y = (circles[159][0].y + circles[159][10].y) / 2;
    z = (circles[159][0].z + circles[159][10].z) / 2;

    vec4 center_t = {x, y, z, 1};
    for(int i = 0; i < 20; i++){
        vertices[n++] = circles[159][i];
        vertices[n++] = circles[159][(i+1) % 20];
        vertices[n++] = center_t;
    }

    for(int i = 0; i < 6400; i++){
        GLfloat cx = (rand() % 10000) / 10000.0;
        GLfloat cy = (rand() % 10000) / 10000.0;
        GLfloat cz = (rand() % 10000) / 10000.0;
        vec4 color = {cx, cy, cz, 1.0};
        colors[i*3] = color;
        colors[i*3+1] = color;
        colors[i*3+2] = color;
    }
}





float z_pos(float x, float y){
    return sqrt(256*256- x*x - y*y);
}

float x_1, y_1, z_1, x_2, y_2, z_2;
GLboolean if_rotate;

void calculate_rotation_m(){
    vec4 v1 = {x_1, y_1, z_1, 0.0};
    vec4 v2 = {x_2, y_2, z_2, 0.0};
    vec4 r_v = vec_cross_product(v1, v2);
    //rotation vector
    r_v = vec_normal(r_v);
    // rotation angle
    float r_angle = vec_dot_product(v1, v2) / (vec_mag(v1) * vec_mag(v2));
    r_angle = acos(r_angle);

    float d = sqrt(pow(r_v.y, 2) + pow(r_v.z, 2));
    mat4 R_x = {{1, 0, 0, 0},
                {0, r_v.z / d, r_v.y / d, 0},
                {0, -r_v.y / d, r_v.z / d, 0},
                {0, 0, 0, 1}};
    mat4 R_y = {{d, 0, r_v.x, 0},
                {0, 1, 0, 0},
                {-r_v.x, 0, d, 0},
                {0, 0, 0, 1}};
    mat4 R_z = rotate_z_m(r_angle);
    mat4 M = m_multiplication(R_y, R_x);
    M = m_multiplication(R_z, M);
    M = m_multiplication(m_transpose(R_y), M);
    M = m_multiplication(m_transpose(R_x), M);

    ctm = m_multiplication(M, ctm);
}

void motion(int x, int y){
    //printf("x is %i, y is %i\n", x, y);
    if(if_rotate){
        x_2 = x - 255;
        y_2 = 255 - y;
        z_2 = z_pos(x_2, y_2);
        if(pow(x_2, 2) + pow(y_2, 2) > pow(256, 2)){
            if_rotate = GL_FALSE;
        }
        if(if_rotate){
            calculate_rotation_m();
            x_1 = x_2;
            y_1 = y_2;
            z_1 = z_2;
            //printf("after: x is %i, y is %i\n", x_1, y_1);
            glutPostRedisplay();
        }
    }
}

void mouse(int button, int state, int x, int y){
    mat4 scale;
    if(button == 3 || button == 4){
        if (button == 3){
            scale = scaling_m(1.02, 1.02, 1.02) ;
        }else{
            scale = scaling_m(1/1.02, 1/1.02, 1/1.02);
        }
        ctm = m_multiplication(scale, ctm);
        glutPostRedisplay();
    }
    else if(button == GLUT_LEFT_BUTTON){
        if(state == GLUT_DOWN){
            x_1 = x - 255; 
            y_1 = 255 - y;
            if(pow(x_1, 2) + pow(y_1, 2) > pow(256, 2)){
                if_rotate = GL_FALSE;
            }else{
                if_rotate = GL_TRUE;
                z_1 = z_pos(x_1, y_1);
            }
        }
        else if(state == GLUT_UP){
            
        }
    }
}




void generate_vertices(double r){
    double angle = (2*PI) / num_t;
    vec4 top = {0, r /2 , 0, 1.0};
    for(int i = 0; i < num_t; i++) {
        vec4 left, right;
        left.x = r * cos(angle * i);
        left.y = - r / 2;
        left.z = r * sin(angle * i);
        left.w = 1.0;

        right.x = r * cos(angle * (i+1));
        right.y = -r / 2;
        right.z = r * sin(angle * (i+1));
        right.w = 1.0;

        vertices[3*i] = top;
        vertices[3*i+1] = right;
        vertices[3*i+2] = left;
    }

    vec4 center = {0, - r/2, 0, 1.0};
    for(int i = 0; i < num_t; i++) {
        vertices[num_t*3 + 3*i] = center;
        vertices[num_t*3 + 3*i+1] = vertices[3*i+2];
        vertices[num_t*3 + 3*i+2] = vertices[3*i+1];
    }
}

void generate_colors(void) {

    for(int i = 0; i < num_t; i++) {
        GLfloat cx = (rand() % 10000) / 10000.0;
        GLfloat cy = (rand() % 10000) / 10000.0;
        GLfloat cz = (rand() % 10000) / 10000.0;
        vec4 color = {cx, cy, cz, 1.0};
        
        colors[3*i] = color;
        colors[3*i+1] = color;
        colors[3*i+2] = color;
        colors[num_t*3+3*i] = color;
        colors[num_t*3+3*i+1] = color;
        colors[num_t*3+3*i+2] = color;
    }
}

void generate_triangle(){
    generate_vertices(0.7);
    generate_colors();
}

void init(void)
{
    generate_spring(0.5, 0.08);
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) sizeof(vertices));


    ctm_location = glGetUniformLocation(program, "ctm");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);

    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctm);

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    if(key == 'q')
        glutLeaveMainLoop();

    //glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, 512, 512);
}


int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Project1");
    glewInit();
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    //glutIdleFunc(idle);
    glutMainLoop();

    return 0;
}