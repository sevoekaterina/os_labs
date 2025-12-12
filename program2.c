// program2.c
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>   // Для dlopen, dlsym, dlclose
#include "libfunc.h" // Для объявления типов функций

int main() {
    void *handle = NULL;
    float (*Derivative)(float, float) = NULL;
    float (*E)(int) = NULL;

    // Загружаем первую библиотеку по умолчанию
    handle = dlopen("./libfunc1.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Ошибка загрузки libfunc1.so: %s\n", dlerror());
        return 1;
    }

    // Загружаем символы (функции) из библиотеки
    Derivative = (float (*)(float, float)) dlsym(handle, "Derivative");
    E = (float (*)(int)) dlsym(handle, "E");
    if (!Derivative || !E) {
        fprintf(stderr, "Ошибка загрузки функций: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    int current_lib = 1; // Текущая загруженная библиотека (1 или 2)
    int command;
    float A, deltaX;
    int x;

    printf("Программа 2 (динамическая загрузка библиотек)\n");
    printf("Доступные команды:\n");
    printf("  0           - Переключить реализацию (libfunc1.so <-> libfunc2.so)\n");
    printf("  1 A deltaX  - Вычислить производную cos в точке A\n");
    printf("  2 x         - Вычислить приближение e\n");
    printf("  Любой другой ввод - выход\n");

    while (1) {
        printf("[lib%d]> ", current_lib);
        if (scanf("%d", &command) != 1) break;

        switch (command) {
            case 0: {
                // Переключение библиотеки
                dlclose(handle);
                if (current_lib == 1) {
                    handle = dlopen("./libfunc2.so", RTLD_LAZY);
                    current_lib = 2;
                } else {
                    handle = dlopen("./libfunc1.so", RTLD_LAZY);
                    current_lib = 1;
                }
                if (!handle) {
                    fprintf(stderr, "Ошибка загрузки библиотеки: %s\n", dlerror());
                    return 1;
                }
                // Повторно загружаем указатели на функции
                Derivative = (float (*)(float, float)) dlsym(handle, "Derivative");
                E = (float (*)(int)) dlsym(handle, "E");
                printf("  Переключено на libfunc%d.so\n", current_lib);
                break;
            }
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
                dlclose(handle);
                return 0;
        }
        while (getchar() != '\n');
    }
    dlclose(handle);
    return 0;
}