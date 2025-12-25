#include <stdio.h>
#include <stdlib.h>
#include <time.h>   // Для измерения времени
#include "maze.h"
#include "bfs.h"

// Функция для создания очереди с начальной емкостью
// capacity - начальный размер очереди
// Возвращает указатель на созданную очередь или NULL при ошибке
Queue* create_queue(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) return NULL;

    q->data = (int*)malloc(capacity * sizeof(int));
    if (!q->data) {
        free(q);
        return NULL;
    }

    q->capacity = capacity;
    q->head = 0;  // Очередь пуста - голова и хвост в начале
    q->tail = 0;
    return q;
}

// Освобождает память, занятую очередью
// q - указатель на очередь для освобождения
void free_queue(Queue* q) {
    if (!q) return;
    free(q->data);  // Освобождаем массив данных
    free(q);        // Освобождаем структуру
}

// Добавляет элемент в конец очереди
// q - указатель на очередь
// value - значение для добавления
void enqueue(Queue* q, int value) {
    // Если массив заполнен, увеличиваем его размер в 2 раза
    if (q->tail >= q->capacity) {
        q->capacity *= 2;
        q->data = (int*)realloc(q->data, q->capacity * sizeof(int));
    }
    q->data[q->tail++] = value;  // Добавляем элемент и увеличиваем tail
}

// Извлекает элемент из начала очереди
// q - указатель на очередь
// Возвращает извлеченное значение или -1 если очередь пуста
int dequeue(Queue* q) {
    if (q->head >= q->tail) return -1;  // Очередь пуста
    return q->data[q->head++];  // Возвращаем элемент и увеличиваем head
}

// Проверяет, пуста ли очередь
// q - указатель на очередь
// Возвращает 1 если очередь пуста, 0 если нет
int is_empty(Queue* q) {
    return q->head >= q->tail;  // Если head достиг tail, очередь пуста
}

// Основная функция поиска пути алгоритмом BFS
// maze - указатель на лабиринт для поиска
// time_us - указатель для сохранения времени выполнения (в микросекундах)
// visited_cells - указатель для сохранения количества посещенных клеток
// path_length - указатель для сохранения длины найденного пути
// Возвращает 1 если путь найден, 0 если нет
int find_path_bfs(Maze* maze, unsigned long long* time_us,
    int* visited_cells, int* path_length) {
    if (!maze) return 0;  // Проверка на NULL указатель

    clock_t start_time = clock();  // Запоминаем время начала поиска
    *visited_cells = 0;            // Обнуляем счетчик посещенных клеток
    *path_length = 0;              // Обнуляем длину пути

    // Выделяем память для массивов, необходимых для работы BFS:

    // parent_row и parent_col - массивы для хранения "родителей" каждой клетки
    // (координаты клетки, из которой мы пришли в данную)
    // Это нужно для восстановления пути в конце
    int** parent_row = (int**)malloc(maze->rows * sizeof(int*));
    int** parent_col = (int**)malloc(maze->rows * sizeof(int*));

    // visited - массив для отметки посещенных клеток (1 - посещена, 0 - нет)
    int** visited = (int**)malloc(maze->rows * sizeof(int*));

    // Проверяем успешность выделения памяти
    if (!parent_row || !parent_col || !visited) {
        // Если выделение не удалось, освобождаем все, что успели выделить
        if (parent_row) free(parent_row);
        if (parent_col) free(parent_col);
        if (visited) free(visited);
        return 0;
    }

    // Выделяем память для каждой строки массивов
    for (int i = 0; i < maze->rows; i++) {
        parent_row[i] = (int*)malloc(maze->cols * sizeof(int));
        parent_col[i] = (int*)malloc(maze->cols * sizeof(int));
        visited[i] = (int*)malloc(maze->cols * sizeof(int));

        // Если выделение для какой-то строки не удалось
        if (!parent_row[i] || !parent_col[i] || !visited[i]) {
            // Освобождаем все уже выделенные строки
            for (int j = 0; j <= i; j++) {
                if (parent_row[j]) free(parent_row[j]);
                if (parent_col[j]) free(parent_col[j]);
                if (visited[j]) free(visited[j]);
            }
            // Освобождаем массивы указателей
            free(parent_row);
            free(parent_col);
            free(visited);
            return 0;
        }

        // Инициализируем массивы начальными значениями
        for (int j = 0; j < maze->cols; j++) {
            parent_row[i][j] = -1;   // -1 означает "нет родителя"
            parent_col[i][j] = -1;
            visited[i][j] = 0;       // 0 означает "не посещена"
        }
    }

    // Создаем очередь для BFS, начальная емкость - общее количество клеток
    Queue* q = create_queue(maze->rows * maze->cols);
    if (!q) {  // Если не удалось создать очередь
        // Освобождаем все выделенные массивы
        for (int i = 0; i < maze->rows; i++) {
            free(parent_row[i]);
            free(parent_col[i]);
            free(visited[i]);
        }
        free(parent_row);
        free(parent_col);
        free(visited);
        return 0;
    }

    // Начинаем поиск со стартовой клетки
    // Преобразуем координаты в линейный индекс для хранения в очереди
    int start_idx = maze->start_row * maze->cols + maze->start_col;
    enqueue(q, start_idx);  // Добавляем стартовую клетку в очередь

    visited[maze->start_row][maze->start_col] = 1;  // Отмечаем как посещенную
    (*visited_cells)++;  // Увеличиваем счетчик посещенных клеток

    // Массивы смещений для проверки 4-х соседей клетки (вверх, вниз, влево, вправо)
    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };

    int found = 0;           // Флаг найденного пути
    int exit_r = -1, exit_c = -1;  // Координаты выхода (если найдем)

    // Основной цикл BFS - пока очередь не пуста
    while (!is_empty(q)) {
        int current_idx = dequeue(q);  // Извлекаем клетку из очереди

        // Преобразуем линейный индекс обратно в координаты
        int r = current_idx / maze->cols;
        int c = current_idx % maze->cols;

        // Если это выходная клетка (значение 3)
        if (maze->cells[r][c] == 3) {
            exit_r = r;
            exit_c = c;
            found = 1;  // Путь найден!
            break;      // Прерываем цикл
        }

        // Проверяем всех 4-х соседей текущей клетки
        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i];  // Координаты соседа
            int nc = c + dc[i];

            // Проверяем, что сосед в пределах лабиринта
            if (nr >= 0 && nr < maze->rows && nc >= 0 && nc < maze->cols) {
                // Если сосед не посещен и не является стеной
                if (!visited[nr][nc] && maze->cells[nr][nc] != 1) {
                    visited[nr][nc] = 1;           // Отмечаем как посещенную
                    (*visited_cells)++;            // Увеличиваем счетчик

                    // Запоминаем "родителя" - клетку, из которой пришли
                    parent_row[nr][nc] = r;
                    parent_col[nr][nc] = c;

                    // Добавляем соседа в очередь для дальнейшего исследования
                    int new_idx = nr * maze->cols + nc;
                    enqueue(q, new_idx);
                }
            }
        }
    }

    // Восстанавливаем путь, если он был найден
    if (found) {
        // Сначала отмечаем все посещенные клетки точками (.)
        // Это нужно для визуализации процесса поиска BFS
        for (int i = 0; i < maze->rows; i++) {
            for (int j = 0; j < maze->cols; j++) {
                if (visited[i][j] && maze->cells[i][j] == 0) {
                    maze->cells[i][j] = -1;  // -1 означает "точка" (посещенная клетка BFS)
                }
            }
        }

        // Восстанавливаем путь от выхода к старту через цепочку родителей
        int current_r = exit_r;
        int current_c = exit_c;
        int path_size = 0;  // Счетчик длины пути

        // Сначала считаем длину пути, идя от выхода к старту
        while (!(current_r == maze->start_row && current_c == maze->start_col)) {
            int prev_r = parent_row[current_r][current_c];
            int prev_c = parent_col[current_r][current_c];
            current_r = prev_r;
            current_c = prev_c;
            path_size++;  // Увеличиваем счетчик длины
        }

        // Теперь пройдем путь от старта к выходу и отметим цифрами
        // Сначала вернемся в стартовую клетку
        current_r = maze->start_row;
        current_c = maze->start_col;

        // Создаем массивы для хранения координат пути в правильном порядке
        int* path_rows = (int*)malloc((path_size + 1) * sizeof(int));
        int* path_cols = (int*)malloc((path_size + 1) * sizeof(int));

        // Собираем путь от старта к выходу, проходя от выхода к старту,
        // но сохраняя координаты в обратном порядке
        int idx = 0;
        int temp_r = exit_r;
        int temp_c = exit_c;

        // Идем от выхода к старту через родителей
        while (!(temp_r == maze->start_row && temp_c == maze->start_col)) {
            // Сохраняем координаты в обратном порядке (от старта к выходу)
            path_rows[path_size - idx - 1] = temp_r;
            path_cols[path_size - idx - 1] = temp_c;
            idx++;

            // Переходим к родительской клетке
            int prev_r = parent_row[temp_r][temp_c];
            int prev_c = parent_col[temp_r][temp_c];
            temp_r = prev_r;
            temp_c = prev_c;
        }

        // Теперь отмечаем путь цифрами от 0 до 9 в правильном порядке
        for (int step = 0; step < path_size; step++) {
            int r = path_rows[step];
            int c = path_cols[step];

            // Не перезаписываем выходную клетку (оставляем 'E')
            if (maze->cells[r][c] != 3) {
                // Значения -100, -101, -102... преобразуются в цифры 0, 1, 2...
                maze->cells[r][c] = -100 - (step % 10);
            }
        }

        *path_length = path_size;  // Сохраняем длину найденного пути

        // Освобождаем временные массивы
        free(path_rows);
        free(path_cols);
    }

    // Замеряем время окончания поиска
    clock_t end_time = clock();
    // Вычисляем время в микросекундах
    *time_us = (unsigned long long)((end_time - start_time) * 1000000 / CLOCKS_PER_SEC);

    // Освобождаем всю выделенную память
    free_queue(q);  // Очередь

    // Освобождаем вспомогательные массивы
    for (int i = 0; i < maze->rows; i++) {
        free(parent_row[i]);
        free(parent_col[i]);
        free(visited[i]);
    }
    free(parent_row);
    free(parent_col);
    free(visited);

    return found;  // Возвращаем результат поиска
}