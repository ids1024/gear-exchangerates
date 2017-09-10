#include "watchtest.h"
#include "apikey.h"
#include <curl/curl.h>
#include <json-glib.h>
#include <net_connection.h>
#include <string>

struct AppData {
    Evas_Object *win;
    Evas_Object *conform;
    Evas_Object *genlist;
    Evas_Object *circle_genlist;
    Eext_Circle_Surface *surface;
    Elm_Genlist_Item_Class genlist_line_class;
    Elm_Genlist_Item_Class genlist_title_class;
    Elm_Genlist_Item_Class genlist_padding_class;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
    auto str = (std::string *)userp;
    str->append((char *)contents, size * nmemb);
    return size * nmemb;
}

static std::string fetch_url(char *url) {
    CURL *curl = curl_easy_init();
    connection_h connection;
    int conn_err = connection_create(&connection);
    if (conn_err == CONNECTION_ERROR_NONE)
        return ""; // XXX

    std::string chunk;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    int curl_err = curl_easy_perform(curl);
    if (curl_err != CURLE_OK)
        return ""; // XXX

    curl_easy_cleanup(curl);
    connection_destroy(connection);

    return chunk;
}

static void win_delete_request_cb(void *data, Evas_Object *obj,
                                  void *event_info) {
    ui_app_exit();
}

static void win_back_cb(void *data, Evas_Object *obj, void *event_info) {
    auto ad = (AppData *)data;
    /* Let window go to hide state. */
    elm_win_lower(ad->win);
}

static char *plain_label_get(void *data, Evas_Object *obj, const char *part) {
    char *label = (char *)data;
    return strdup(label);
}

static void create_base_gui(AppData *ad) {
    ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
    elm_win_autodel_set(ad->win, EINA_TRUE);

    if (elm_win_wm_rotation_supported_get(ad->win)) {
        int rots[4] = {0, 90, 180, 270};
        elm_win_wm_rotation_available_rotations_set(ad->win,
                                                    (const int *)(&rots), 4);
    }

    evas_object_smart_callback_add(ad->win, "delete,request",
                                   win_delete_request_cb, NULL);
    eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb,
                                   ad);

    ad->conform = elm_conformant_add(ad->win);
    elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
    evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND,
                                     EVAS_HINT_EXPAND);
    elm_win_resize_object_add(ad->win, ad->conform);
    evas_object_show(ad->conform);

    /* Show window after base gui is set up */
    evas_object_show(ad->win);

    ad->surface = eext_circle_surface_conformant_add(ad->conform);
    ad->genlist = elm_genlist_add(ad->conform);

    ad->genlist_line_class.item_style = "1text";
    ad->genlist_line_class.func.text_get = plain_label_get;
    ad->genlist_line_class.func.content_get = NULL;
    ad->genlist_line_class.func.state_get = NULL;
    ad->genlist_line_class.func.del = NULL;

    ad->genlist_title_class.item_style = "title";
    ad->genlist_title_class.func.text_get = plain_label_get;
    ad->genlist_title_class.func.content_get = NULL;
    ad->genlist_title_class.func.state_get = NULL;
    ad->genlist_title_class.func.del = NULL;

    elm_genlist_item_append(ad->genlist, &(ad->genlist_title_class),
                            (void *)"My Service Launcher", NULL,
                            ELM_GENLIST_ITEM_NONE, NULL, NULL);

    auto chunk = fetch_url(
        "https://openexchangerates.org/api/latest.json?app_id=" APIKEY);

    JsonParser *jsonParser = NULL;
    GError *error = NULL;
    jsonParser = json_parser_new();

    // XXX errors
    json_parser_load_from_data(jsonParser, chunk.c_str(), chunk.size(), NULL);
    auto root = json_parser_get_root(jsonParser);
    auto obj = json_node_get_object(root);
    auto rates = json_object_get_object_member(obj, "rates");
    auto list = json_object_get_members(rates);
    for (auto i = list; i != NULL; i = i->next) {
        gdouble rate =
            json_object_get_double_member(rates, (const gchar *)i->data);
        gchar *str = g_strdup_printf("%s - %f", i->data, rate);
        elm_genlist_item_append(ad->genlist, &(ad->genlist_line_class), str,
                                NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
    }

    g_list_free(list);

    elm_object_content_set(ad->conform, ad->genlist);
    evas_object_show(ad->genlist);

    ad->circle_genlist =
        eext_circle_object_genlist_add(ad->genlist, ad->surface);
    eext_circle_object_genlist_scroller_policy_set(
        ad->circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
    eext_rotary_object_event_activated_set(ad->circle_genlist, EINA_TRUE);
}

static bool app_create(void *data) {
    /* Hook to take necessary actions before main event loop starts
        Initialize UI resources and application's data
        If this function returns true, the main loop of application starts
        If this function returns false, the application is terminated */
    auto ad = (AppData *)data;

    create_base_gui(ad);

    return true;
}

static void app_control(app_control_h app_control, void *data) {
    /* Handle the launch request. */
}

static void app_pause(void *data) {
    /* Take necessary actions when application becomes invisible. */
}

static void app_resume(void *data) {
    /* Take necessary actions when application becomes visible. */
}

static void app_terminate(void *data) {
    /* Release all resources. */
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data) {
    /*APP_EVENT_LANGUAGE_CHANGED*/
    char *locale = NULL;
    system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
                                     &locale);
    elm_language_set(locale);
    free(locale);
    return;
}

static void ui_app_orient_changed(app_event_info_h event_info,
                                  void *user_data) {
    /*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
    return;
}

static void ui_app_region_changed(app_event_info_h event_info,
                                  void *user_data) {
    /*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data) {
    /*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data) {
    /*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[]) {
    AppData ad = {
        0,
    };
    int ret = 0;

    ui_app_lifecycle_callback_s event_callback = {
        0,
    };
    app_event_handler_h handlers[5] = {
        NULL,
    };

    event_callback.create = app_create;
    event_callback.terminate = app_terminate;
    event_callback.pause = app_pause;
    event_callback.resume = app_resume;
    event_callback.app_control = app_control;

    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY],
                             APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY],
                             APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED],
                             APP_EVENT_DEVICE_ORIENTATION_CHANGED,
                             ui_app_orient_changed, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
                             APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed,
                             &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
                             APP_EVENT_REGION_FORMAT_CHANGED,
                             ui_app_region_changed, &ad);

    ret = ui_app_main(argc, argv, &event_callback, &ad);
    if (ret != APP_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
    }

    return ret;
}
