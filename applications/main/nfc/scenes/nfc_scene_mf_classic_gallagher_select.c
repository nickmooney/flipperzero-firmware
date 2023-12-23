#include "../nfc_app_i.h"
#include <nfc/helpers/gallagher_util.h>

void nfc_scene_mf_classic_gallagher_select_submenu_callback(void* context, uint32_t index) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, index);
}

void nfc_scene_mf_classic_gallagher_select_on_enter(void* context) {
    NfcApp* instance = context;
    Submenu* submenu = instance->submenu;

#define X(name, enumName, englishName, size)                    \
    submenu_add_item(                                           \
        submenu,                                                \
        "Edit " #englishName,                                   \
        enumName,                                               \
        nfc_scene_mf_classic_gallagher_select_submenu_callback, \
        instance);

    GALLAGHER_CREDENTIAL_FIELDS_LIST
#undef X

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_classic_gallagher_select_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneMfClassicGallagherEdit, event.event);
        scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicGallagherEdit);
        return true;
    }

    return false;
}

void nfc_scene_mf_classic_gallagher_select_on_exit(void* context) {
    NfcApp* instance = context;

    submenu_reset(instance->submenu);
}