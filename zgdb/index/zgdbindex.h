#ifndef ZGDBPROJECT_ZGDBINDEX_H
#define ZGDBPROJECT_ZGDBINDEX_H

#include "../format/zgdbfile.h"

/*
 * Флаги состояния индекса
 * INDEX_NEW - индекс создан, привязанного блока еще не существует, id и offset равны 0
 * INDEX_ALIVE - индекс привязан к существующему блоку, имеет id и соответствующий offset
 * INDEX_DEAD - индекс существует, привязан к блоку, который является "мёртвым" и может быть
 *              переиспользован, id равен 0, offset сохраняется без изменений
 */
//typedef enum flags : uint8_t {
//    INDEX_NEW = 0,
//    INDEX_ALIVE = 1,
//    INDEX_DEAD = 2
//} index_flags;
const uint8_t INDEX_NEW = 0;
const uint8_t INDEX_ALIVE = 1;
const uint8_t INDEX_DEAD = 2;


/*
 * Структура для ID привязанного к индексу блока
 * timestamp - время создания блока в UNIX timestamp (4 байта)
 * offset - смещение блока на момент создания (5 байт)
 */
typedef struct __attribute__((packed)) id {
    uint32_t timestamp;
    uint64_t offset: 40;
} blockId;

/*
 * Структура для индекса
 * flag - флаг состояния (см. index_flags)
 * offset - смещение привязанного блока от конца индексов в файле
 * id - идентификатор привязанного блока
 */
typedef struct __attribute__((packed)) index {
    uint8_t flag;
    uint64_t offset: 40;
    blockId id;
} zgdbIndex;

/*
 * Функция получения индекса по его порядковому номеру.
 * Возвращает NULL при неудаче
 */
zgdbIndex* getIndexByOrder(zgdbFile* file, uint64_t order);

/*
 * Функция получения индекса по ID привязанного блока.
 * Возвращает NULL при неудаче
 */
zgdbIndex* getIndexById(zgdbFile* file, blockId id);

/*
 * Функция получения порядкового номера индекс по ID привязанного блока.
 * Возвращает ???NULL при неудаче
 */
uint64_t getIndexOrderById(zgdbFile* file, blockId id);

void killIndexByOrder(zgdbFile* file, uint64_t order);

void killIndexById(zgdbFile* file, blockId id);

#endif
