#include <stdlib.h>
#include <string.h>
#include "zgdbschema.h"

documentSchema initSchema(size_t capacity) {
    documentSchema schema;
    schema.capacity = capacity;
    schema.size = 0;
    schema.elements = (element*) malloc(capacity * sizeof(element));
    schema.reqToast = false;
    schema.sizeOfElements = 0;
    schema.minToastCapacity = 0;
    return schema;
}

void destroySchema(documentSchema* schema) {
    for (int i = 0; i < schema->size; ++i) {
        if (schema->elements[i].type == TYPE_TEXT)
            free(schema->elements[i].textValue.data);
    }
    free(schema->elements);
}

addStatus addIntToSchema(documentSchema* schema, char* key, int32_t initValue) {
    for (int i = 0; i < schema->size; ++i) {
        if(strcmp(schema->elements[i].key, key) == 0)
            return NAME_EXISTS;
    }
    if (schema->size == schema->capacity)
        return SCHEMA_OVERFLOW;
    schema->elements[schema->size].type = TYPE_INT;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].integerValue = initValue;
    schema->size++;
    schema->sizeOfElements += sizeof(uint8_t) + 13 * sizeof(char) + sizeof(int32_t);
    return ADD_OK;
}

addStatus addDoubleToSchema(documentSchema* schema, char* key, double initValue) {
    for (int i = 0; i < schema->size; ++i) {
        if(strcmp(schema->elements[i].key, key) == 0)
            return NAME_EXISTS;
    }
    if (schema->size == schema->capacity)
        return SCHEMA_OVERFLOW;
    schema->elements[schema->size].type = TYPE_DOUBLE;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].doubleValue = initValue;
    schema->size++;
    schema->sizeOfElements += sizeof(uint8_t) + 13 * sizeof(char) + sizeof(double);
    return ADD_OK;
}

addStatus addBooleanToSchema(documentSchema* schema, char* key, uint8_t initValue) {
    for (int i = 0; i < schema->size; ++i) {
        if(strcmp(schema->elements[i].key, key) == 0)
            return NAME_EXISTS;
    }
    if (schema->size == schema->capacity)
        return SCHEMA_OVERFLOW;
    schema->elements[schema->size].type = TYPE_BOOLEAN;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].booleanValue = initValue;
    schema->size++;
    schema->sizeOfElements += sizeof(uint8_t) + 13 * sizeof(char) + sizeof(uint8_t);
    return ADD_OK;
}

addStatus addTextToSchema(documentSchema* schema, char* key, char* initValue) {
    for (int i = 0; i < schema->size; ++i) {
        if(strcmp(schema->elements[i].key, key) == 0)
            return NAME_EXISTS;
    }
    if (schema->size == schema->capacity)
        return SCHEMA_OVERFLOW;
    schema->reqToast = true;
    schema->elements[schema->size].type = TYPE_TEXT;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].textValue.size = strlen(initValue) + 1;
    schema->elements[schema->size].textValue.data = malloc(schema->elements[schema->size].textValue.size);
    strcpy(schema->elements[schema->size].textValue.data, initValue);
    div_t divRes = div((int) schema->elements[schema->size].textValue.size, CHUNK_SIZE);
    int chunks = divRes.quot;
    if(divRes.rem != 0)
        chunks++;
    printf("Chunks schema: %d\n", chunks);
    schema->minToastCapacity += chunks;
    schema->size++;
    schema->sizeOfElements += sizeof(uint8_t) + 13 * sizeof(char) + sizeof(firstTextChunk);
    return ADD_OK;
}