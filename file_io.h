#ifndef FILE_IO_H  // Защита от повторного включения
#define FILE_IO_H

#include "maze.h"

// Функция для сохранения лабиринта в текстовый файл
// maze - указатель на лабиринт для сохранения
// filename - имя файла для сохранения
// Возвращает 1 при успешном сохранении, 0 при ошибке
int save_maze_to_file(const Maze* maze, const char* filename);

// Функция для загрузки лабиринта из текстового файла
// filename - имя файла для загрузки
// Возвращает указатель на загруженный лабиринт или NULL при ошибке
Maze* load_maze_from_file(const char* filename);

#endif