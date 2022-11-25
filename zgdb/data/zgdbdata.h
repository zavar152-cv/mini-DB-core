#ifndef ZGDBPROJECT_ZGDBDATA_H
#define ZGDBPROJECT_ZGDBDATA_H

#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "format/zgdbfile.h"

/*
 * Описание возможных типов данных в документе
 * TYPE_INT - int32
 * TYPE_DOUBLE - double
 * TYPE_BOOLEAN - uint8 (два значения)
 * TYPE_TEXT - UTF-8 text (с длиной в начале)
 */
typedef enum elementType {
    TYPE_INT = 0x01,
    TYPE_DOUBLE = 0x02,
    TYPE_BOOLEAN = 0x03,
    TYPE_TEXT = 0x04,
} elementType;

/*
 * Описание символов-терминаторов в документе
 * DOCUMENT_TERMINATOR - терминатор в конце документа
 * NULL_TERMINATOR - нуль терминатор для строк и ключей
 */
typedef enum terminators {
    DOCUMENT_TERMINATOR = 0xFF,
    NULL_TERMINATOR = 0x00
} terminators;

/*
 * Структура для ID документа
 * timestamp - время создания документа в UNIX timestamp (4 байта)
 * offset - смещение документа на момент создания
 * Если блок вложенный, то его id = 0
 */
typedef struct __attribute__((packed)) documentId {
    uint32_t timestamp;
    off_t offset;
} documentId;

/*
 * Структура для заголовка документа
 * size - размер документа в байтах (5 байт)
 * capacity - максимальный размер документа (5 байт)
 * indexAttached - порядковый номер привязанного индекса (5 байт)
 * id - идентификатор документа (12 байт)
 * name - имя документа (строчка длиной в 12 байт и нуль)
 * indexSon - порядковый номер индекса первого сына (5 байт)
 * indexBrother - порядковый номер индекса брата (5 байт)
 * attrCount - количество атрибутов (4 байта)
 *
 * 0 для indexSon и indexBrother означает NULL, т.к. ни у кого не может быть
 * сыном или братом корень дерева (корень привязан к нулевому индексу)
 */
typedef struct documentHeader {
    uint64_t size: 40;
    uint64_t capacity: 40;
    uint64_t indexAttached: 40;
    uint64_t indexSon: 40;
    uint64_t indexBrother: 40;
    uint32_t attrCount;
    char name[13];
    documentId id;
} documentHeader;

/*
 * Структура для строчки
 * size - размер
 * data - указатель на символы
 */
typedef struct text {
    uint32_t size;
    char* data;
} text;

typedef struct document document;

/*
 * Структура для представления элемента вида - key:value
 * type - один из типов выше (enum elementType)
 * key - ключ (строчка длиной в 12 байт и нуль)
 * В union храниться значение элемента типов int, double, boolean, text;
 */
typedef struct element {
    uint8_t type;
    char key[13];
    union {
        int32_t integerValue;
        double doubleValue;
        uint8_t booleanValue;
        text textValue;
    };
} element;

/*
 * Структура для описания документа
 * header - заголовок документа
 * isRoot - является ли корнем
 * indexParent - индекс родителя (у корня 0)
 */
typedef struct document {//TODO remove elements
    bool isRoot;
    uint64_t indexParent: 40;
    documentHeader header;
} document;

bool isRootDocument(document document);

bool hasChild(document document);

bool hasBrother(document document);

documentId generateId(off_t offset);

off_t getElementSize(element cur);

off_t writeElement(zgdbFile* file, element cur);

element* readElement(zgdbFile* file);

#endif
