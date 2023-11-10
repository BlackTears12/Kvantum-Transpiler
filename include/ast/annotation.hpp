#pragma once
#include <string>

using std::string;
using std::array;
namespace kvantum
{
    class Annotation
    {
    public:
        enum Type
        {
            Native,ANNOTATION_NUMBER
        };

        static bool isValid(const string &id) { return id == "@native"; }

        static Annotation* getAnnotation(const string &id)
        {
            if (id == "@native")
                return &annotations[Native];
            throw std::invalid_argument(id);
        }

        Type getType() const { return type; }
    private:
        static array<Annotation,ANNOTATION_NUMBER> annotations;
        explicit Annotation(Annotation::Type t) : type(t) {}
        Type type;
    };
}