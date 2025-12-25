#include <stdio.h>
#include <stdlib.h>
#include <time.h>    // Для генерации случайных чисел
#include <string.h>  // Для работы со строками
#include "maze.h"

// Создает пустой лабиринт заданного размера
// rows - количество строк, cols - количество столбцов
// Возвращает указатель на созданный лабиринт или NULL при ошибке выделения памяти
Maze* create_maze(int rows, int cols) {
    Maze* maze = (Maze*)malloc(sizeof(Maze));  // Выделяем память для структуры лабиринта
    if (!maze) return NULL;  // Если выделение не удалось, возвращаем NULL

    maze->rows = rows;
    maze->cols = cols;
    maze->last_stats = NULL;  // Пока статистики нет

    // Выделение памяти для двумерного массива cells (строки)
    // rows указателей на int
    maze->cells = (int**)malloc(rows * sizeof(int*));
    if (!maze->cells) {  // Если не удалось выделить память для строк
        free(maze);       // Освобождаем уже выделенную память
        return NULL;
    }

    // Выделяем память для каждой строки (массивов столбцов)
    for (int i = 0; i < rows; i++) {
        maze->cells[i] = (int*)malloc(cols * sizeof(int));
        if (!maze->cells[i]) {  // Если выделение строки не удалось
            // Освобождаем уже выделенную память для предыдущих строк
            for (int j = 0; j < i; j++) {
                free(maze->cells[j]);
            }
            free(maze->cells);  // Освобождаем массив указателей
            free(maze);         // Освобождаем структуру
            return NULL;
        }
    }

    // Установка начальных значений всех клеток как стен (1)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            maze->cells[i][j] = 1;  // Изначально все клетки - стены
        }
    }

    return maze;  // Возвращаем указатель на созданный лабиринт
}

// Освобождает всю память, занятую лабиринтом
// maze - указатель на лабиринт, который нужно освободить
void free_maze(Maze* maze) {
    if (!maze) return;  // Если указатель NULL, ничего не делаем

    // Освобождаем каждую строку массива cells
    for (int i = 0; i < maze->rows; i++) {
        free(maze->cells[i]);
    }
    free(maze->cells);  // Освобождаем массив указателей на строки

    // Освобождаем строку со статистикой, если она была выделена
    if (maze->last_stats) {
        free(maze->last_stats);
    }

    free(maze);  // Освобождаем саму структуру
}

// Выводит лабиринт в консоль с декоративной рамкой по краям
// maze - указатель на лабиринт для отображения
void print_maze(const Maze* maze) {
    if (!maze) return;  // Проверка на NULL указатель

    // Верхняя граница лабиринта
    printf("\n");
    for (int j = 0; j < maze->cols + 2; j++) {
        printf("#");
    }
    printf("\n");

    // Вывод каждой строки лабиринта с боковыми границами
    for (int i = 0; i < maze->rows; i++) {
        printf("#");  // Левая граница
        for (int j = 0; j < maze->cols; j++) {
            int cell = maze->cells[i][j];
            // Преобразование числового значения клетки в символ для отображения
            if (cell == 0) printf(" ");       // Проход - пробел
            else if (cell == 1) printf("#");  // Стена - решетка
            else if (cell == 2) printf("S");  // Старт - буква S
            else if (cell == 3) printf("E");  // Выход - буква E
            else if (cell == -1) printf("."); // Посещенная клетка BFS - точка
            else if (cell < -1) printf("%d", (abs(cell) - 100) % 10); // Путь BFS - цифры 0-9
            else printf("%d", (cell - 4) % 10); // Посещенные клетки DFS - цифры 0-9
        }
        printf("#\n");  // Правая граница
    }

    // Нижняя граница лабиринта
    for (int j = 0; j < maze->cols + 2; j++) {
        printf("#");
    }
    printf("\n\n");
}

// Генерация лабиринта с использованием алгоритма Прима
// rows - количество строк, cols - количество столбцов (минимум 3x3)
// Возвращает указатель на созданный лабиринт или NULL при ошибке
Maze* generate_maze_prim(int rows, int cols) {
    if (rows < 3 || cols < 3) return NULL;  // Проверка минимального размера

    Maze* maze = create_maze(rows, cols);  // Создаем пустой лабиринт
    if (!maze) return NULL;

    srand((unsigned int)time(NULL));  // Инициализация генератора случайных чисел

    // ВАЖНО: делаем старт в левом верхнем углу (0,0)
    int start_r = 0;
    int start_c = 0;

    // ВАЖНО: делаем выход в противоположном углу (нижний правый)
    int exit_r = rows - 1;
    int exit_c = cols - 1;

    // Помечаем начальную клетку как проход
    maze->cells[start_r][start_c] = 0;

    // Структура для хранения информации о стенах в алгоритме Прима
    // Алгоритм Прима работает со списком "фронтальных" стен
    typedef struct {
        int r;        // Строка стены
        int c;        // Столбец стены
        int from_r;   // Строка клетки, от которой идет стена
        int from_c;   // Столбец клетки, от которой идет стена
    } Wall;

    Wall* walls = NULL;      // Динамический массив стен
    int wall_count = 0;      // Текущее количество стен в массиве
    int wall_capacity = 4;   // Начальная емкость массива стен

    // Выделяем начальную память для массива стен
    walls = (Wall*)malloc(wall_capacity * sizeof(Wall));
    if (!walls) {  // Если не удалось выделить память
        free_maze(maze);
        return NULL;
    }

    // Массивы смещений для проверки соседних клеток (вверх, вниз, влево, вправо)
    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };

    // Добавляем стены вокруг начальной клетки в список фронтальных стен
    for (int i = 0; i < 4; i++) {
        int nr = start_r + dr[i];  // Координата соседней клетки по строке
        int nc = start_c + dc[i];  // Координата соседней клетки по столбцу

        // Проверяем, что соседняя клетка в пределах лабиринта и является стеной
        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && maze->cells[nr][nc] == 1) {
            // Если массив стен заполнен, увеличиваем его емкость в 2 раза
            if (wall_count >= wall_capacity) {
                wall_capacity *= 2;
                Wall* new_walls = (Wall*)realloc(walls, wall_capacity * sizeof(Wall));
                if (!new_walls) {  // Если realloc не удался
                    free(walls);
                    free_maze(maze);
                    return NULL;
                }
                walls = new_walls;
            }

            // Добавляем стену в список
            walls[wall_count].r = nr;
            walls[wall_count].c = nc;
            walls[wall_count].from_r = start_r;
            walls[wall_count].from_c = start_c;
            wall_count++;
        }
    }

    // Основной цикл алгоритма Прима - пока есть стены в списке
    int iterations = 0;
    int max_iterations = rows * cols * 10;  // Максимальное число итераций для предотвращения бесконечного цикла

    while (wall_count > 0 && iterations < max_iterations) {
        iterations++;

        // Выбираем случайную стену из списка
        int wall_idx = rand() % wall_count;
        Wall wall = walls[wall_idx];

        // Определяем клетку за стеной (в два шага от текущей клетки)
        int opposite_r = wall.r + (wall.r - wall.from_r);
        int opposite_c = wall.c + (wall.c - wall.from_c);

        // Проверяем, что клетка за стеной в пределах лабиринта и является стеной
        if (opposite_r >= 0 && opposite_r < rows && opposite_c >= 0 && opposite_c < cols &&
            maze->cells[opposite_r][opposite_c] == 1) {

            // Прорубаем стену и клетку за ней (делаем их проходами)
            maze->cells[wall.r][wall.c] = 0;
            maze->cells[opposite_r][opposite_c] = 0;

            // Добавляем новые стены вокруг только что открытой клетки в список
            for (int i = 0; i < 4; i++) {
                int nr = opposite_r + dr[i];
                int nc = opposite_c + dc[i];

                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && maze->cells[nr][nc] == 1) {
                    // Проверяем, нужно ли увеличить массив стен
                    if (wall_count >= wall_capacity) {
                        wall_capacity *= 2;
                        Wall* new_walls = (Wall*)realloc(walls, wall_capacity * sizeof(Wall));
                        if (!new_walls) {
                            free(walls);
                            free_maze(maze);
                            return NULL;
                        }
                        walls = new_walls;
                    }

                    // Добавляем новую стену в список
                    walls[wall_count].r = nr;
                    walls[wall_count].c = nc;
                    walls[wall_count].from_r = opposite_r;
                    walls[wall_count].from_c = opposite_c;
                    wall_count++;
                }
            }
            // создаем дополнительный проход к выходу для гарантии пути
            int distance_to_exit = abs(opposite_r - exit_r) + abs(opposite_c - exit_c);
            if (distance_to_exit < 3) {
                // 25% вероятность создания дополнительного прохода к выходу
                if (rand() % 4 == 0) {
                    // Пробуем прорубить еще одну стену в направлении выхода
                    for (int i = 0; i < 4; i++) {
                        int nr = opposite_r + dr[i];
                        int nc = opposite_c + dc[i];
                        int nr2 = nr + dr[i];  // Клетка через стену
                        int nc2 = nc + dc[i];

                        // Проверяем, что можно прорубить стену к существующему проходу
                        if (nr2 >= 0 && nr2 < rows && nc2 >= 0 && nc2 < cols &&
                            maze->cells[nr][nc] == 1 && maze->cells[nr2][nc2] == 0) {
                            maze->cells[nr][nc] = 0;  // Прорубаем стену
                            break;
                        }
                    }
                }
            }
        }

        // Удаляем обработанную стену из списка (перемещаем последнюю на ее место)
        walls[wall_idx] = walls[wall_count - 1];
        wall_count--;
    }

    free(walls);  // Освобождаем массив стен

    // Теперь убедимся, что к выходу есть проход
    // Проверяем соседей выходной клетки
    int has_path_to_exit = 0;
    for (int i = 0; i < 4; i++) {
        int nr = exit_r + dr[i];
        int nc = exit_c + dc[i];

        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && maze->cells[nr][nc] == 0) {
            has_path_to_exit = 1;  // Нашли проход к выходу
            break;
        }
    }

    // Если к выходу нет прохода, принудительно прорубаем путь
    if (!has_path_to_exit) {
        // Пытаемся найти ближайший проход к выходу
        int found_path = 0;
        // Ищем по возрастающему расстоянию от выхода
        for (int distance = 1; distance < rows + cols && !found_path; distance++) {
            for (int i = 0; i <= distance && !found_path; i++) {
                int j = distance - i;

                // Проверяем клетки на заданном расстоянии от выхода
                for (int sign_i = -1; sign_i <= 1 && !found_path; sign_i += 2) {
                    for (int sign_j = -1; sign_j <= 1 && !found_path; sign_j += 2) {
                        int nr = exit_r + sign_i * i;
                        int nc = exit_c + sign_j * j;

                        // Если нашли проход, прорубаем к нему путь
                        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols &&
                            maze->cells[nr][nc] == 0) {
                            // Прямолинейно прорубаем путь от выхода к найденному проходу
                            int current_r = exit_r;
                            int current_c = exit_c;

                            while (!(current_r == nr && current_c == nc)) {
                                int next_r = current_r;
                                int next_c = current_c;

                                // Определяем направление движения к проходу
                                if (current_r < nr) next_r++;
                                else if (current_r > nr) next_r--;
                                else if (current_c < nc) next_c++;
                                else if (current_c > nc) next_c--;

                                maze->cells[next_r][next_c] = 0;  // Прорубаем клетку
                                current_r = next_r;
                                current_c = next_c;
                            }

                            found_path = 1;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Устанавливаем стартовую клетку
    maze->cells[start_r][start_c] = 2;  // Значение 2 - старт
    maze->start_row = start_r;
    maze->start_col = start_c;

    // Устанавливаем выходную клетку
    maze->cells[exit_r][exit_c] = 3;  // Значение 3 - выход
    maze->exit_row = exit_r;
    maze->exit_col = exit_c;

    return maze;  // Возвращаем готовый лабиринт
}