#include <stdlib.h>
#include <string.h>
#include "zgdbschema.h"

documentSchema initSchema(size_t capacity) {
    documentSchema schema;
    schema.capacity = capacity;
    schema.size = 0;
    schema.elements = (element*) malloc(capacity * sizeof(element));
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
}

void addDoubleToSchema(documentSchema* schema, char* key, double initValue) {
    schema->elements[schema->size].type = TYPE_DOUBLE;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].doubleValue = initValue;
    schema->size++;
}

void addBooleanToSchema(documentSchema* schema, char* key, uint8_t initValue) {
    schema->elements[schema->size].type = TYPE_BOOLEAN;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].booleanValue = initValue;
    schema->size++;
}

void addTextToSchema(documentSchema* schema, char* key, text initValue) {
    schema->elements[schema->size].type = TYPE_TEXT;
    strcpy(schema->elements[schema->size].key, key);
    schema->elements[schema->size].textValue = initValue;
    schema->size++;
}