#ifndef ZGDBPROJECT_ZGDBSCHEMA_H
#define ZGDBPROJECT_ZGDBSCHEMA_H

#include "../format/zgdbfile.h"
#include "../data/zgdbdata.h"

/*
 * Структура для схемы документа
 * elements - указатель на массив элементов
 * elementCount - количество элементов
 */
typedef struct documentSchema {
    size_t elementCount;
    element* elements;
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
void addStringToSchema(documentSchema* schema, unsigned char* key, string initValue);

/*
 * Функция для создания документа в файле из заданной схеме
 */
void createDocument(zgdbFile* file, documentSchema* schema);//TODO вставка по идее должна быть не здесь

#endif
