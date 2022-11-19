#include "zgdbdata.h"

bool isRootDocument(document document) {
    return strcmp(document.header.name, "root") == 0 && document.header.indexAttached == 0 && document.header.attrCount == 0 && document.header.indexBrother == 0;
}

bool hasChild(document document) {
    return document.header.indexSon != 0;
}