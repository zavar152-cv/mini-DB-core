#ifndef ZGDBPROJECT_ZGDBSCHEMA_H
#define ZGDBPROJECT_ZGDBSCHEMA_H

#include "format/zgdbfile.h"
#include "data/zgdbdata.h"

/*
 * Структура для схемы документа
 * elements - указатель на массив элементов
 * size - заполненность схемы
 * capacity - количество элементов
 */
typedef struct documentSchema {
    size_t capacity;
    size_t size;
    off_t sizeOfElements;
    size_t minToastCapacity;
    bool reqToast;
    element* elements;
} documentSchema;

/*
 * Функция для инициализации схемы
 */
documentSchema initSchema(size_t capacity);

void destroySchema(documentSchema* schema);

/*
 * Функции для расширения схемы
 */
void addIntToSchema(documentSchema* schema, char* key, int32_t initValue);
void addDoubleToSchema(documentSchema* schema, char* key, double initValue);
void addBooleanToSchema(documentSchema* schema, char* key, uint8_t initValue);
void addTextToSchema(documentSchema* schema, char* key, char* initValue);

#endif
