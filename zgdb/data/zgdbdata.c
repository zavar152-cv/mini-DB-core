#include "zgdbdata.h"

bool isRootDocument(document document) {
    return document.isRoot;
}

bool hasChild(document document) {
    return document.header.indexSon != 0;
}

bool hasBrother(document document) {
    return document.header.indexBrother != 0;
}