#ifndef __CPLAYER
#define __CPLAYER


#include <glm/glm.hpp>
#include <GL/glew.h>
#include "ground.hpp"
#include "mesh/Mesh.hpp"
#include "collider.hpp"
#include "CSceneObject.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

extern void blurr();

using BlurCallback = void (*)();
using PointCallback = void(*)();
class CPlayer : public Mesh {

public:
    BlurCallback blurCallback = nullptr;
    PointCallback pointCallback = nullptr;
    void setBlurCallback(BlurCallback callback) {
        blurCallback = callback;
    }
    void setPointCallback(PointCallback callback) {
        pointCallback = callback;
    }

    glm::vec3 Position = glm::vec3(0.0, 0.0, 3.0);
    glm::vec3 test_pos = Position;
    // kat orientacji obiektu
    float Angle = 0.0;


    glm::vec3 Direction = glm::vec3(1.0, 0.0, 0.0);


    CGround *myGround = NULL;
    CSphereCollider *Collider = NULL;





    CPlayer() {}

    void init2(CGround *ground) {
        myGround = ground;

        Position.y = myGround->getAltitute(glm::vec2(Position.x, Position.z));
        update2();

    }

    void update2() {
        matModel = glm::translate(glm::mat4(1.0), Position);
        matModel = glm::rotate(matModel, Angle - 30.0f, glm::vec3(0.0, 1.0, 0.0));

    }

    void move2(float val, std::vector<CSceneObject*> &colliders) {
        bool collision_check = false;
        test_pos = Position;
        test_pos += Direction * val;
        test_pos.y = myGround->getAltitute(glm::vec2(test_pos.x, test_pos.z));
        this->Collider->Position = &test_pos;
        for (auto &collider: colliders) {
            if (this->Collider->isCollision(collider->Collider)) {
                if (collider->type == CSceneObject::object_type::stop)
                    collision_check = true;
                else if (collider->type == CSceneObject::object_type::annoy) {
                    if (collider->hide==0)
                        blurCallback();
                    collider->hide = 1;
                }
                else {
                    if (collider->hide==0)
                        pointCallback();
                    collider->hide_lol();
                }
            }
        }
        if (!collision_check && test_pos.y != 0.0) {
            Position = test_pos;
            update2();
        } else {
//            if (val < 0)
//                Position += Direction * val;
//            update2();


        }


//        printf("pos: (%f, %f, %f)\n", Position.x, Position.y, Position.z);
//        printf("test: (%f, %f, %f)\n", test_pos.x, test_pos.y, test_pos.z);


    }

//    void move2(float val) {
//        test_pos = Position;
//        test_pos += Direction * val;
//        test_pos.y = myGround->getAltitute(glm::vec2(test_pos.x, test_pos.z));
//        if (test_pos.y != 0.0) {
//            Position = test_pos;
//        }
//        // aktualizacja
//        update2();
//    }

    // zmiana orientacji obiektu
    void rotate2(float angle) {
        Angle += angle;
        Direction.x = cos(Angle);
        Direction.z = -sin(Angle);

        // aktualizacja
        update2();
    }

};


#endif
