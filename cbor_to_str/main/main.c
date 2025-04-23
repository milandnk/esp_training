#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "cbor.h"

static const char *TAG = "CBOR_TO_JSON";

// Simulate CBOR data generation (similar to esp_insights_cbor_encode_meta_begin)
size_t generate_cbor_data(uint8_t *buf, size_t buf_size)
{
    CborEncoder encoder, outer_map, diag_map;
    cbor_encoder_init(&encoder, buf, buf_size, 0);

    // Outer map with one key: "diagmeta"
    cbor_encoder_create_map(&encoder, &outer_map, 1);
    cbor_encode_text_stringz(&outer_map, "diagmeta");

    // Inner diagmeta map (indefinite length for simplicity)
    cbor_encoder_create_map(&outer_map, &diag_map, CborIndefiniteLength);

    cbor_encode_text_stringz(&diag_map, "ver");
    cbor_encode_text_stringz(&diag_map, "1.0.0");

    cbor_encode_text_stringz(&diag_map, "ts");
    cbor_encode_uint(&diag_map, 123456789);

    cbor_encode_text_stringz(&diag_map, "sha256");
    cbor_encode_text_stringz(&diag_map, "deadbeef1234567890");

    cbor_encoder_close_container(&outer_map, &diag_map);
    cbor_encoder_close_container(&encoder, &outer_map);

    return cbor_encoder_get_buffer_size(&encoder, buf);
}

// Decode CBOR and print as JSON
void decode_cbor_to_json(const uint8_t *data, size_t data_len)
{
    CborParser parser;
    CborValue it, outer_map;

    cbor_parser_init(data, data_len, 0, &parser, &it);

    if (!cbor_value_is_map(&it)) {
        ESP_LOGE(TAG, "Top-level CBOR is not a map!");
        return;
    }

    cbor_value_enter_container(&it, &outer_map);

    printf("{\n");

    while (!cbor_value_at_end(&outer_map)) {
        char key[64];
        size_t key_len = sizeof(key);
        if (cbor_value_is_text_string(&outer_map)) {
            cbor_value_copy_text_string(&outer_map, key, &key_len, &outer_map);
            printf("  \"%s\": {\n", key);

            CborValue diag_map;
            cbor_value_enter_container(&outer_map, &diag_map);

            while (!cbor_value_at_end(&diag_map)) {
                char subkey[64];
                size_t subkey_len = sizeof(subkey);
                cbor_value_copy_text_string(&diag_map, subkey, &subkey_len, &diag_map);
                printf("    \"%s\": ", subkey);

                if (cbor_value_is_text_string(&diag_map)) {
                    char val[128];
                    size_t val_len = sizeof(val);
                    cbor_value_copy_text_string(&diag_map, val, &val_len, &diag_map);
                    printf("\"%s\"", val);
                } else if (cbor_value_is_unsigned_integer(&diag_map)) {
                    uint64_t val;
                    cbor_value_get_uint64(&diag_map, &val);
                    cbor_value_advance(&diag_map);
                    printf("%llu", val);
                } else {
                    printf("\"<unsupported type>\"");
                    cbor_value_advance(&diag_map);
                }

                if (!cbor_value_at_end(&diag_map)) {
                    printf(",");
                }
                printf("\n");
            }

            cbor_value_leave_container(&outer_map, &diag_map);
            printf("  }\n");
        }
    }

    printf("}\n");
    cbor_value_leave_container(&it, &outer_map);
}

void app_main(void)
{
    ESP_LOGI(TAG, "CBOR to JSON decoder example");

    uint8_t cbor_buf[256];
    size_t cbor_len = generate_cbor_data(cbor_buf, sizeof(cbor_buf));

    ESP_LOGI(TAG, "Generated CBOR data of size %d bytes", (int)cbor_len);

    decode_cbor_to_json(cbor_buf, cbor_len);
}
