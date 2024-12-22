#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function declarations
int compress_file(const char *input_filename, const char *output_filename);
int decompress_file(const char *input_filename, const char *output_filename);
int run_tests(const char *test_file, int is_compression);

int compress_file(const char *input_filename, const char *output_filename)
{
    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL)
    {
        printf("Error: Unable to open input file\n");
        return 0;
    }

    fseek(input_file, 0, SEEK_END);
    if (ftell(input_file) == 0)
    {
        printf("Error: Input file is empty\n");
        fclose(input_file);
        return 0;
    }
    rewind(input_file);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL)
    {
        printf("Error: Unable to open output file\n");
        fclose(input_file);
        return 0;
    }

    char current_char, prev_char = '\0';
    int count = 1;
    int compressed_length = 0;

    // Loop through each character in the file
    while ((current_char = fgetc(input_file)) != EOF)
    {
        // Skip newlines and ensure only valid characters are processed
        if (current_char == '\n')
        {
            if (prev_char != '\0')  // If there was any previous character, output its count
            {
                fprintf(output_file, "%c %d", prev_char, count);
                compressed_length += 2 + (count > 9 ? (count > 99 ? 3 : 2) : 1);
                prev_char = '\0';  // Reset prev_char to start fresh for the next line
                count = 0;
            }
            fprintf(output_file, "\n"); // Move to the next line
        }
        else if (current_char != ' ')  // Skip spaces, as per the output requirement
        {
            if (current_char == prev_char)
            {
                count++;
            }
            else
            {
                // Output previous character and its count
                if (prev_char != '\0')
                {
                    fprintf(output_file, "%c %d;", prev_char, count);
                    compressed_length += 2 + (count > 9 ? (count > 99 ? 3 : 2) : 1);
                }
                prev_char = current_char;
                count = 1;
            }
        }
    }

    // Write the last character if any
    if (prev_char != '\0')
    {
        fprintf(output_file, "%c %d", prev_char, count);
        compressed_length += 2 + (count > 9 ? (count > 99 ? 3 : 2) : 1);
    }

    fclose(input_file);
    fclose(output_file);
    return compressed_length;
}

int decompress_file(const char *input_filename, const char *output_filename)
{
    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL)
    {
        printf("Error: Unable to open input file\n");
        return 0;
    }

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL)
    {
        printf("Error: Unable to open output file\n");
        fclose(input_file);
        return 0;
    }

    char line[1024];  // Buffer to hold each line from the input file

    // Read each line from the input file
    while (fgets(line, sizeof(line), input_file) != NULL)
    {
        char *ptr = line;
        while (*ptr != '\0')
        {
            char current_char;
            int count;

            // Read the character
            if (sscanf(ptr, "%c %d", &current_char, &count) == 2)
            {
                // Write the character to the output file 'count' times
                for (int i = 0; i < count; i++)
                {
                    fputc(current_char, output_file);
                }

                // Move pointer past the character-count pair and semicolon
                while (*ptr != ';' && *ptr != '\0') 
                {
                    ptr++;
                }

                // Skip past the semicolon
                if (*ptr == ';')
                {
                    ptr++;
                }
            }
            else
            {
                // If we encounter invalid input, break out of the loop
                break;
            }
        }

        // Write a newline after processing each line
        fputc('\n', output_file);
    }

    fclose(input_file);
    fclose(output_file);
    return 1;
}

int run_tests(const char *test_file, int is_compression)
{
    FILE *fp = fopen(test_file, "r");
    if (!fp)
    {
        printf("Error: Cannot open test file\n");
        return 0;
    }

    int MAX_LINE_LENGTH = 1023;
    char line[MAX_LINE_LENGTH];
    char input[MAX_LINE_LENGTH], expected[MAX_LINE_LENGTH];
    char temp_input1[] = "temp_input1.txt";
    char temp_input2[] = "temp_input2.txt";
    char temp_out1[] = "temp_output1.txt";
    char temp_out2[] = "temp_output2.txt";
    int test_count = 0;
    int passed = 0;

    while (fgets(line, sizeof(line), fp))
    {
        if (line[0] == '\0' || strspn(line, " \t\n") == strlen(line))
        {
            continue;
        }

        // Split the line into input and expected output based on the comma
        if (sscanf(line, "%[^,], %[^\n]", input, expected) != 2)
        {
            printf("Error parsing test case %d: Invalid format\n", test_count + 1);
            continue;
        }

        // Create the temporary input file with the input string
        FILE *temp_in1 = fopen(temp_input1, "w");
        if (!temp_in1)
        {
            printf("Error creating temporary input file\n");
            fclose(fp);
            return 0;
        }
        fprintf(temp_in1, "%s", input);
        fclose(temp_in1);

        // Create the temporary input file with the expected output string
        FILE *temp_in2 = fopen(temp_input2, "w");
        if (!temp_in2)
        {
            printf("Error creating temporary input file\n");
            fclose(fp);
            return 0;
        }
        fprintf(temp_in2, "%s", expected);
        fclose(temp_in2);

        int success;
        if (is_compression)
        {
            // Run the compression function
            success = compress_file(temp_input1, temp_out1);
            // Only check the output of compression
            if (success > 0)
            {
                FILE *out1 = fopen(temp_out1, "r");
                if (!out1)
                {
                    printf("Error opening temporary out1 file\n");
                    fclose(fp);
                    return 0;
                }

                char actual1[MAX_LINE_LENGTH];
                if (!fgets(actual1, sizeof(actual1), out1))
                {
                    printf("Error reading output for test case %d\n", test_count + 1);
                    fclose(out1);
                    continue;
                }
                fclose(out1);
                actual1[strcspn(actual1, "\n")] = '\0';

                if (strcmp(actual1, expected) == 0)
                {
                    passed++;
                    printf("Test case %d: PASSED\n", test_count + 1);
                }
                else
                {
                    printf("Test case %d: FAILED\n", test_count + 1);
                    printf("Input: %s\n", input);
                    printf("Expected: %s\n", expected);
                    printf("Actual: %s\n", actual1);
                }
            }
            else
            {
                printf("Test case %d: FAILED\n", test_count + 1);
                printf("Error during compression\n");
            }
        }
        else
        {
            // Run the decompression function (only if it's not compression)
            success = decompress_file(temp_input2, temp_out2);
            // Only check the output of decompression
            if (success > 0)
            {
                FILE *out2 = fopen(temp_out2, "r");
                if (!out2)
                {
                    printf("Error opening temporary out2 file\n");
                    fclose(fp);
                    return 0;
                }

                char actual2[MAX_LINE_LENGTH];
                if (!fgets(actual2, sizeof(actual2), out2))
                {
                    printf("Error reading output for test case %d\n", test_count + 1);
                    fclose(out2);
                    continue;
                }
                fclose(out2);
                actual2[strcspn(actual2, "\n")] = '\0';

                if (strcmp(actual2, input) == 0)
                {
                    passed++;
                    printf("Test case %d: PASSED\n", test_count + 1);
                }
                else
                {
                    printf("Test case %d: FAILED\n", test_count + 1);
                    printf("Input: %s\n", input);
                    printf("Expected: %s\n", expected);
                    printf("Actual: %s\n", actual2);
                }
            }
            else
            {
                printf("Test case %d: FAILED\n", test_count + 1);
                printf("Error during decompression\n");
            }
        }

        test_count++;
    }

    fclose(fp);
    remove(temp_input1);
    remove(temp_input2);
    remove(temp_out1);
    remove(temp_out2);

    printf("\nTest Results: %d/%d passed\n", passed, test_count);
    return passed == test_count;
}

int main(int argc, char *argv[])
{

    if (strcmp(argv[1], "--run-code") == 0)
    {
        if (argc == 5)
        {
            if (strcmp(argv[2], "--compress") == 0)
            {
                int compressed_length = compress_file(argv[3], argv[4]);
                if (compressed_length > 0)
                {
                    printf("Compression successful\n");
                    printf("Compressed length: %d\n", compressed_length);
                }
                else
                    printf("Error in compression process.\n");
            }
            else if (strcmp(argv[2], "--decompress") == 0)
            {
                int decompressed_length = decompress_file(argv[3], argv[4]);
                if (decompressed_length > 0)
                {
                    printf("Decompression successful\n");
                    printf("Decompressed length: %d\n", decompressed_length);
                }
                else
                    printf("Error in decompression process.\n");
            }
            else
            {
                printf("compress_or_decompress must be: --compress, --decompress\n");
                return 1;
            }
        }
        else
        {
            printf("Invalid arguments for run_code");
        }
    }
    else if (strcmp(argv[1], "--run-test") == 0){
        if (argc == 4)
        {
            if (strcmp(argv[2], "--compress") == 0)
            {
                return run_tests(argv[3], 1) ? 0 : 1;
            }
            else if (strcmp(argv[2], "--decompress") == 0)
            {
                return run_tests(argv[3], 0) ? 0 : 1;
            }
            else
            {
                printf("Usage: %s <mode> input_file output_file\n", argv[0]);
                printf("Modes: --compress, --decompress \n");
                return 1;
            }
        }
        else
        {
            printf("Usage: %s <run_or_test> <compress_or_decompress> input_file output_file\n", argv[0]);
            printf("run_or_test: --run-code, --run-test\n");
            return 1;
        }
    }
    else
    {
        printf("Invalid arguments for run_code");
        return 1;
    }
    return 0;
}
