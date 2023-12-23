#include "../nfc_app_i.h"
#include <nfc/helpers/gallagher_util.h>
#include <nfc/helpers/nfc_util.h>

// TODO: Am I going to run into trouble storing static data here? When?
MfClassicData* data;
GallagherCredential credential;

void nfc_scene_mf_classic_gallagher_edit_byte_input_callback(void* context) {
    NfcApp* instance = context;

    enum GallagherCredentialFieldToSet field_to_set =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicGallagherEdit);

    // Which field do we want to copy from byte input back to the credential object?
#define X(name, enumName, englishName, size)                                         \
    case enumName:                                                                   \
        credential.name =                                                            \
            nfc_util_bytes2num(instance->byte_input_store, sizeof(credential.name)); \
        break;

    switch(field_to_set) { GALLAGHER_CREDENTIAL_FIELDS_LIST }
#undef X

    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventByteInputDone);
}

void prepare_byte_input_for_field(GallagherCredential credential, NfcApp* instance) {
    enum GallagherCredentialFieldToSet field_to_set =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicGallagherEdit);
    uint8_t member_size = 0;

// Which field of the credential are we going to ask the user for, and how large is it?
#define X(name, enumName, englishName, size)                                                      \
    case enumName:                                                                                \
        member_size = sizeof(credential.name);                                                    \
        nfc_util_num2bytes(credential.name, sizeof(credential.name), instance->byte_input_store); \
        byte_input_set_header_text(instance->byte_input, "Enter the " #englishName " in hex");    \
        break;

    switch(field_to_set) { GALLAGHER_CREDENTIAL_FIELDS_LIST }
#undef X

    byte_input_set_result_callback(
        instance->byte_input,
        nfc_scene_mf_classic_gallagher_edit_byte_input_callback,
        NULL,
        instance,
        instance->byte_input_store,
        member_size);
}

void nfc_scene_mf_classic_gallagher_edit_on_enter(void* context) {
    NfcApp* instance = context;
    NfcDevice* device = instance->nfc_device;
    furi_assert(device);

    // Create our own working copy of the MIFARE data
    data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    // Parse the existing credential, store it in memory
    const uint8_t credential_sector_start_block_number =
        mf_classic_get_first_block_num_of_sector(GALLAGHER_CREDENTIAL_SECTOR);
    const uint8_t* credential_block_start_ptr =
        &data->block[credential_sector_start_block_number].data[0];
    gallagher_deobfuscate_and_parse_credential(&credential, credential_block_start_ptr);

    // Set scene state
    // We don't set the state value here any more, since we should be setting it in the previous scene...
    prepare_byte_input_for_field(credential, instance);

    // Start the byte input flow to ask for all four values
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewByteInput);
}

bool nfc_scene_mf_classic_gallagher_edit_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    NfcDevice* device = instance->nfc_device;
    furi_assert(device);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            // Turn the credential data back into encoded (16-byte) format
            const uint8_t credential_sector_start_block_number =
                mf_classic_get_first_block_num_of_sector(GALLAGHER_CREDENTIAL_SECTOR);
            // non-const here -- we are going to modify it
            uint8_t* credential_block_start_ptr =
                &data->block[credential_sector_start_block_number].data[0];
            gallagher_obfuscate_credential(credential_block_start_ptr, credential);

            // We've written to memory now, but we need to set the data of the NFC device
            nfc_device_set_data(device, NfcProtocolMfClassic, data);
            // And save it using the shadow file functionality.
            // This means the user can easily restore it to the original read.
            nfc_save_shadow_file(instance);

            scene_manager_previous_scene(instance->scene_manager);
            consumed = true;
        }
    }
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
