#ifndef ZGDBPROJECT_ZGDBINDEX_H
#define ZGDBPROJECT_ZGDBINDEX_H

#include <stdbool.h>

#include "format/zgdbfile.h"

/*
 * Флаги состояния индекса
 * INDEX_NEW - индекс создан, привязанного блока еще не существует, id и offset равны 0
 * INDEX_ALIVE - индекс привязан к существующему блоку, имеет id и соответствующий offset
 * INDEX_DEAD - индекс существует, привязан к блоку, который является "мёртвым" и может быть
 *              переиспользован, id равен 0, offset сохраняется без изменений
 * INDEX_INVALID - служебный флаг, необходим для возврата ошибки из функций
 */
typedef enum indexFlags {
    INDEX_NEW = 0,
    INDEX_ALIVE = 1,
    INDEX_DEAD = 2,
    INDEX_INVALID = 3
} indexFlags;

/*
 * Структура для индекса
 * flag - флаг состояния (см. index_flags)
 * offset - смещение привязанного блока от начала файла
 */
typedef struct __attribute__((packed)) zgdbIndex {
    uint8_t flag;
    uint64_t offset;
} zgdbIndex;

/*
 * Функция создания индекса в файле.
 * Возвращает indexNumber из заголовка при неудаче
 */
uint64_t createIndex(zgdbFile* file);

/*
 * Функция создания нескольких индексов в файле.
 * Возвращает indexNumber из заголовка при неудаче
 */
uint64_t createIndexes(zgdbFile* file, uint64_t n);

/*
 * Функция получения индекса по его порядковому номеру.
 * Возвращает INDEX_INVALID во флаге индекса при неудаче
 */
zgdbIndex getIndex(zgdbFile* file, uint64_t order);

/*
 * Функция, которая помечает индекс по его порядковому номеру как живой и "привязывает" к нему блок.
 * Установит в flag - INDEX_ALIVE и изменит blockOffset.
 * Возвращает false в случае неудачи
 */
bool attachIndexToBlock(zgdbFile* file, uint64_t order, off_t blockOffset);

/*
 * Функция, которая помечает индекс по его порядковому номеру как мёртвый.
 * Установит в flag - INDEX_DEAD.
 * Возвращает false в случае неудачи
 */
bool killIndex(zgdbFile* file, uint64_t order);

/*
 * Функция для обновления offset в блоке по его порядковому номеру
 */
bool updateOffset(zgdbFile* file, uint64_t indexOrder, off_t offset);

#endif
