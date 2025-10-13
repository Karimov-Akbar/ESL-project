typedef struct {
    float x;
    float y;
    float z;
} Vector3D;

Vector3D vector_sum(Vector3D v1, Vector3D v2);
Vector3D vector_sub(Vector3D v1, Vector3D v2);
float vector_dot(Vector3D v1, Vector3D v2);
Vector3D vector_cross(Vector3D v1, Vector3D v2);