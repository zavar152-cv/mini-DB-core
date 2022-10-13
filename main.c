#include <stdio.h>

typedef struct Index {
    long long id;
    long offset;
} Index;

int main() {
    FILE *file;
    char shelf1[] = "kek";
    char shelf2[] = "hello";
    int a = 46;
    file = fopen("/tmp/bintest", "w+b");
//    fwrite(shelf2, sizeof(char), 5, file);
//    fseek(file, 2, SEEK_SET);
//    fwrite(shelf1, sizeof(char), 3, file);
    fseek(file, 0, SEEK_SET);
    fwrite(&a, sizeof(int), 1, file);
    fseek(file, sizeof(int), SEEK_SET);
    a = 19;
    fwrite(&a, sizeof(int), 1, file);

    int b = 0;
    fseek(file, 0, SEEK_SET);
    fread(&b, sizeof(int), 1, file);
    printf("%d\n", b);
    b = 0;
    fseek(file, sizeof(int), SEEK_SET);
    fread(&b, sizeof(int), 1, file);
    printf("%d\n", b);

    Index index;
    index.id = 1;
    index.offset = 4;

    fseek(file, sizeof(int)*2, SEEK_SET);
    fwrite(&index, sizeof(Index), 1, file);

    Index tIndex;
    fseek(file, sizeof(int)*2, SEEK_SET);
    fread(&tIndex, sizeof(Index), 1, file);
    printf("%lld\n", tIndex.id);
    printf("%ld\n", tIndex.offset);
    fclose(file);
}
