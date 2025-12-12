// libfunc.h
#ifndef LIBFUNC_H
#define LIBFUNC_H

// Функция 1: Производная cos(x) в точке A
// Возвращает значение производной (f'(A))
float Derivative(float A, float deltaX);

// Функция 2: Вычисление приближения числа e
// x - параметр точности (для метода 1: (1+1/x)^x; для метода 2: верхний предел суммы ряда)
float E(int x);

#endif