// func2.c
#include <math.h>
#include "libfunc.h"

// Реализация 2 для Derivative: Центральная разностная производная (точнее)
float Derivative(float A, float deltaX) {
    return (cosf(A + deltaX) - cosf(A - deltaX)) / (2 * deltaX);
}

// Реализация 2 для E: Сумма ряда Тейлора для e (e = Σ(1/n!))
float E(int x) {
    float sum = 1.0f; // 0! = 1
    float factorial = 1.0f;
    for (int n = 1; n <= x; ++n) {
        factorial *= n; // Вычисляем n! итеративно
        sum += 1.0f / factorial;
    }
    return sum;
}