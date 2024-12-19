#include <stdio.h>
#include <string.h>
#include <stdbool.h>
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

bool run_test(FILE *file, int test_case) {
    char input1[100]  , expected[100], result[100] ;

    // Read input and the expected result
    if (fscanf(file, "%s %s", input1, expected) !=2){
        if (feof(file)) {
            // End of file reached, no more test cases
            return false;
        }
        printf("Test case %d: Error reading dataset file\n", test_case);
        return false;
    }

    //  our fuctions
    FILE *tempInput = tmpfile();
    FILE *tempCompressed = tmpfile();
    FILE *tempDecompressed = tmpfile();

    if (!tempInput || !tempCompressed || !tempDecompressed) {
        perror("Error creating temporary files");
        return false;
    }

    // Write input1 to tempInput
    fprintf(tempInput, "%s", input1);
    rewind(tempInput);

    // Run compression
    compress(tempInput, tempCompressed);
    rewind(tempCompressed);

    // Run decompression
    decompress(tempCompressed, tempDecompressed);
    rewind(tempDecompressed);

    // Read decompressed result back
    fscanf(tempDecompressed, "%s", result);

    // Compare result with expected
    bool passed = (strcmp(result, expected) == 0);

    printf("Test case %d: %s\n", test_case, passed ? "Passed" : "Failed");

    fclose(tempInput);
    fclose(tempCompressed);
    fclose(tempDecompressed);

    return passed;
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
    FILE *file = fopen("dataset.txt", "r");
    if (file == NULL) {
        perror("Error opening dataset file");
        return 1;
    }

    int test_case = 0;
    int passed = 0, failed = 0;

    printf("Running tests...\n");

    while (!feof(file)) {
        test_case++;
        // Run each test and count passed/failed
        if (!run_test(file, test_case)) {
            // If run_test returns false due to EOF, exit loop
            if (feof(file)) break;
            failed++;
        } else {
            passed++;
        }
    }

    fclose(file);

    printf("\nTotal tests: %d | Passed: %d | Failed: %d\n", passed + failed, passed, failed);

    return (failed == 0) ? 0 : 1; 
}

