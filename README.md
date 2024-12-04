# RLE Compression Tool

## Project Overview
This project implements a Run-Length Encoding (RLE) compression tool in C. The tool compresses and decompresses text files to efficiently manage storage and transmission. 

- **Compression**: Replaces consecutive repeated characters with the character followed by its count (e.g., `aaaabbbccdaa` ? `a4b3c2d1a2`).
- **Decompression**: Reconstructs the original text from the compressed representation.
- **Purpose**: To demonstrate practical usage of RLE in reducing data size in a computationally efficient manner.

## Solution Description

### Approach
- **Compression**:
  1. Traverse the input string once.
  2. Count consecutive repeating characters.
  3. Append the character and its count to the compressed string.

- **Decompression**:
  1. Read the compressed string.
  2. Parse each character and its count.
  3. Reconstruct the original string by expanding each character according to its count.

### Time Complexity
- **Compression**: O(n), where `n` is the length of the input string. Each character is processed once.
- **Decompression**: O(m), where `m` is the length of the compressed string. Each character and its count are processed once.

### Space Complexity
- **Compression**: O(n) in the worst case, where no characters repeat (e.g., `abcde` ? `a1b1c1d1e1`).
- **Decompression**: O(k), where `k` is the size of the original string.

### Why This Solution is Optimal
- The linear time complexity ensures scalability for larger inputs.
- Memory usage is minimal, as the algorithm operates directly on strings without additional data structures.
- RLE is a well-suited solution for data with repeated patterns, making this implementation effective for real-world use cases like early image formats (BMP, TIFF).

## How to Clone and Run the Project

### Prerequisites
- A C compiler (e.g., GCC).
- Git installed on your machine.

### Steps to Clone and Compile
1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/rle-compression-tool.git
   cd rle-compression-tool

## Compile Code
gcc -o rle_tool rle_tool.c

## Running the Program
./rle_tool compress input.txt compressed.rle


##Decompression Mode
./rle_tool decompress compressed.rle output.txt