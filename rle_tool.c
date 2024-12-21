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
        // Skip leading spaces
        while (*ptr == ' ') ptr++;
        if (!*ptr || *ptr == '\n') break;

        // Check for a printable character
        if (!isprint(*ptr)) {
            printf("Error: Non-printable character found\n");
            return 0;
        }
        ptr++;

        // Expect a space
        if (*ptr != ' ') {
            printf("Error: Missing space after character\n");
            return 0;
        }
        ptr++;

        // Check for a digit (multi-digit numbers allowed)
        if (!isdigit(*ptr)) {
            printf("Error: Missing number after character\n");
            return 0;
        }
        while (isdigit(*ptr)) ptr++;

        // Allow only spaces or newlines after a number
        if (*ptr && *ptr != ' ' && *ptr != '\n') {
            printf("Error: Unexpected character found after number\n");
            return 0;
        }
    }

    return 1; // Valid line
}

// Compression function
int compress_file(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Unable to open input file\n");
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
        printf("Error: Unable to open output file\n");
        fclose(input_file);
        return 0;
    }

    char current_char, prev_char = '\0';
    int count = 1;
    int compressed_length = 0;

    // Read the first character
    if ((prev_char = fgetc(input_file)) == EOF || prev_char == '\n') {
        fclose(input_file);
        fclose(output_file);
        return 0; // Nothing to compress
    }

    if (prev_char == ' ') {
        printf("Error: Input contains invalid spaces\n");
        fclose(input_file);
        fclose(output_file);
        return 0;
    }

    while ((current_char = fgetc(input_file)) != EOF) {
        // Validate input (no spaces in the middle of the line)
        if (current_char == ' ') {
            printf("Error: Input contains invalid spaces\n");
            fclose(input_file);
            fclose(output_file);
            return 0;
        }

        if (current_char == prev_char && current_char != '\n') {
            count++;
        } else {
            // Write compressed data for the previous character
            if (prev_char != '\n') {
                if (compressed_length > 0) {
                    fprintf(output_file, " ");
                }
                fprintf(output_file, "%c %d", prev_char, count);
                compressed_length += 2 + (count > 9 ? (count > 99 ? 3 : 2) : 1);
            }

            // Handle newlines
            if (current_char == '\n') {
                fprintf(output_file, "\n");
                compressed_length++;
            }

            prev_char = current_char;
            count = 1;
        }
    }

    // Write the last character
    if (prev_char != '\n') {
        if (compressed_length > 0) {
            fprintf(output_file, " ");
        }
        fprintf(output_file, "%c %d", prev_char, count);
        compressed_length += 2 + (count > 9 ? (count > 99 ? 3 : 2) : 1);
    }

    fclose(input_file);
    fclose(output_file);
    return compressed_length;
}


// Decompression function with improved line handling
int decompress_file(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "rb");
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

    FILE *output_file = fopen(output_filename, "wb");
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

            while (*ptr && (isdigit(*ptr) || *ptr == ' ')) {
                ptr++;
            }
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
    int MAX_LINE_LENGTH = 1023;
    char line[MAX_LINE_LENGTH];
    char input[MAX_LINE_LENGTH], expected[MAX_LINE_LENGTH];
    char temp_input[] = "temp_input.txt";
    char temp_output[] = "temp_output.txt";
    int test_count = 0;
    int passed = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Skip empty or whitespace-only lines
        if (line[0] == '\0' || strspn(line, " \t\n") == strlen(line)) {
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
    printf(argv[1]);
    if (argc == 4) {
        if (strcmp(argv[1], "-testcompress") == 0) {
            return run_tests(argv[2], 1) ? 0 : 1;
        }
        if (strcmp(argv[1], "-testdecompress") == 0) {
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
