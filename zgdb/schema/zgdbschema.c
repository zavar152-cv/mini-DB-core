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
    free(schema->elements);
}

void addIntToSchema(documentSchema* schema, char* key, int32_t initValue) {
    schema->elements[schema->size].type = TYPE_INT;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].integerValue = initValue;
    schema->size++;
    schema->sizeOfElements += sizeof(uint8_t) + 13 * sizeof(char) + sizeof(int32_t);
}

void addDoubleToSchema(documentSchema* schema, char* key, double initValue) {
    schema->elements[schema->size].type = TYPE_DOUBLE;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].doubleValue = initValue;
    schema->size++;
    schema->sizeOfElements += sizeof(uint8_t) + 13 * sizeof(char) + sizeof(double);
}

void addBooleanToSchema(documentSchema* schema, char* key, uint8_t initValue) {
    schema->elements[schema->size].type = TYPE_BOOLEAN;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].booleanValue = initValue;
    schema->size++;
    schema->sizeOfElements += sizeof(uint8_t) + 13 * sizeof(char) + sizeof(uint8_t);
}

void addTextToSchema(documentSchema* schema, char* key, char* initValue) {
    schema->reqToast = true;
    schema->elements[schema->size].type = TYPE_TEXT;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].textValue.size = strlen(initValue) + 1;
    schema->elements[schema->size].textValue.data = malloc(schema->elements[schema->size].textValue.size);
    strcpy(schema->elements[schema->size].textValue.data, initValue);
    schema->size++;
    schema->sizeOfElements += sizeof(uint8_t) + 13 * sizeof(char) + sizeof(firstTextChunk);
    schema->minToastCapacity += 1;
}