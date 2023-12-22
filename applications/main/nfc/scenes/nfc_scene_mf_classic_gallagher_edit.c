#include "../nfc_app_i.h"
#include <nfc/helpers/gallagher_util.h>
#include <nfc/helpers/nfc_util.h>

// https://stackoverflow.com/questions/3553296/sizeof-single-struct-member-in-c
#define member_size(type, member) sizeof(((type*)0)->member)

// TODO: Should this be stored using the existing scene state functionality?
MfClassicData* data;
GallagherCredential credential;

void nfc_scene_mf_classic_gallagher_edit_byte_input_callback(void* context) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventByteInputDone);
}

void nfc_scene_mf_classic_gallagher_edit_on_enter(void* context) {
    NfcApp* instance = context;
    NfcDevice* device = instance->nfc_device;
    furi_assert(device);

    data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    // The following is more-or-less copied from gallagher.c in plugins/supported_cards -- consider refactoring.
    // What could this API look like?
    // Get credential from MfClassicData?

    // Parse credential
    const uint8_t credential_sector_start_block_number =
        mf_classic_get_first_block_num_of_sector(GALLAGHER_CREDENTIAL_SECTOR);
    const uint8_t* credential_block_start_ptr =
        &data->block[credential_sector_start_block_number].data[0];
    gallagher_deobfuscate_and_parse_credential(&credential, credential_block_start_ptr);

    // Setup view
    ByteInput* byte_input = instance->byte_input;
    // Copy existing CN into CN field
    nfc_util_num2bytes(credential.card, sizeof(credential.card), instance->byte_input_store);
    byte_input_set_header_text(byte_input, "Enter the CN in hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_scene_mf_classic_gallagher_edit_byte_input_callback,
        NULL,
        instance,
        instance->byte_input_store,
        member_size(GallagherCredential, card));
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewByteInput);
}

bool nfc_scene_mf_classic_gallagher_edit_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    NfcDevice* device = instance->nfc_device;
    furi_assert(device);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            // Turn user-provided CN bytes back into a uint32_t
            credential.card =
                nfc_util_bytes2num(instance->byte_input_store, sizeof(credential.card));

            // Turn the credential data back into encoded (16-byte) format
            const uint8_t credential_sector_start_block_number =
                mf_classic_get_first_block_num_of_sector(GALLAGHER_CREDENTIAL_SECTOR);
            // non-const here -- we are going to modify it
            uint8_t* credential_block_start_ptr =
                &data->block[credential_sector_start_block_number].data[0];
            gallagher_obfuscate_credential(credential_block_start_ptr, credential);

            // We've written to memory now, but we need to set the data of the NFC device
            nfc_device_set_data(device, NfcProtocolMfClassic, data);
            // Note that this doesn't seem to _save_ to disk...

            // scene_manager_next_scene(instance->scene_manager, NfcSceneSaveSuccess);
            // scene_manager_previous_scene(instance->scene_manager);
            consumed = true;
        }
    }

    scene_manager_previous_scene(instance->scene_manager);
    return consumed;
}

void nfc_scene_mf_classic_gallagher_edit_on_exit(void* context) {
    NfcApp* instance = context;

    // Clear view
    byte_input_set_result_callback(instance->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(instance->byte_input, "");

    // Free allocated data
    mf_classic_free(data);
    data = NULL;
}
