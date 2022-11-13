#ifndef ZGDBPROJECT_ZGDBFILE_H
#define ZGDBPROJECT_ZGDBFILE_H
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdint.h>
#include "index/list/freelist.h"
#ifdef __linux__
#include <sys/mman.h>
#endif
#ifdef __MINGW32__
#include "mman-win32/mman.h"
#endif

/*
 * Структура для заголовка файла, где храниться мета информация
 * zgdbType - 4 символа в UTF8 (ZGDB)
 * version - версия типа
 * indexCount - количество всех индексов (5 байт)
 * betweenSpace - пространство между первым блоком и последним индексом
 * fileSize - размер файла (с учётом заголовка), должен быть 8 байт
 * freeListOffset - смещение на структуру с доступными индексами (reserved)
 */
typedef struct __attribute__((packed)) zgdbHeader {
    uint32_t zgdbType;
    uint8_t version;
    uint64_t indexCount: 40;
    uint8_t betweenSpace;
    off_t fileSize;
    uint64_t freeListOffset;
} zgdbHeader;

/*
 * Структура для представления zgdb файла
 */
typedef struct zgdbFile {
    FILE* file;
    zgdbHeader zgdbHeader;
    void* pIndexesMmap;
    freeIndexesList freeList;
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
