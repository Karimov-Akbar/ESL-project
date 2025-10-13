#include <stdio.h>
#include "vector3d.h"

int main() {
    Vector3D a = {1.0, 2.0, 3.0};
    Vector3D b = {4.0, 5.0, 6.0};

    // Пример использования функции:
    Vector3D sum = vector_sum(a, b);
    printf("Sum: (%.2f, %.2f, %.2f)\n", sum.x, sum.y, sum.z);

    // Добавьте вызовы sub, dot, cross
    return 0;
}