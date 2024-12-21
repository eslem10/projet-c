#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// Function declarations
int compress_file(const char *input_filename, const char *output_filename);
int decompress_file(const char *input_filename, const char *output_filename);
int validate_line(const char *line);
int run_tests(const char *test_file, int is_compression);

// Validation function
int validate_line(const char *line) {
    const char *ptr = line;
    
    while (*ptr) {
        while (*ptr == ' ') ptr++;
        if (!*ptr || *ptr == '\n') break;
        
        if (!isprint(*ptr)) return 0;
        ptr++;
        
        if (*ptr != ' ') return 0;
        ptr++;
        
        // Modified to handle multi-digit numbers
        if (!isdigit(*ptr)) return 0;
        while (isdigit(*ptr)) ptr++;
        
        if (*ptr && *ptr != ' ' && *ptr != '\n') return 0;
    }
    return 1;
}

// Compression function
int compress_file(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error opening input file\n");
        return 0;
    }

    fseek(input_file, 0, SEEK_END);
    if (ftell(input_file) == 0) {
        printf("Error: Input file is empty\n");
        fclose(input_file);
        return 0;
    }
    rewind(input_file);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        printf("Error opening output file\n");
        fclose(input_file);
        return 0;
    }

    char current_char, prev_char;
    int count = 1;
    int first_in_line = 1;
    int compressed_length = 0;

    prev_char = fgetc(input_file);
    if (prev_char == EOF) {
        fclose(input_file);
        fclose(output_file);
        return 0;
    }
    if (prev_char == ' ') {
        printf("Error: Input contains spaces\n");
        fclose(input_file);
        fclose(output_file);
        return 0;
    }

    while ((current_char = fgetc(input_file)) != EOF) {
        if (current_char == ' ') {
            printf("Error: Input contains spaces\n");
            fclose(input_file);
            fclose(output_file);
            return 0;
        }

        if (current_char == prev_char && current_char != ',') {
            count++;
        } else {
            if (prev_char != '\n') {
                if (!first_in_line) {
                    fprintf(output_file, " ");
                }
                fprintf(output_file, "%c %d", prev_char, count);
                compressed_length += 2;
                first_in_line = 0;
            }
            if (current_char == ',') {
                fprintf(output_file, "\n");
                first_in_line = 1;
            }
            count = 1;
            prev_char = current_char;
        }
    }

    if (prev_char != EOF && prev_char != '\n') {
        if (!first_in_line) {
            fprintf(output_file, " ");
        }
        fprintf(output_file, "%c %d", prev_char, count);
        compressed_length += 2;
    }

    fclose(input_file);
    fclose(output_file);
    return compressed_length;
}

// Decompression function with improved line handling
int decompress_file(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error opening input file\n");
        return 0;
    }

    fseek(input_file, 0, SEEK_END);
    if (ftell(input_file) == 0) {
        printf("Error: Input file is empty\n");
        fclose(input_file);
        return 0;
    }
    rewind(input_file);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        printf("Error opening output file\n");
        fclose(input_file);
        return 0;
    }

    char line[1024];
    char current_char;
    int count;
    int decompressed_length = 0;
    int last_char_newline = 1;

    while (fgets(line, sizeof(line), input_file)) {
        if (!validate_line(line)) {
            printf("Invalid format in compressed file\n");
            fclose(input_file);
            fclose(output_file);
            return 0;
        }

        char *ptr = line;
        int is_empty_line = 1;

        while (*ptr && sscanf(ptr, " %c %d", &current_char, &count) == 2) {
            is_empty_line = 0;
            for (int i = 0; i < count; i++) {
                fputc(current_char, output_file);
                decompressed_length++;
            }
            
            ptr += 2;
            while (*ptr && isdigit(*ptr)) ptr++;
            while (*ptr && *ptr == ' ') ptr++;
        }
        
        if (!is_empty_line || !last_char_newline) {
            fputc('\n', output_file);
            last_char_newline = 1;
        }
    }

    fclose(input_file);
    fclose(output_file);
    return decompressed_length;
}

// Testing function
int run_tests(const char *test_file, int is_compression) {
    FILE *fp = fopen(test_file, "r");
    if (!fp) {
        printf("Error: Cannot open test file\n");
        return 0;
    }
    int MAX_LINE_LENGTH=1023;
    char line[MAX_LINE_LENGTH];
    char input[MAX_LINE_LENGTH], expected[MAX_LINE_LENGTH];
    char temp_input[] = "temp_input.txt";
    char temp_output[] = "temp_output.txt";
    int test_count = 0;
    int passed = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\0') {
            continue;
        }

        // Parse the input and expected result from the line
        if (sscanf(line, "%[^,] %[^\n]", input, expected) != 2) {
            printf("Error parsing test case %d: Invalid format\n", test_count + 1);
            continue;
        }

        // Write the input to a temporary file
        FILE *temp_in = fopen(temp_input, "w");
        if (!temp_in) {
            printf("Error creating temporary input file\n");
            fclose(fp);
            return 0;
        }
        fprintf(temp_in, "%s", input);
        fclose(temp_in);

        // Perform compression or decompression based on the flag
        int success;
        if (is_compression) {
            success = compress_file(temp_input, temp_output);
        } else {
            success = decompress_file(temp_input, temp_output);
        }

        if (success > 0) {
            // Compare the output with the expected result
            FILE *out = fopen(temp_output, "r");
            if (!out) {
                printf("Error opening temporary output file\n");
                fclose(fp);
                return 0;
            }

            char actual[MAX_LINE_LENGTH];
            if (!fgets(actual, sizeof(actual), out)) {
                printf("Error reading output for test case %d\n", test_count + 1);
                fclose(out);
                continue;
            }
            fclose(out);

            // Remove trailing newline from actual output
            actual[strcspn(actual, "\n")] = '\0';

            if (strcmp(actual, expected) == 0) {
                passed++;
                printf("Test case %d: PASSED\n", test_count + 1);
            } else {
                printf("Test case %d: FAILED\n", test_count + 1);
                printf("Input: %s\n", input);
                printf("Expected: %s\n", expected);
                printf("Actual: %s\n", actual);
            }
        } else {
            printf("Test case %d: FAILED\n", test_count + 1);
            printf("Error during processing\n");
        }

        test_count++;
    }

    fclose(fp);
    remove(temp_input);
    remove(temp_output);

    printf("\nTest Results: %d/%d passed\n", passed, test_count);
    return passed == test_count;
}



int main(int argc, char *argv[]) {
    if (argc == 3) {
        if (strcmp(argv[1], "-test-compress") == 0) {
            return run_tests(argv[2], 1) ? 0 : 1;
        }
        if (strcmp(argv[1], "-test-decompress") == 0) {
            return run_tests(argv[2], 0) ? 0 : 1;
        }
    }

    if (argc != 4) {
        printf("Usage: %s <mode> input_file output_file\n", argv[0]);
        printf("Modes: -compress, -decompress, -test-compress, -test-decompress\n");
        return 1;
    }

    if (strcmp(argv[1], "-compress") == 0) {
        int compressed_length = compress_file(argv[2], argv[3]);
        if (compressed_length > 0) {
            printf("Compression successful\n");
            printf("Compressed length: %d\n", compressed_length);
        }
        else
            printf("Error in compression process.\n");
    } else if (strcmp(argv[1], "-decompress") == 0) {
        int decompressed_length = decompress_file(argv[2], argv[3]);
        if (decompressed_length > 0) {
            printf("Decompression successful\n");
            printf("Decompressed length: %d\n", decompressed_length);
        }
        else
            printf("Error in decompression process.\n");
    } else {
        printf("Invalid mode. Use -compress, -decompress, -test-compress, or -test-decompress\n");
        return 1;
    }

    return 0;
}
