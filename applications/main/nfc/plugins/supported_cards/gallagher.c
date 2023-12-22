/* gallagher.c - Parser for Gallagher/Cardax cards (New Zealand).
 * Author: Nick Mooney (nick@mooney.nz)
 * 
 * Reference: https://github.com/megabug/gallagher-research
*/

#include "nfc_supported_card_plugin.h"

#include "protocols/mf_classic/mf_classic.h"
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_application/flipper_application.h>
#include <nfc/helpers/nfc_util.h>
#include <nfc/helpers/gallagher_util.h>

#define TAG "Gallagher"

/* To the best of my knowledge, this is never called (when configured correctly).
 * I can only find codepaths that lead to calling gallagher_read.
 * Perhaps with other types of tags, verify would be useful.
*/
/*
static bool gallagher_verify(Nfc* nfc) {
    const uint8_t block_num = mf_classic_get_first_block_num_of_sector(CREDENTIAL_SECTOR);
    MfClassicAuthContext auth_context;
    MfClassicError error = mf_classic_poller_sync_auth(
        nfc, CREDENTIAL_SECTOR, (MfClassicKey*)&KEY_A, MfClassicKeyTypeA, &auth_context);
    if(error != MfClassicErrorNone) {
        FURI_LOG_D(TAG, "Failed to authenticate to block %u: %d", block_num, error);
        return false;
    }
    return true;
}
*/

static bool gallagher_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    if(!(data->type == MfClassicType1k || data->type == MfClassicType4k)) {
        return false;
    }

    // It's possible for a single tag to contain multiple credentials,
    // but this is currently unimplementecd.
    const uint8_t credential_sector_start_block_number =
        mf_classic_get_first_block_num_of_sector(GALLAGHER_CREDENTIAL_SECTOR);

    // Test 1: The first 8 bytes and the second 8 bytes should be bitwise inverses.
    const uint8_t* credential_block_start_ptr =
        &data->block[credential_sector_start_block_number].data[0];
    uint64_t cardholder_credential = nfc_util_bytes2num(credential_block_start_ptr, 8);
    uint64_t cardholder_credential_inverse = nfc_util_bytes2num(credential_block_start_ptr + 8, 8);
    // TODO: Is this accurate? Due to endianness, this is testing the bytes in the wrong order,
    // but the result still should be correct.
    if(cardholder_credential != ~cardholder_credential_inverse) {
        return false;
    }

    // Test 2: The contents of the second block should be equal to the GALLAGHER_CARDAX_ASCII constant.
    const uint8_t* cardax_block_start_ptr =
        &data->block[credential_sector_start_block_number + 1].data[0];
    if(memcmp(cardax_block_start_ptr, &GALLAGHER_CARDAX_ASCII, MF_CLASSIC_BLOCK_SIZE) != 0) {
        return false;
    }

    // Deobfuscate the credential data
    GallagherCredential credential;
    gallagher_deobfuscate_and_parse_credential(&credential, credential_block_start_ptr);

    furi_string_cat_printf(
        parsed_data,
        "\e#Gallagher NZ\nFAC: %02X %02X\nCARD: %02X %02X %02X %02X\nREGION: %02X ISSUE: %02X",
        (credential.facility >> 8) & 0xFF,
        credential.facility & 0xFF,
        (uint8_t)((credential.card >> 24) & 0xFF),
        (uint8_t)((credential.card >> 16) & 0xFF),
        (uint8_t)((credential.card >> 8) & 0xFF),
        (uint8_t)(credential.card & 0xFF),
        credential.region,
        credential.issue);
    return true;
}

static const NfcSupportedCardsPlugin gallagher_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = NULL,
    .read = NULL,
    .parse = gallagher_parse,
};

static const FlipperAppPluginDescriptor gallagher_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &gallagher_plugin,
};

/* Plugin entry point */
const FlipperAppPluginDescriptor* gallagher_plugin_ep() {
    return &gallagher_plugin_descriptor;
}