#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Для работы со строками
#include "maze.h"

// Сохраняет лабиринт в текстовый файл
// maze - указатель на лабиринт для сохранения
// filename - имя файла (путь к файлу)
// Возвращает 1 при успешном сохранении, 0 при ошибке
int save_maze_to_file(const Maze* maze, const char* filename) {
    if (!maze || !filename) return 0;  // Проверка входных параметров

    FILE* file;
    // Открываем файл для записи в текстовом режиме
    // Используем безопасную версию fopen
    if (fopen_s(&file, filename, "w") != 0) {
        return 0;  // Не удалось открыть файл
    }

    // Записываем размеры лабиринта в первую строку файла
    fprintf(file, "%d %d\n", maze->rows, maze->cols);

    // Записываем матрицу лабиринта
    for (int i = 0; i < maze->rows; i++) {
        for (int j = 0; j < maze->cols; j++) {
            fprintf(file, "%d", maze->cells[i][j]);  // Записываем значение клетки

            // Разделяем числа пробелами (кроме последнего в строке)
            if (j < maze->cols - 1) {
                fprintf(file, " ");
            }
        }
        fprintf(file, "\n");  // Переход на новую строку после каждой строки лабиринта
    }

    // Если есть статистика от последнего алгоритма, записываем ее как комментарий
    if (maze->last_stats) {
        fprintf(file, "# %s\n", maze->last_stats);  //# добавляет комментарий в файл
    }

    fclose(file);  // Закрываем файл
    return 1;      // Успешное сохранение
}

// Загружает лабиринт из текстового файла
// filename - имя файла для загрузки
// Возвращает указатель на загруженный лабиринт или NULL при ошибке
Maze* load_maze_from_file(const char* filename) {
    if (!filename) return NULL;  // Проверка входного параметра

    FILE* file;
    // Открываем файл для чтения в текстовом режиме
    if (fopen_s(&file, filename, "r") != 0) {
        return NULL;  // Не удалось открыть файл
    }

    int rows, cols;
    // Считываем размеры лабиринта из первой строки файла
    if (fscanf_s(file, "%d %d", &rows, &cols) != 2 || rows <= 0 || cols <= 0) {
        fclose(file);  // Закрываем файл
        return NULL;   // Неверный формат размеров
    }

    // Создаем лабиринт заданного размера
    Maze* maze = create_maze(rows, cols);
    if (!maze) {  // Если не удалось создать лабиринт
        fclose(file);
        return NULL;
    }

    // Считываем матрицу лабиринта из файла
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Считываем значение клетки
            if (fscanf_s(file, "%d", &maze->cells[i][j]) != 1) {
                free_maze(maze);  // Освобождаем частично загруженный лабиринт
                fclose(file);     // Закрываем файл
                return NULL;      // Ошибка чтения данных
            }

            // Определяем координаты старта и выхода по их значениям
            if (maze->cells[i][j] == 2) {  // Значение 2 - старт
                maze->start_row = i;
                maze->start_col = j;
            }
            else if (maze->cells[i][j] == 3) {  // Значение 3 - выход
                maze->exit_row = i;
                maze->exit_col = j;
            }
        }
    }

    // Читаем возможную строку со статистикой из файла
    char buffer[256];
    // fgets читает строку из файла (максимум 255 символов + нулевой)
    if (fgets(buffer, sizeof(buffer), file)) {
        // Удаляем символ новой строки из конца строки
        buffer[strcspn(buffer, "\n")] = 0;

        // Проверяем, начинается ли строка с комментария (#)
        if (strlen(buffer) > 0 && buffer[0] == '#') {
            // Пропускаем символ # и начальные пробелы
            char* stats_start = buffer + 1;
            while (*stats_start == ' ') stats_start++;

            // Если после # есть текст, сохраняем его как статистику
            if (strlen(stats_start) > 0) {
                // Выделяем память для строки статистики
                maze->last_stats = (char*)malloc(strlen(stats_start) + 1);
                // Копируем строку с использованием безопасной функции
                strcpy_s(maze->last_stats, strlen(stats_start) + 1, stats_start);
            }
        }
    }

    fclose(file);  // Закрываем файл
    return maze;   // Возвращаем загруженный лабиринт
}