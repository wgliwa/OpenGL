#ifndef UNTITLED4_MESH_HPP
#define UNTITLED4_MESH_HPP


#include <stdexcept>
#include <stb_image.h>
#include "../../utils/obj_loader.hpp"

struct MaterialParam {
    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;
    float Shininess;
};

class Mesh {
public:
    GLuint idProgram;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    unsigned int idVAO, idVBO_uv, idVBO_normal, idVBO_coord;
    MaterialParam material;
    unsigned int texture;
    glm::mat3 matNormal;
    glm::mat4 matModel;
    float totalRotation = 0.0f;

    Mesh(){
        matModel=glm::mat4(1.0);
    }

    Mesh(glm::vec3& init_pos) {
        matModel = glm::mat4(1.0); //todo
    }
//Mesh(const Mesh &other) {
//    // Copy vertices, uvs, normals, and material parameters
//    vertices = other.vertices;
//    uvs = other.uvs;
//    normals = other.normals;
//    material = other.material;
//
//    // Generate and bind new OpenGL VAO
//    glGenVertexArrays(1, &idVAO);
//    glBindVertexArray(idVAO);
//
//    // Copy vertex buffer data
//    glGenBuffers(1, &idVBO_coord);
//    glBindBuffer(GL_ARRAY_BUFFER, idVBO_coord);
//    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
//    glGenBuffers(1, &idVBO_uv);
//    glBindBuffer(GL_ARRAY_BUFFER, idVBO_uv);
//    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
//    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
//    glEnableVertexAttribArray(2);
//
//    glGenBuffers(1, &idVBO_normal);
//    glBindBuffer(GL_ARRAY_BUFFER, idVBO_normal);
//    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
//    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
//    glEnableVertexAttribArray(1);
//
//    glBindVertexArray(0);
//}
    void load_vertices_from_file(const std::string &name) {
        if (!loadOBJ(name.c_str(), vertices, uvs, normals))
            throw std::invalid_argument("wrong path");

        glGenVertexArrays(1, &idVAO);
        glBindVertexArray(idVAO);

        glGenBuffers(1, &idVBO_coord);
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_coord);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &idVBO_uv);
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_uv);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);

        glGenBuffers(1, &idVBO_normal);
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_normal);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);


        glBindVertexArray(0);
    }
    void prep_basic(){
        glUseProgram(idProgram);
        sendMaterialParameters();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(idProgram, "fragment_texture"), 0);
        glUniformMatrix4fv(glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));
        matNormal = glm::mat3(glm::transpose(glm::inverse(matModel)));
        glUniformMatrix3fv(glGetUniformLocation(idProgram, "matNormal"), 1, GL_FALSE, glm::value_ptr(matNormal));
        glUniform1i(glGetUniformLocation(idProgram, "env_map"), 0);
    }

    void draw() {
        prep_basic();
        glBindVertexArray(idVAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    void draw_with_env_map(int mode, GLuint skybox_texture){
        prep_basic();
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        glUniform1i(glGetUniformLocation(skybox_texture, "tex_skybox"), 1);
        if(mode==1 || mode==2){
            glUniform1i(glGetUniformLocation(idProgram, "env_map"), mode);
        }else
            throw std::invalid_argument("1 or 2 bozo");
        draw();

    }
    void rotatey(float angle){
        matModel = glm::rotate(matModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    void vertical_lean(){
        matModel = glm::rotate(matModel, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    void hover(float angle) {
        matModel = glm::rotate(matModel, glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        matModel = glm::rotate(matModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        matModel = glm::rotate(matModel, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    void move(float amount, char axis) {
        glm::vec3 translationAmount(0.0f);
        switch (axis) {
            case 'x':
                translationAmount.x = amount;
                break;
            case 'y':
                translationAmount.y = amount;
                break;
            case 'z':
                translationAmount.z = amount;
                break;
            default:
                // Handle invalid axis
                return;
        }
        matModel = glm::translate(matModel, translationAmount);
    }
    void circle_around_point(const glm::vec3& orbitCenter, float orbitRadius, float orbitSpeed, float deltaTime) {//todo
        float currentAngle = orbitSpeed * deltaTime;
        float xPos = orbitCenter.x + orbitRadius * cos(currentAngle);
        float yPos = orbitCenter.y;
        float zPos = orbitCenter.z + orbitRadius * sin(currentAngle);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, yPos, zPos));
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), currentAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        matModel = translationMatrix * rotationMatrix;
    }

    void sendMaterialParameters() {
        glUniform3fv(glGetUniformLocation(idProgram, "myMaterial.Ambient"), 1, glm::value_ptr(material.Ambient));
        glUniform3fv(glGetUniformLocation(idProgram, "myMaterial.Diffuse"), 1, glm::value_ptr(material.Diffuse));
        glUniform3fv(glGetUniformLocation(idProgram, "myMaterial.Specular"), 1,
                     glm::value_ptr(material.Specular));
        glUniform1f(glGetUniformLocation(idProgram, "myMaterial.Shininess"), material.Shininess);
    }

    void load_texture(const std::string &name) {
        glGenTextures(1, &texture);
        int tex_width, tex_height, tex_n;
        unsigned char *tex_data = stbi_load(name.c_str(), &tex_width, &tex_height, &tex_n, 0);

        if (tex_data == nullptr) {
            printf("Image '%s' can't be loaded!\n", name.c_str());
        } else {
            GLenum format = tex_n == 4 ? GL_RGBA : GL_RGB;
            glBindTexture(GL_TEXTURE_2D, texture);

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


    void scale(float scaleFactor) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i] *= scaleFactor;
        }
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_coord);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    }
};


#endif //UNTITLED4_MESH_HPP
