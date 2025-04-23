#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "example";

void list_files(const char *path)
{
    ESP_LOGI(TAG, "Listing files in directory: %s", path);
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        ESP_LOGE(TAG, "Could not open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        ESP_LOGI(TAG, "Found file: %s", entry->d_name);
    }
    closedir(dir);
}

void check_spiffs_info(const char *partition_label)
{
    size_t total = 0, used = 0;
    esp_err_t ret = esp_spiffs_info(partition_label, &total, &used);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(partition_label);
    }
}

void create_write_file(const char *path, const char *content)
{
    ESP_LOGI(TAG, "Opening file %s for writing", path);
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "%s", content);
    fclose(f);
    ESP_LOGI(TAG, "File written");
}

void read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    while (fgets(line, sizeof(line), f))
    {
        ESP_LOGI(TAG, "Read from file: '%s'", line);
    }
    fclose(f);
}

void append_file(const char *path, const char *content)
{
    ESP_LOGI(TAG, "Opening file %s for appending", path);
    FILE *f = fopen(path, "a");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for appending");
        return;
    }
    fprintf(f, "%s", content);
    fclose(f);
    ESP_LOGI(TAG, "File appended");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL, 
        .max_files = 5,
        .format_if_mount_failed = true
    };

    // Initialize and mount SPIFFS filesystem
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    check_spiffs_info(conf.partition_label);

    // Create and write to a file
    create_write_file("/spiffs/hello.txt", "Hello Milan");
    create_write_file("/spiffs/hello1.txt", "Hello Pipaliya");

    // Append to the file
    append_file("/spiffs/hello.txt", "hello milan!!!!!!!!.\n");

    // Read the file
    read_file("/spiffs/hello.txt");

    // List all files in SPIFFS
    list_files("/spiffs");

    // Unmount SPIFFS
    esp_vfs_spiffs_unregister(conf.partition_label);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}
