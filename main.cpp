#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
# define STB_IMAGE_IMPLEMENTATION
#include <random>
#include "classes/Mesh.hpp"
#include "classes/Skybox.hpp"
#include "utils/utilities.hpp"
#include "classes/player.hpp"
#include "classes/camera.hpp"
using Clock = std::chrono::high_resolution_clock;

void write_to_minimap_fbo();
void draw_minimap();
void default_fbo();
void write_to_postprocessing_fbo();
void draw_objects();
void draw_postprocessing();
void set_uniform();
std::chrono::time_point<Clock> getTimePoint();
glm::mat4 matProj;
int minimap_switch = 0;
GLuint frame_texture;

void load_texture(GLuint *tex, const std::string &name);
CTPSCamera myCamera;
int blurr_duration = 0;
int pause = 0;
unsigned int quadVAO;
GLuint minimap_fbo, postprocessing_fbo;
GLuint minimap_depth_buffer, postprocessing_depth_buffer;
GLuint minimap_texture_buffer;
Mesh ground, ice,grass;
Mesh kwadrat;
Skybox sb;
CPlayer player;
CGround ground_col;
GLuint default_program, minimap_program;
std::vector<CSceneObject *> myColliders;
std::vector<Mesh *> nonColliders;
CSceneObject rock, tree, candy,mine;

int points=0;
int max_points = 10;
int victory = 0;
using glm::vec3;
int blinn = 0;
int light_on = 1;
int direction = 0;
float angle = 0.0;
auto startTime = getTimePoint();
auto currentTime = getTimePoint();
double deltaTime;
float rotationSpeed, rotationAngle;
float Time = 0.0;
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 gen(seed);
float posX, posZ, radius;
int window_width = 1280;
int window_height = 800;
int buffer_width = 800;
int buffer_height = 600;
GLuint SkyBox_Program;
GLuint postprocessing_texture_buffer;
GLuint postprocessing_program;
unsigned int screen_vao, screen_vbo;
int frame_switch = 1;
int post_effect = 0;
GLuint view_frame;
struct LightParam {
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    vec3 Attenuation;
    vec3 Position;
};
float screen_v[] = {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
};
struct LightParam myLight =
        {
                vec3(0.3, 0.3, 0.3),
                vec3(5.0, 5.0, 5.0),
                vec3(1.5, 1.5, 1.5),
                vec3(1.0, 0.0, 0.01),
                vec3(2.0, 3.0, 3.0)
        };
struct MaterialParam myMaterial[2] =
        {
                {
                        vec3(0.105882f, 0.058824f, 0.113725f),
                        vec3(0.427451f, 0.470588f, 0.541176f),
                        vec3(0.4, 0.4, 0.4),
                        9.84615f
                },
                {
                        vec3(0.2, 0.2, 0.2),
                        vec3(1.0, 1.0, 1.0),
                        vec3(1.0, 1.0, 1.0),
                        100.0
                }
        };




std::chrono::time_point<Clock> getTimePoint() {
    return Clock::now();
}

double calculateDeltaTime(const std::chrono::time_point<Clock> &start, const std::chrono::time_point<Clock> &end) {
    return std::chrono::duration<double>(end - start).count();
}

void sendLightParameters(LightParam light) {

    GLint programId;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programId);

    glUniform3fv(glGetUniformLocation(programId, "myLight.Ambient"), 1, glm::value_ptr(light.Ambient));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Diffuse"), 1, glm::value_ptr(light.Diffuse));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Specular"), 1, glm::value_ptr(light.Specular));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Attenuation"), 1, glm::value_ptr(light.Attenuation));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Position"), 1, glm::value_ptr(light.Position));
}





void DisplayScene() {
    currentTime = getTimePoint();
    deltaTime = calculateDeltaTime(startTime, currentTime);
    startTime = currentTime;
     rotationSpeed = 30.0f;
     rotationAngle = rotationSpeed * deltaTime;
    if (pause == 0)
        angle += 0.015f;
    radius = 3.0f;
    posX = player.Position.x + radius * cos(angle);
    posZ = player.Position.z + radius * sin(angle);
    write_to_minimap_fbo();
    write_to_postprocessing_fbo();
    default_fbo();
    if (post_effect != 0)
        draw_postprocessing();
    draw_minimap();
    myCamera.SendPV(default_program);
    glUseProgram(0);
    glutSwapBuffers();
}

void write_to_postprocessing_fbo() {
    glUseProgram(default_program);
    glBindFramebuffer(GL_FRAMEBUFFER, postprocessing_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);
    set_uniform();
    draw_objects();
}

void set_uniform() {
    myLight.Position = glm::vec3(posX, player.Position.y+8.0, posZ);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(default_program);
    myCamera.Update();
    glUniformMatrix4fv(glGetUniformLocation(default_program, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
    glUniformMatrix4fv(glGetUniformLocation(default_program, "matView"), 1, GL_FALSE, glm::value_ptr(myCamera.matView));
    glm::vec3 cameraPos = ExtractCameraPos(myCamera.matView);
    glUniform3fv(glGetUniformLocation(default_program, "cameraPos"), 1, &cameraPos[0]);
    glUniform1i(glGetUniformLocation(default_program, "blinn"), blinn);
    glUniform1i(glGetUniformLocation(default_program, "light_on"), light_on);
    glUniform1i(glGetUniformLocation(default_program, "direction"), direction);
    sendLightParameters(myLight);
}

void draw_objects() {
    ground.draw();
    mine.sendMaterialParameters();
    mine.draw_with_env_map(1, sb.SkyBox_Texture);
    mine.move(0.1, 'c');
    player.draw();
    ice.draw();
    candy.hover(rotationAngle);
    for (auto &collider: myColliders) {
        if (collider->hide == 0)
            collider->draw();
        if (collider->type == CSceneObject::object_type::take)
            collider->hover(rotationAngle);
    }
    for (auto &i: nonColliders) {
        i->draw();
    }
    sb.DrawSkyBox(matProj, myCamera.matView);
}

void draw_postprocessing() {
    glUseProgram(postprocessing_program);
    glUniform1i(glGetUniformLocation(postprocessing_program, "frame_switch"), frame_switch);
    glUniform1i(glGetUniformLocation(postprocessing_program, "effect"), post_effect);
    glUniform1i(glGetUniformLocation(postprocessing_program, "screenTexture"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postprocessing_texture_buffer);
    glUniform1i(glGetUniformLocation(postprocessing_program, "frame_texture"), 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, view_frame);
    glBindVertexArray(screen_vao);
    glDisable(GL_DEPTH_TEST);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void write_to_minimap_fbo() {
    glUseProgram(default_program);
    glBindFramebuffer(GL_FRAMEBUFFER, minimap_fbo);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, buffer_width, buffer_height);
    glUseProgram(default_program);
    set_uniform();
    myCamera.matView = glm::lookAt(glm::vec3(0.0f, 5.0f, 0.0f),
                          glm::vec3(0.0f, 0.0f, 0.0f),
                          glm::vec3(0.0f, 0.0f, 1.0f));


    myCamera.matView = glm::translate(myCamera.matView, glm::vec3(CameraTranslate_x, 0.0, CameraTranslate_z));
    myCamera.matView = glm::rotate(myCamera.matView, CameraRotate_y, glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(default_program, "matView"), 1, GL_FALSE, glm::value_ptr(myCamera.matView));
    draw_objects();
}

void default_fbo() {
    glUseProgram(default_program);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);
    set_uniform();
    draw_objects();

}

void draw_minimap() {
    if (minimap_switch == 1) {
        glUseProgram(minimap_program);
        glUniform1i(glGetUniformLocation(minimap_program, "screenTexture"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, minimap_texture_buffer);
        glUniform1i(glGetUniformLocation(minimap_program, "frame_texture"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, frame_texture);
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, 6);

    }
}

void generate_program_and_shaders(GLuint *id, const char *vertex_file, const char *fragment_file) {
    *id = glCreateProgram();
    glAttachShader(*id, LoadShader(GL_VERTEX_SHADER, vertex_file));
    glAttachShader(*id, LoadShader(GL_FRAGMENT_SHADER, fragment_file));
    LinkAndValidateProgram(*id);
}

void create_vao_from_vertices(float *vertices, unsigned int *vao, unsigned int *vbo, int size) {
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
}

void blurr() {
    post_effect = 3;
    if (blurr_duration == 0)
        blurr_duration += 300;
};

void handleBlurEffect() {
    if (blurr_duration > 0) {
        blurr_duration--;
    } else {
        post_effect = 0;
    }
}
void point(){
    points +=1;
    if (points == max_points){
        victory=1;
        exit(0);
    }
    printf("%d %d\n",points,victory);

}

void Animation(int frame) {
    Time += 0.01;
    glutPostRedisplay();
    glutTimerFunc(1000 / 60, Animation, 0);
    handleBlurEffect();
}


void generate_non_collision(Mesh &to_copy, int n) {
    std::uniform_real_distribution<float> dis(-30.0f, 30.0f);
    std::uniform_real_distribution<float> dis_rot(0.0f, 360.0f);
    for (int i = 0; i < n; i++) {
        Mesh *newObj = new Mesh(to_copy);
        float rot_angle = dis_rot(gen);
        float x = dis(gen);
        float z = dis(gen);
        float y = ground_col.getAltitute(glm::vec2(x, z));
        if (y != 0.0) {
            newObj->matModel = glm::translate(glm::mat4(1.0), glm::vec3(x, y, z));
            newObj->rotatey(rot_angle);
            nonColliders.push_back(newObj);
        }
        else
            delete newObj;
    }
}
void generate(const CSceneObject &to_copy, int n) {
    std::uniform_real_distribution<float> dis(-30.0f, 30.0f);
    std::uniform_real_distribution<float> dis_rot(0.0f, 360.0f);
    for (int i = 0; i < n; i++) {
        // Create a copy of the original object
        CSceneObject *newObj = new CSceneObject(to_copy);
        int collision_check = 0;
        int safety = 20;
        float rot_angle = dis_rot(gen);
        float x= dis(gen);
        float z= dis(gen);;
        float y = ground_col.getAltitute(glm::vec2(x, z));
        while (y == 0.0 && safety > 0) {
            safety -= 1;
            x = dis(gen);
            z = dis(gen);
            y = ground_col.getAltitute(glm::vec2(x, z));
        }
        if (safety>0){
        glm::vec3 newPos(x, y, z);
        newObj->SetPosition(newPos);
        newObj->rotatey(rot_angle);
        newObj->Collider = new CSphereCollider(&newObj->Position, to_copy.Collider->Radius);
        for (auto &c: myColliders) {
            if (newObj->Collider->isCollision(c->Collider)) {
                collision_check = 1;
                break;
            }
        }
        if (collision_check==0 && !player.Collider->isCollision(newObj->Collider)) {
            myColliders.push_back(newObj);
        }
        else
            delete newObj;
    }}
}

void Initialize() {
    startTime = getTimePoint();

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_NOTEQUAL, 0.0 );



    generate_program_and_shaders(&default_program, "shaders/default/vertex.glsl", "shaders/default/fragment.glsl");
    generate_program_and_shaders(&minimap_program, "shaders/minimap_fbo/vertex.glsl",
                                 "shaders/minimap_fbo/fragment.glsl");
    generate_program_and_shaders(&postprocessing_program, "shaders/postprocessing/vertex.glsl",
                                 "shaders/postprocessing/fragment.glsl");
    generate_program_and_shaders(&SkyBox_Program, "shaders/skybox/skybox_vertex.glsl",
                                 "shaders/skybox/skybox_fragment.glsl");
    glUseProgram(default_program);
    sb.SkyBox_Program = SkyBox_Program;
    sb.CreateSkyBox();
    stbi_set_flip_vertically_on_load(true);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    ground.load_vertices_from_file("objects/sround.obj");
    ground.material = myMaterial[0];
    ground.load_texture("textures/snow.png");
    ground.idProgram = default_program;

    ice.load_vertices_from_file("objects/ice.obj");
    ice.move(0.055, 'y');
    ice.material = myMaterial[0];
    ice.load_texture("textures/ice.jpg");
    ice.idProgram = default_program;


    kwadrat.material = myMaterial[1];
    kwadrat.load_texture("textures/metal.jpg");
    kwadrat.idProgram = default_program;


    player.load_vertices_from_file("objects/173.obj");
    player.scale(0.1);
    player.material = myMaterial[0];
    player.load_texture("textures/173texture.jpg");
    player.idProgram = default_program;
    player.Collider = new CSphereCollider(&player.Position, 0.5f);
    player.setBlurCallback(blurr);
    player.setPointCallback(point);
    myColliders.push_back(&rock);

    myCamera.Init(&player,glm::vec3(0.0f, 8.0f, 0.0f));
    ground_col.Init(ground.vertices);
    player.init2(&ground_col);


    rock.load_vertices_from_file("objects/rock.obj");
    rock.material = myMaterial[1];
    rock.load_texture("textures/rock.jpeg");
    rock.SetPosition(glm::vec3(-5.0, 0.0, 2.0));
    rock.Collider = new CSphereCollider(&rock.Position, 0.3f);
    rock.idProgram = default_program;
    rock.scale(0.1);
    rock.type = CSceneObject::object_type::stop;

    tree.load_vertices_from_file("objects/Tree.obj");
    tree.material = myMaterial[1];
    tree.load_texture("textures/tree_texture.png");
    tree.SetPosition(glm::vec3(-15.0, 0.0, 2.0));
    tree.Collider = new CSphereCollider(&tree.Position, 1.0f);
    tree.idProgram = default_program;
    tree.scale(0.05);
    tree.type = CSceneObject::object_type::stop;


    mine.load_vertices_from_file("objects/mine.obj");
    mine.material = myMaterial[1];
    mine.load_texture("textures/mine_texture.jpeg");
    mine.SetPosition(glm::vec3(-15.0, 0.0, 10.0));
    mine.Collider = new CSphereCollider(&mine.Position, 1.0f);
    mine.idProgram = default_program;
    mine.scale(0.1);
    mine.type = CSceneObject::object_type::annoy;

    candy.load_vertices_from_file("objects/coin.obj");
    candy.material = myMaterial[1];
    candy.load_texture("textures/cointexture.jpg");
    candy.SetPosition(glm::vec3(-10.0, 0.0, 2.0));
    candy.Collider = new CSphereCollider(&candy.Position, 2.0f);
    candy.idProgram = default_program;
    candy.type = CSceneObject::object_type::take;

    grass.load_vertices_from_file("objects/flower.obj");

    grass.material = myMaterial[1];
    grass.load_texture("textures/flower_texture.png");
    grass.idProgram = default_program;
    grass.matModel = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
    grass.scale(2.0);



    generate(candy, max_points);
    generate(tree, 10);
    generate(rock, 20);
    generate(mine, 15);
    generate_non_collision(grass,30);


    for (auto &c: myColliders) {
        if (c->type == CSceneObject::object_type::take) {
            c->vertical_lean();
        }
    }
    candy.hide=1;



    create_vao_from_vertices(screen_v, &screen_vao, &screen_vbo, sizeof(screen_v) / sizeof(screen_v[0]));

    glutTimerFunc(0, Animation, 0);
    glGenerateMipmap(GL_TEXTURE_2D);
    load_texture(&frame_texture, "textures/minimap_frame.png");
    load_texture(&view_frame, "textures/view_frame.png");
    glGenFramebuffers(1, &minimap_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, minimap_fbo);
    glGenTextures(1, &minimap_texture_buffer);
    glBindTexture(GL_TEXTURE_2D, minimap_texture_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buffer_width, buffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           minimap_texture_buffer, 0);


    glGenRenderbuffers(1, &minimap_depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, minimap_depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, buffer_width, buffer_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, minimap_depth_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: Framebuffer is not complete!\n");
        exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glGenFramebuffers(1, &postprocessing_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, postprocessing_fbo);
    glGenTextures(1, &postprocessing_texture_buffer);
    glBindTexture(GL_TEXTURE_2D, postprocessing_texture_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           postprocessing_texture_buffer, 0);


    glGenRenderbuffers(1, &postprocessing_depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, postprocessing_depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, window_width, window_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, postprocessing_depth_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: Framebuffer is not complete!\n");
        exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);


}


void Reshape(int width, int height) {
    window_width = width;
    window_height = height;
    matProj = glm::perspectiveFov(glm::radians(60.0f), (float) width, (float) height, 0.1f, 100.f);
    myCamera.UpdatePerspective(window_width,window_height);
}


void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
            glutLeaveMainLoop();
            break;
        case 'a':
            player.rotate2(0.1);
            break;

        case 'd':
            player.rotate2(-0.1);
            break;

        case 'w':
            if (blurr_duration == 0)
                player.move2(0.1, myColliders);
            else {
                player.move2(0.03, myColliders);
                player.rotate2(0.01);
            }
            break;

        case 's':
            if (blurr_duration == 0)
                player.move2(-0.1, myColliders);
            else {
                player.move2(-0.03, myColliders);
                player.rotate2(0.01);
            }
            break;

        case 'q':
            CameraRotate_y -= 0.1f;
            break;
        case 'e':
            CameraRotate_y += 0.1f;
            break;


    }
    glutPostRedisplay();
}


void load_texture(GLuint *tex, const std::string &name) {
    glGenTextures(1, tex);
    int tex_width, tex_height, tex_n;
    unsigned char *tex_data = stbi_load(name.c_str(), &tex_width, &tex_height, &tex_n, 0);

    if (tex_data == nullptr) {
        printf("Image '%s' can't be loaded!\n", name.c_str());
    } else {
        GLenum format = tex_n == 4 ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, *tex);

        glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, tex_data);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


        stbi_image_free(tex_data);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Menu(int value) {
    switch (value) {
        case 1:
            pause = (pause == 1) ? 0 : 1;
            break;
        case 2:
            light_on = (light_on == 1) ? 0 : 1;
            break;
        case 3:
            blinn = (blinn == 1) ? 0 : 1;
            break;
        case 4:
            direction = (direction == 1) ? 0 : 1;
            break;
        case 5:
            minimap_switch = (minimap_switch == 1) ? 0 : 1;
            break;
        case 60:
            post_effect = 1;
            break;
        case 61:
            post_effect = 2;
            break;
        case 62:
            post_effect = 3;
            break;
        case 63:
            post_effect = 0;
            break;
        case 7:
            frame_switch = (frame_switch == 1) ? 0 : 1;
            break;
        default:
            printf("Wybrano %d \n", value);
    }
}


int main(int argc, char *argv[]) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow("Programownie grafiki 3D w OpenGL");

    glutDisplayFunc(DisplayScene);
    glutReshapeFunc(Reshape);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutMouseWheelFunc(MouseWheel);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeys);


    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        printf("GLEW Error\n");
        exit(1);
    }


    if (!GLEW_VERSION_3_2) {
        printf("Brak OpenGL 3.2!\n");
        exit(1);
    }
    int postprocessing_menu = glutCreateMenu(Menu);
    glutAddMenuEntry("Inverse", 60);
    glutAddMenuEntry("Sharpen", 61);
    glutAddMenuEntry("Blur", 62);
    glutAddMenuEntry("Off", 63);


    glutCreateMenu(Menu);
    glutAddMenuEntry("Pause/Unpause Animation", 1);
    glutAddMenuEntry("Turn Light On/Off", 2);
    glutAddMenuEntry("Blinn On/Off", 3);
    glutAddMenuEntry("Directional Light On/Off", 4);
    glutAddMenuEntry("Minimap On/Off", 5);
    glutAddSubMenu("Postprocessing", postprocessing_menu);
    glutAddMenuEntry("Frame On/Off", 7);


    glutAttachMenu(GLUT_RIGHT_BUTTON);

    Initialize();

    glutTimerFunc(1000 / 60, Animation, 0);
    glutMainLoop();

    for (auto &collider: myColliders) {
        delete collider->Collider;
        delete collider;
    }
    myColliders.clear();
    delete tree.Collider;
    delete player.Collider;
    delete rock.Collider;
    return 0;
}
