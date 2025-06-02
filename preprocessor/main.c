#include "preprocessor.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    assert(argc == 2);
    // Check if the correct number of arguments is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Open the file for reading
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Read and process the file line by line
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Process each line (for example, print it)
        printf("%s", line);
    }

    // Close the file
    fclose(file);
    return 0;
}


