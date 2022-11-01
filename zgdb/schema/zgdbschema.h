#ifndef ZGDBPROJECT_ZGDBSCHEMA_H
#define ZGDBPROJECT_ZGDBSCHEMA_H

#include "../format/zgdbfile.h"
#include "../data/zgdbdata.h"

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
 * Функция для создания документа в файле из заданной схеме
 */
void createDocument(zgdbFile* file, documentSchema* schema);

#endif
