#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>

// Структура для разделяемой памяти
typedef struct {
    int count;
    int numbers[1000];
    bool finished;
} shared_data;

// Функция проверки числа на простоту
bool is_prime(int num) {
    if (num <= 1) return false;
    if (num <= 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    
    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <shared_memory_name>\n", argv[0]);
        exit(1);
    }
    
    char *shm_name = argv[1];
    
    // Открываем разделяемую память
    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open in child");
        exit(1);
    }
    
    // Отображаем разделяемую память
    shared_data *shared = mmap(NULL, sizeof(shared_data), 
                               PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap in child");
        close(shm_fd);
        exit(1);
    }
    
    close(shm_fd);
    
    char buffer[256];
    
    // Чтение чисел из стандартного ввода (который перенаправлен в файл)
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        int number;
        
        if (sscanf(buffer, "%d", &number) == 1) {
            // Проверка на отрицательное число
            if (number < 0) {
                shared->finished = true;
                munmap(shared, sizeof(shared_data));
                exit(0);
            }
            
            // Проверка на простое число
            if (is_prime(number)) {
                shared->finished = true;
                munmap(shared, sizeof(shared_data));
                exit(0);
            } else {
                // Записываем составное число в разделяемую память
                if (shared->count < 1000) {
                    shared->numbers[shared->count] = number;
                    shared->count++;
                }
            }
        }
    }
    
    shared->finished = true;
    munmap(shared, sizeof(shared_data));
    return 0;
}