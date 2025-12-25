#include <stdio.h>
#include <time.h>   // Для измерения времени выполнения
#include "maze.h"

// Рекурсивная вспомогательная функция для алгоритма DFS
// Выполняет поиск в глубину от текущей клетки (row, col)
// maze - указатель на лабиринт
// row, col - текущие координаты
// step - указатель на счетчик шагов (для нумерации посещенных клеток)
// start_time - время начала поиска (для вычисления общего времени)
// time_us - указатель для сохранения времени выполнения
// steps - указатель для сохранения общего количества шагов
// Возвращает 1 если путь найден, 0 если нет
static int dfs_recursive(Maze* maze, int row, int col, int* step, clock_t start_time,
    unsigned long long* time_us, int* steps) {
    // Проверка границ лабиринта
    if (row < 0 || row >= maze->rows || col < 0 || col >= maze->cols) {
        return 0;  // Выход за границы - путь не найден
    }

    int cell = maze->cells[row][col];  // Получаем значение текущей клетки

    // Если это стена (1) или уже посещали эту клетку в этом пути (>=4)
    if (cell == 1 || cell >= 4) {
        return 0;  // Не можем идти через стену или уже посещенную клетку
    }

    // Если нашли выход (значение 3)
    if (cell == 3) {
        // Фиксируем время окончания поиска и вычисляем общее время
        clock_t end_time = clock();
        *time_us = (unsigned long long)((end_time - start_time) * 1000000 / CLOCKS_PER_SEC);
        return 1;  // Путь найден!
    }

    // Отмечаем текущую клетку как посещенную (кроме старта, который имеет значение 2)
    if (cell == 0) {  // Только проходы отмечаем цифрами
        // Значения 4+ используются для отметки посещенных клеток с нумерацией 0-9
        maze->cells[row][col] = 4 + (*step % 10);
        (*step)++;     // Увеличиваем счетчик шагов
        (*steps)++;    // Увеличиваем общий счетчик шагов
    }
    // Старт (2) не трогаем - оставляем как 'S' для отображения

    // Пытаемся идти в соседние клетки в определенном порядке:
    // 1. Вправо (col + 1) - приоритет движения вправо
    // 2. Вниз (row + 1)   - затем вниз
    // 3. Влево (col - 1)  - затем влево
    // 4. Вверх (row - 1)  - затем вверх
    // Этот порядок создает характерное поведение DFS: идет в одном направлении до упора
    if (dfs_recursive(maze, row, col + 1, step, start_time, time_us, steps)) return 1;
    if (dfs_recursive(maze, row + 1, col, step, start_time, time_us, steps)) return 1;
    if (dfs_recursive(maze, row, col - 1, step, start_time, time_us, steps)) return 1;
    if (dfs_recursive(maze, row - 1, col, step, start_time, time_us, steps)) return 1;

    // Если из этой клетки никуда нельзя пойти (тупик), возвращаемся назад
    // Не стираем отметку (цифру), чтобы видеть все посещенные клетки в итоговом выводе
    return 0;
}

// Основная функция поиска пути алгоритмом DFS
// maze - указатель на лабиринт для поиска
// time_us - указатель для возврата времени выполнения (в микросекундах)
// steps - указатель для возврата количества шагов
// Возвращает 1 если путь найден, 0 если нет
int find_path_dfs(Maze* maze, unsigned long long* time_us, int* steps) {
    if (!maze) return 0;  // Проверка на NULL указатель

    clock_t start_time = clock();  // Запоминаем время начала поиска
    int step = 0;                  // Счетчик для нумерации посещенных клеток (0-9)
    *steps = 0;                    // Обнуляем счетчик шагов

    // Запускаем рекурсивный поиск из стартовой клетки
    int result = dfs_recursive(maze, maze->start_row, maze->start_col, &step,
        start_time, time_us, steps);

    // Если не нашли путь, все равно замеряем время выполнения
    if (!result) {
        clock_t end_time = clock();
        *time_us = (unsigned long long)((end_time - start_time) * 1000000 / CLOCKS_PER_SEC);
    }

    return result;  // Возвращаем результат поиска
}