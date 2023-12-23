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

extern const uint8_t GALLAGHER_ENCODE_TABLE[256];
extern const uint8_t GALLAGHER_DECODE_TABLE[256];
extern const uint8_t GALLAGHER_CARDAX_ASCII[MF_CLASSIC_BLOCK_SIZE];
extern const uint8_t GALLAGHER_CREDENTIAL_SECTOR;

#define GALLAGHER_CREDENTIAL_FIELDS_LIST     \
    X(region, Region, region code, 8)        \
    X(issue, Issue, issue level, 8)          \
    X(facility, Facility, facility code, 16) \
    X(card, Card, card number, 32)

typedef struct GallagherCredential {
#define X(name, enumName, englishName, size) uint##size##_t name;
    GALLAGHER_CREDENTIAL_FIELDS_LIST
#undef X
} GallagherCredential;

#define X(name, enumName, englishName, size) enumName,
enum GallagherCredentialFieldToSet { GALLAGHER_CREDENTIAL_FIELDS_LIST };
#undef X

void gallagher_deobfuscate_and_parse_credential(
    GallagherCredential* credential,
    const uint8_t* cardholder_data_obfuscated);

void gallagher_obfuscate_credential(uint8_t* destination, GallagherCredential credential);

#ifdef __cplusplus
}
#endif