#include <stdint.h>

// Structure to represent a Code 39 character and its binary pattern
typedef struct {
    char character;        // The Code 39 character
    uint16_t pattern;      // Binary representation of the pattern
} Code39Char;

// Array of Code 39 characters and their binary patterns
const Code39Char CODE_39_CHARACTERS[] = {
    {'0', 0xA6D}, // "101001101101"
    {'1', 0xD4B}, // "110100101011"
    {'2', 0xB2B}, // "101100101011"
    {'3', 0xD2B}, // "110110010101"
    {'4', 0xA5B}, // "101001101011"
    {'5', 0xD5B}, // "110100110101"
    {'6', 0xB5B}, // "101100110101"
    {'7', 0xAB3}, // "101001011011"
    {'8', 0xDB3}, // "110100101101"
    {'9', 0xB3B}, // "101100101101"
    {'A', 0xD14}, // "110101001011"
    {'B', 0xB16}, // "101101001011"
    {'C', 0xD34}, // "110110100101"
    {'D', 0xA2A}, // "101011001011"
    {'E', 0xD2A}, // "110101100101"
    {'F', 0xB2A}, // "101101100101"
    {'G', 0xA0B}, // "101010011011"
    {'H', 0xD0B}, // "110101001101"
    {'I', 0xB0B}, // "101101001101"
    {'J', 0xA2B}, // "101011001101"
    {'K', 0xD2C}, // "110101010011"
    {'L', 0xB2C}, // "101101010011"
    {'M', 0xD5A}, // "110110101001"
    {'N', 0xA5A}, // "101011010011"
    {'O', 0xD5A}, // "110101101001"
    {'P', 0xB5A}, // "101101101001"
    {'Q', 0xA59}, // "101010110011"
    {'R', 0xD59}, // "110101011001"
    {'S', 0xB59}, // "101101011001"
    {'T', 0xA5B}, // "101011011001"
    {'U', 0x4A6}, // "110010101011"
    {'V', 0x4B2}, // "100110101011"
    {'W', 0x4D5}, // "110011010101"
    {'X', 0x4A5}, // "100101101011"
    {'Y', 0x4B5}, // "110010110101"
    {'Z', 0x4D5}, // "100110110101"
};

// Number of Code 39 characters
#define CODE_39_CHAR_COUNT (sizeof(CODE_39_CHARACTERS) / sizeof(Code39Char))
