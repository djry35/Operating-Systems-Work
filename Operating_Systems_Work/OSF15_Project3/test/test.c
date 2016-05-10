#include <stdio.h>
#include <dyn_array.h>
#include "../src/shelp.c"

int main(int argc, char **argv) {
    ls(NULL);
    ls("test.c");
    ls("../../");
    cat("test.c");
    dyn_array_t *strings = tokenizer("bob,sue,fred", ",");
    size_t end = dyn_array_size(strings);
    for (size_t i = 0; i < end; ++i) {
        printf("%s\n", *((char **) dyn_array_at(strings, i)));
    }
    dyn_array_destroy(strings);
    return 0;
}
