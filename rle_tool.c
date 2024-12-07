#include <stdio.h>
#include <string.h>

void compress(FILE *input, FILE *output) {
    char currentChar, prevChar;
    int count = 1;
    
    if ((prevChar = fgetc(input)) != EOF) {
        while ((currentChar = fgetc(input)) != EOF) {
            if (currentChar == prevChar) {
                count++;
            } else {
                fprintf(output, "%c%d", prevChar, count);
                prevChar = currentChar;
                count = 1;
            }
        }
        // Write the last character and count
        fprintf(output, "%c%d", prevChar, count);
    }
}

void decompress(FILE *input, FILE *output) {
    char ch;
    int count;
    
    while (fscanf(input, "%c%d", &ch, &count) != EOF) {
        for (int i = 0; i < count; i++) {
            fputc(ch, output);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <mode> <input file> <output file>\n", argv[0]);
        return 1;
    }

    FILE *inputFile = fopen(argv[2], "r");
    if (inputFile == NULL) {
        perror("Error opening input file");
        return 1;
    }

    FILE *outputFile = fopen(argv[3], "w");
    if (outputFile == NULL) {
        perror("Error opening output file");
        fclose(inputFile);
        return 1;
    }

    if (strcmp(argv[1], "compress") == 0) {
        compress(inputFile, outputFile);
    } else if (strcmp(argv[1], "decompress") == 0) {
        decompress(inputFile, outputFile);
    } else {
        printf("Invalid mode. Use 'compress' or 'decompress'.\n");
    }

    fclose(inputFile);
    fclose(outputFile);
    return 0;
}