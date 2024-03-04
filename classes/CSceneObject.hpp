#include "mesh/Mesh.hpp"
#include "collider.hpp"

#ifndef UNTITLED4_CSCENEOBJECT_HPP
#define UNTITLED4_CSCENEOBJECT_HPP
class CSceneObject : public Mesh
{
public:
    enum class object_type{
        stop,
        take,
        annoy,
    };
//    Mesh mesh;
    // Dodatkowe skladowe wzgledem CMesh

    // ------------------------------------------
    // NOWE: Collider do testu kolizji
    // ------------------------------------------
    CSphereCollider *Collider = NULL; //nice
    glm::vec3 Position;
    CSceneObject::object_type type = CSceneObject::object_type::stop;
    int hide = 0;


    CSceneObject() : Mesh(), Collider(nullptr), Position(glm::vec3(0.0f)), type(object_type::stop), hide(0) {}
    CSceneObject(const CSceneObject &other)
            : Mesh(other), Collider(nullptr), Position(other.Position), type(other.type), hide(other.hide) {
        if (other.Collider) {
            if (dynamic_cast<CSphereCollider*>(other.Collider)) {
                this->Collider = new CSphereCollider(*(dynamic_cast<CSphereCollider*>(other.Collider)));
            }
        }
    }
    void hide_lol(){
        hide = 1;
    }
    // ------------------------------------------
    // NOWE: Ustawienie pozycji obiektu na scenie
    // ------------------------------------------
    void SetPosition(glm::vec3 _pos)
    {
        this->Position = _pos;
        matModel = glm::translate(glm::mat4(1.0), this->Position);
    }


};
#endif //UNTITLED4_CSCENEOBJECT_HPP
