/* gallagher_util.h - Utilities for parsing Gallagher cards (New Zealand).
 * Author: Nick Mooney (nick@mooney.nz)
 * 
 * Reference: https://github.com/megabug/gallagher-research
*/

#pragma once

#include <stdint.h>
#include <lib/nfc/protocols/mf_classic/mf_classic.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The Gallagher obfuscation algorithm is a 256-byte substitution table. The below array is generated from
 * https://github.com/megabug/gallagher-research/blob/master/formats/cardholder/substitution-table.bin.
*/
const unsigned char GALLAGHER_DECODE_TABLE[256] = {
    '/',    'n',    '\xdd', '\xdf', '\x1d', '\x0f', '\xb0', 'v',    '\xad', '\xaf', '\x7f', '\xbb',
    'w',    '\x85', '\x11', 'm',    '\xf4', '\xd2', '\x84', 'B',    '\xeb', '\xf7', '4',    'U',
    'J',    ':',    '\x10', 'q',    '\xe7', '\xa1', 'b',    '\x1a', '>',    'L',    '\x14', '\xd3',
    '^',    '\xb2', '}',    'V',    '\xbc', '\'',   '\x82', '`',    '\xe3', '\xae', '\x1f', '\x9b',
    '\xaa', '+',    '\x95', 'I',    's',    '\xe1', '\x92', 'y',    '\x91', '8',    'l',    '\x19',
    '\x0e', '\xa9', '\xe2', '\x8d', 'f',    '\xc7', 'Z',    '\xf5', '\x1c', '\x80', '\x99', '\xbe',
    'N',    'A',    '\xf0', '\xe8', '\xa6', ' ',    '\xab', '\x87', '\xc8', '\x1e', '\xa0', 'Y',
    '{',    '\x0c', '\xc3', '<',    'a',    '\xcc', '@',    '\x9e', '\x06', 'R',    '\x1b', '2',
    '\x8c', '\x12', '\x93', '\xbf', '\xef', ';',    '%',    '\r',   '\xc2', '\x88', '\xd1', '\xe0',
    '\x07', '-',    'p',    '\xc6', ')',    'j',    'M',    'G',    '&',    '\xa3', '\xe4', '\x8b',
    '\xf6', '\x97', ',',    ']',    '=',    '\xd7', '\x96', '(',    '\x02', '\x08', '0',    '\xa7',
    '"',    '\xc9', 'e',    '\xf8', '\xb7', '\xb4', '\x8a', '\xca', '\xb9', '\xf2', '\xd0', '\x17',
    '\xff', 'F',    '\xfb', '\x9a', '\xba', '\x8f', '\xb6', 'i',    'h',    '\x8e', '!',    'o',
    '\xc4', '\xcb', '\xb3', '\xce', 'Q',    '\xd4', '\x81', '\x00', '.',    '\x9c', 't',    'c',
    'E',    '\xd9', '\x16', '5',    '_',    '\xed', 'x',    '\x9f', '\x01', 'H',    '\x04', '\xc1',
    '3',    '\xd6', 'O',    '\x94', '\xde', '1',    '\x9d', '\n',   '\xac', '\x18', 'K',    '\xcd',
    '\x98', '\xb8', '7',    '\xa2', '\x83', '\xec', '\x03', '\xd8', '\xda', '\xe5', 'z',    'k',
    'S',    '\xd5', '\x15', '\xa4', 'C',    '\xe9', '\x90', 'g',    'X',    '\xc0', '\xa5', '\xfa',
    '*',    '\xb1', 'u',    'P',    '9',    '\\',   '\xe6', '\xdc', '\x89', '\xfc', '\xcf', '\xfe',
    '\xf9', 'W',    'T',    'd',    '\xa8', '\xee', '#',    '\x0b', '\xf1', '\xea', '\xfd', '\xdb',
    '\xbd', '\t',   '\xb5', '[',    '\x05', '\x86', '\x13', '\xf3', '$',    '\xc5', '?',    'D',
    'r',    '|',    '~',    '6'};

// The second block of a Gallagher credential sector is the literal
// "www.cardax.com  " (note two padding spaces)
const uint8_t GALLAGHER_CARDAX_ASCII[MF_CLASSIC_BLOCK_SIZE] =
    {'w', 'w', 'w', '.', 'c', 'a', 'r', 'd', 'a', 'x', '.', 'c', 'o', 'm', ' ', ' '};
const uint8_t GALLAGHER_CREDENTIAL_SECTOR = 15;

typedef struct GallagherCredential {
    uint8_t region;
    uint8_t issue;
    uint16_t facility;
    uint32_t card;
} GallagherCredential;

void gallagher_deobfuscate_and_parse_credential(
    GallagherCredential* credential,
    const uint8_t* cardholder_data_obfuscated);

#ifdef __cplusplus
}
#endif