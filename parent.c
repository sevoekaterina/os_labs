#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

// Структура для разделяемой памяти
typedef struct {
    int count;
    int numbers[1000];
    bool finished;
} shared_data;

int main() {
    char filename[100];
    pid_t pid;
    
    // Получение имени файла от пользователя
    printf("Введите имя файла: ");
    fflush(stdout);
    
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        perror("Ошибка чтения имени файла");
        exit(1);
    }
    
    // Удаление символа новой строки из имени файла
    filename[strcspn(filename, "\n")] = 0;
    
    // Проверяем существование файла
    if (access(filename, F_OK) == -1) {
        printf("Ошибка: файл '%s' не существует\n", filename);
        exit(1);
    }
    
    if (access(filename, R_OK) == -1) {
        printf("Ошибка: нет прав на чтение файла '%s'\n", filename);
        exit(1);
    }
    
    // Создаем уникальное имя для разделяемой памяти
    char shm_name[100];
    snprintf(shm_name, sizeof(shm_name), "/prime_check_%d", getpid());
    printf("Имя разделяемой памяти: %s\n", shm_name);
    
    // Создаем и настраиваем разделяемую память
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Ошибка создания разделяемой памяти");
        exit(1);
    }
    
    // Устанавливаем размер разделяемой памяти
    if (ftruncate(shm_fd, sizeof(shared_data)) == -1) {
        perror("Ошибка установки размера памяти");
        close(shm_fd);
        shm_unlink(shm_name);
        exit(1);
    }
    
    // Отображаем разделяемую память
    shared_data *shared = mmap(NULL, sizeof(shared_data), 
                               PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("Ошибка отображения памяти");
        close(shm_fd);
        shm_unlink(shm_name);
        exit(1);
    }
    
    close(shm_fd);
    
    // Инициализируем структуру
    shared->count = 0;
    shared->finished = false;
    
    printf("Создание дочернего процесса...\n");
    
    // Создание дочернего процесса
    pid = fork();
    
    if (pid == -1) {
        perror("Ошибка создания процесса");
        munmap(shared, sizeof(shared_data));
        shm_unlink(shm_name);
        exit(1);
    }
    
    if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс запущен (PID: %d)\n", getpid());
        
        // Открываем файл
        int file_fd = open(filename, O_RDONLY);
        if (file_fd == -1) {
            perror("Ошибка открытия файла в дочернем процессе");
            exit(1);
        }
        
        // Перенаправляем стандартный ввод на файл
        if (dup2(file_fd, STDIN_FILENO) == -1) {
            perror("Ошибка перенаправления ввода");
            close(file_fd);
            exit(1);
        }
        
        close(file_fd); // Файловый дескриптор больше не нужен
        
        // Запускаем дочернюю программу с ТОЛЬКО одним аргументом - именем разделяемой памяти
        execl("./child", "child", shm_name, NULL);
        
        // Если execl вернул управление, значит произошла ошибка
        perror("Ошибка запуска дочерней программы");
        exit(1);
        
    } else {
        // Родительский процесс
        printf("Дочерний процесс создан с PID: %d\n", pid);
        printf("Ожидание завершения дочернего процесса...\n");
        
        int status;
        int timeout = 10; // 10 секунд таймаут
        int waited = 0;
        
        // Ждем завершения дочернего процесса с таймаутом
        while (1) {
            pid_t result = waitpid(pid, &status, WNOHANG);
            
            if (result == -1) {
                perror("Ошибка в waitpid");
                break;
            } else if (result == 0) {
                // Дочерний процесс еще работает
                if (waited >= timeout) {
                    printf("Таймаут! Завершаем дочерний процесс...\n");
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0);
                    break;
                }
                
                printf("Ожидание... (прошло %d сек.)\n", waited + 1);
                sleep(1);
                waited++;
                
                // Проверяем флаг завершения в разделяемой памяти
                if (shared->finished) {
                    printf("Дочерний процесс установил флаг завершения\n");
                    // Даем дочернему процессу время на завершение
                    sleep(1);
                    continue;
                }
            } else {
                // Дочерний процесс завершился
                break;
            }
        }
        
        // Проверяем, завершился ли дочерний процесс
        if (WIFEXITED(status)) {
            printf("Дочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Дочерний процесс завершился по сигналу: %d\n", WTERMSIG(status));
        }
        
        printf("Результат (составные числа):\n");
        
        // Читаем данные из разделяемой памяти
        if (shared->count > 0) {
            for (int i = 0; i < shared->count; i++) {
                printf("%d\n", shared->numbers[i]);
            }
            printf("Всего найдено %d составных чисел\n", shared->count);
        } else {
            printf("Составные числа не найдены\n");
        }
        
        // Освобождаем разделяемую память
        munmap(shared, sizeof(shared_data));
        
        // Удаляем объект разделяемой памяти
        if (shm_unlink(shm_name) == -1) {
            perror("Ошибка удаления разделяемой памяти");
        }
        
        printf("Обработка завершена.\n");
    }
    
    return 0;
}