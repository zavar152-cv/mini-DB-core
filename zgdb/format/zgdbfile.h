#ifndef ZGDBPROJECT_ZGDBFILE_H
#define ZGDBPROJECT_ZGDBFILE_H

#include <stdio.h>
#include <stdint.h>

/*
 * Структура для заголовка файла, где храниться мета информация
 * zgdbType - 4 символа в UTF8 (ZGDB)
 * indexCount - количество всех индексов (5 байт)
 */
typedef struct __attribute__((packed)) zgdbHeader {
    uint32_t zgdbType;
    uint64_t indexCount: 40;
} zgdbHeader;

/*
 * Структура для представления zgdb файла
 */
typedef struct zgdbFile {
    FILE* file;
    zgdbHeader* zgdbHeader;
} zgdbFile;

/*
 * Загрузка/создание zgdb файла
 */
zgdbFile* loadOrCreateZgdbFile(const char* path);

/*
 * Закрытие zgdb файла
 */
uint8_t closeZgdbFile(zgdbFile* file);

/*
 * Сохранение информации в заголовке на диск
 */
void saveHeader(zgdbFile* file);

#endif
