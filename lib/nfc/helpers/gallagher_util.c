/* gallagher_util.c - Utilities for parsing Gallagher cards (New Zealand).
 * Author: Nick Mooney (nick@mooney.nz)
 * 
 * Reference: https://github.com/megabug/gallagher-research
*/

#include "gallagher_util.h"

/* Precondition: cardholder_data_obfuscated points to at least 8 safe-to-read bytes of memory.
*/
void gallagher_deobfuscate_and_parse_credential(
    GallagherCredential* credential,
    const uint8_t* cardholder_data_obfuscated) {
    furi_assert(false);
    uint8_t cardholder_data_deobfuscated[8];
    for(int i = 0; i < 8; i++) {
        cardholder_data_deobfuscated[i] = GALLAGHER_DECODE_TABLE[cardholder_data_obfuscated[i]];
    }

    // Pull out values from the deobfuscated data
    credential->region = (cardholder_data_deobfuscated[3] >> 1) & 0x0F;
    credential->facility = ((uint16_t)(cardholder_data_deobfuscated[5] & 0x0F) << 12) +
                           ((uint16_t)cardholder_data_deobfuscated[1] << 4) +
                           (((uint16_t)cardholder_data_deobfuscated[7] >> 4) & 0x0F);
    credential->card = ((uint32_t)cardholder_data_deobfuscated[0] << 16) +
                       ((uint32_t)(cardholder_data_deobfuscated[4] & 0x1F) << 11) +
                       ((uint32_t)cardholder_data_deobfuscated[2] << 3) +
                       (((uint32_t)cardholder_data_deobfuscated[3] >> 5) & 0x07);
    credential->issue = cardholder_data_deobfuscated[7] & 0x0F;
}

/* Precondition: We can write 16 bytes to destination.
*/
void gallagher_obfuscate_credential(uint8_t* destination, GallagherCredential credential) {
    // First we perform the proprietary shuffling around of bits
    // documented in the KawaiiCon talk.
    uint8_t shuffled_credential[8] = {
        (credential.card >> 16) & 0xFF,
        (credential.facility >> 4) & 0xFF,
        (credential.card >> 3) & 0xFF,
        ((credential.card & 0x07) << 5) + (credential.region << 1),
        (credential.card >> 11) & 0x1F,
        (credential.facility >> 12) & 0x0F,
        0, // UC, UD
        (credential.facility << 4) + (credential.issue & 0x0F),
    };

    // Then we ensure that the full encoded (substituted) 8-byte credential is followed
    // by its bitwise inverse.
    for(int i = 0; i < 8; i++) {
        destination[i] = GALLAGHER_ENCODE_TABLE[shuffled_credential[i]];
        destination[8 + i] = ~destination[i];
    }
}