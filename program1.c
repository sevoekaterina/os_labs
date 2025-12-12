// program1.c
#include <stdio.h>
#include <stdlib.h>
#include "libfunc.h" // Используем контракт

int main() {
    int command;
    float A, deltaX;
    int x;

    printf("Программа 1 (статическая линковка с libfunc1.so)\n");
    printf("Доступные команды:\n");
    printf("  1 A deltaX  - Вычислить производную cos в точке A\n");
    printf("  2 x         - Вычислить приближение e\n");
    printf("  Любой другой ввод - выход\n");

    while (1) {
        printf("> ");
        if (scanf("%d", &command) != 1) break;

        switch (command) {
            case 1:
                if (scanf("%f %f", &A, &deltaX) == 2) {
                    printf("  Derivative(%.2f, %.4f) = %.6f\n", A, deltaX, Derivative(A, deltaX));
                }
                break;
            case 2:
                if (scanf("%d", &x) == 1) {
                    if (x > 0) {
                        printf("  E(%d) = %.12f\n", x, E(x));
                    } else {
                        printf("  Ошибка: x должен быть > 0\n");
                    }
                }
                break;
            default:
                printf("Выход...\n");
                return 0;
        }
        // Очистка буфера ввода
        while (getchar() != '\n');
    }
    return 0;
}