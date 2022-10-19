#ifndef ZGDBPROJECT_ZGDBDATA_H
#define ZGDBPROJECT_ZGDBDATA_H

#include <stdbool.h>

#include "../format/zgdbfile.h"

/*
 * Описание возможных типов данных в документе
 * TYPE_INT - int32
 * TYPE_DOUBLE - double
 * TYPE_BOOLEAN - uint8 (два значения)
 * TYPE_STRING - UTF-8 string (с длиной в начале)
 * TYPE_DOCUMENT - вложенный документ
 */
typedef enum elementType {
    TYPE_INT = 0x01,
    TYPE_DOUBLE = 0x02,
    TYPE_BOOLEAN = 0x03,
    TYPE_STRING = 0x04,
    TYPE_EMBEDDED_DOCUMENT = 0x05
} elementType;

/*
 * Описание символов-терминаторов в документе
 * DOCUMENT_TERMINATOR - терминатор в конце документа
 * EMBEDDED_DOCUMENT_TERMINATOR - терминатор в конце вложенного документа
 * NULL_TERMINATOR - нуль терминатор для строк и ключей
 */
typedef enum terminators {
    DOCUMENT_TERMINATOR = 0xFF,
    EMBEDDED_DOCUMENT_TERMINATOR = 0xFE,
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
    uint64_t offset;
} documentId;

/*
 * Структура для заголовка документа
 * size - размер документа в байтах (5 байт)
 * indexOrder - порядковый номер привязанного индекса (5 байт)
 * internalOffset - смещение документа относительно родительского (5 байт)
 */
typedef struct __attribute__((packed)) documentHeader {
    uint64_t size: 40;
    union {
        uint64_t indexOrder: 40;
        uint64_t internalOffset: 40;
    };
    documentId id;
} documentHeader;

/*
 * Структура для строчки
 * size - размер
 * data - указатель на символы
 */
typedef struct string {
    uint32_t size;
    unsigned char* data;
} string;

typedef struct document document;

/*
 * Структура для представления элемента вида - key:value
 * type - один из типов выше (enum elementType)
 * key - ключ (строчка длиной в 12 байт и нуль)
 * В union храниться значение элемента в случае типов int, double, boolean;
 * А в случае типов string и document храниться указатель на соответствующую
 * структуру
 */
typedef struct element {
    uint8_t type;
    unsigned char key[13];
    union {
        int32_t integerValue;
        double doubleValue;
        uint8_t booleanValue;
        string* stringValue;
        document* documentValue;
    };
} element;

/*
 * Структура для описания документа
 * header - заголовок документа
 * elements - указатель на массив элементов
 * elementCount - количество элементов
 */
typedef struct document {
    documentHeader header;
    element* elements;
    size_t elementCount;
} document;

/*
 * Структура для схемы документа
 * size - размер блока в байтах (5 байт)
 * elements - указатель на массив элементов
 * elementCount - количество элементов
 */
typedef struct documentSchema {
    uint64_t size: 40;
    element* elements;
    size_t elementCount;
} documentSchema;

/*
 * Функция для инициализации схемы
 */
documentSchema initSchema(size_t elementCount);

/*
 * Функции для расширения схемы
 */
void addIntToSchema(documentSchema* schema, unsigned char* key, int32_t initValue);
void addDoubleToSchema(documentSchema* schema, unsigned char* key, double initValue);
void addBooleanToSchema(documentSchema* schema, unsigned char* key, uint8_t initValue);
void addStringToSchema(documentSchema* schema, unsigned char* key, string* initValue);
void addDocumentToSchema(documentSchema* schema, unsigned char* key, document* initValue);

/*
 * Функция создания индекса в файле.
 * Возвращает indexNumber из заголовка при неудаче
 */
uint64_t createIndex(zgdbFile* file);

/*
 * Функция для создания документа в файле из заданной схеме
 */
void createDocument(zgdbFile* file, documentSchema* schema);



#endif
