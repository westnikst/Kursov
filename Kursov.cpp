#include <stdio.h>
#include <stdlib.h>
#include <locale.h>        // Для поддержки русского языка
#include "maze.h"
#include "dfs.h"
#include "bfs.h"
#include "file_io.h"

// Функция для отображения главного меню программы
void print_menu() {
    printf("\n=== Меню программы Лабиринт ===\n");
    printf("1. Создать лабиринт\n");
    printf("2. Загрузить лабиринт из файла\n");
    printf("3. Сохранить лабиринт в файл\n");
    printf("4. Показать лабиринт\n");
    printf("5. Найти путь (DFS)\n");
    printf("6. Найти путь (BFS)\n");
    printf("7. Очистить решение\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}

int main() {
    setlocale(LC_ALL, "Russian");  // Устанавливаем русскую локаль

    Maze* maze = NULL;              // Указатель на текущий лабиринт (изначально не создан)
    int choice;                     // Переменная для хранения выбора пользователя
    char filename[256];             // Буфер для имени файла
    int rows, cols;                 // Переменные для размеров лабиринта
    unsigned long long time_us;     // Переменная для хранения времени выполнения (в микросекундах)
    int steps, visited_cells, path_length;  // Переменные для статистики

    do {
        print_menu();               // Показываем меню

        // Считываем выбор пользователя с защитой от некорректного ввода
        if (scanf_s("%d", &choice) != 1) {
            while (getchar() != '\n'); // Очищаем буфер ввода при ошибке
            printf("Ошибка ввода. Попробуйте снова.\n");
            continue;
        }

        // Обработка выбора пользователя
        switch (choice) {
        case 1: // Создать лабиринт алгоритмом Прима
            if (maze != NULL) free_maze(maze); // Освобождаем предыдущий лабиринт, если был

            printf("Введите количество строк (минимум 3): ");
            scanf_s("%d", &rows);
            printf("Введите количество столбцов (минимум 3): ");
            scanf_s("%d", &cols);

            // Проверка корректности введенных размеров
            if (rows < 3 || cols < 3) {
                printf("Некорректные данные! Минимальный размер 3x3.\n");
                break;
            }

            // Создаем новый лабиринт с помощью алгоритма Прима
            maze = generate_maze_prim(rows, cols);
            if (maze) {
                printf("Лабиринт создан успешно!\n");
                print_maze(maze);  // Показываем созданный лабиринт
            }
            else {
                printf("Ошибка создания лабиринта!\n");
            }
            break;

        case 2: // Загрузить лабиринт из файла
            if (maze != NULL) free_maze(maze); // Освобождаем предыдущий лабиринт

            printf("Введите имя файла: ");
            scanf_s("%255s", filename, (unsigned)_countof(filename));

            // Загружаем лабиринт из указанного файла
            maze = load_maze_from_file(filename);
            if (maze) {
                printf("Лабиринт загружен успешно!\n");
                print_maze(maze);
            }
            else {
                printf("Ошибка загрузки файла!\n");
            }
            break;

        case 3: // Сохранить лабиринт в файл
            if (maze == NULL) {
                printf("Сначала создайте лабиринт!\n");
                break;
            }

            printf("Введите имя файла: ");
            scanf_s("%255s", filename, (unsigned)_countof(filename));

            // Сохраняем текущий лабиринт в файл
            if (save_maze_to_file(maze, filename)) {
                printf("Лабиринт сохранен успешно!\n");
            }
            else {
                printf("Ошибка сохранения файла!\n");
            }
            break;

        case 4: // Показать лабиринт
            if (maze == NULL) {
                printf("Сначала создайте лабиринт!\n");
            }
            else {
                print_maze(maze);  // Просто отображаем лабиринт
            }
            break;

        case 5: // Найти путь с помощью алгоритма DFS (поиск в глубину)
            if (maze == NULL) {
                printf("Сначала создайте лабиринт!\n");
            }
            else {
                // Сбрасываем все предыдущие пометки (цифры пути и точки BFS)
                for (int i = 0; i < maze->rows; i++) {
                    for (int j = 0; j < maze->cols; j++) {
                        if (maze->cells[i][j] >= 4 || maze->cells[i][j] < 0) {
                            maze->cells[i][j] = 0; // Возвращаем в состояние "проход"
                        }
                    }
                }

                // Запускаем алгоритм DFS
                if (find_path_dfs(maze, &time_us, &steps)) {
                    printf("Путь найден!\n");
                    printf("Время выполнения: %llu мкс\n", time_us);
                    printf("Количество шагов: %d\n", steps);

                    // Сохраняем статистику в структуре лабиринта для возможного сохранения в файл
                    if (maze->last_stats) free(maze->last_stats);
                    maze->last_stats = (char*)malloc(100);
                    sprintf_s(maze->last_stats, 100,
                        "DFS: time=%llu мкс, steps=%d", time_us, steps);

                    print_maze(maze);  // Показываем лабиринт с найденным путем
                }
                else {
                    printf("Путь не найден!\n");
                }
            }
            break;

        case 6: // Найти путь с помощью алгоритма BFS (поиск в ширину)
            if (maze == NULL) {
                printf("Сначала создайте лабиринт!\n");
            }
            else {
                // Сбрасываем все предыдущие пометки
                for (int i = 0; i < maze->rows; i++) {
                    for (int j = 0; j < maze->cols; j++) {
                        if (maze->cells[i][j] >= 4 || maze->cells[i][j] < 0) {
                            maze->cells[i][j] = 0;
                        }
                    }
                }

                // Запускаем алгоритм BFS
                if (find_path_bfs(maze, &time_us, &visited_cells, &path_length)) {
                    printf("Путь найден!\n");
                    printf("Время выполнения: %llu мкс\n", time_us);
                    printf("Посещенные клетки: %d\n", visited_cells);
                    printf("Длина найденного пути: %d\n", path_length);

                    // Сохраняем статистику BFS
                    if (maze->last_stats) free(maze->last_stats);
                    maze->last_stats = (char*)malloc(100);
                    sprintf_s(maze->last_stats, 100,
                        "BFS: time=%llu мкс, visited=%d, path_length=%d",
                        time_us, visited_cells, path_length);

                    print_maze(maze);
                }
                else {
                    printf("Путь не найден!\n");
                }
            }
            break;

        case 7: // Очистить решение (убрать все пометки алгоритмов)
            if (maze == NULL) {
                printf("Сначала создайте лабиринт!\n");
            }
            else {
                // Сбрасываем все пометки алгоритмов (цифры DFS и точки BFS)
                for (int i = 0; i < maze->rows; i++) {
                    for (int j = 0; j < maze->cols; j++) {
                        if (maze->cells[i][j] >= 4 || maze->cells[i][j] < 0) {
                            maze->cells[i][j] = 0;
                        }
                    }
                }

                // Очищаем сохраненную статистику
                if (maze->last_stats) {
                    free(maze->last_stats);
                    maze->last_stats = NULL;
                }

                printf("Решение очищено!\n");
                print_maze(maze);  // Показываем чистый лабиринт
            }
            break;

        case 0:
            printf("Выход из программы.\n");
            break;

        default:
            printf("Неверный выбор. Попробуйте снова.\n");
        }

        // Очистка буфера ввода после каждого действия
        while (getchar() != '\n');

    } while (choice != 0);  // Продолжаем до тех пор, пока пользователь не выберет выход

    // Освобождаем память, выделенную для лабиринта, перед завершением программы
    if (maze != NULL) {
        free_maze(maze);
    }

    return 0;
}