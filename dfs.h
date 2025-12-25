#ifndef DFS_H  // «ащита от повторного включени€
#define DFS_H

#include "maze.h"

// ‘ункци€ поиска пути алгоритмом DFS (Depth-First Search - поиск в глубину)
// maze - указатель на лабиринт, в котором ищем путь
// time_us - указатель дл€ возврата времени выполнени€ в микросекундах
// steps - указатель дл€ возврата количества шагов (посещенных клеток)
// ¬озвращает 1 если путь найден, 0 если нет
int find_path_dfs(Maze* maze, unsigned long long* time_us, int* step);

#endif