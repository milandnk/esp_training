#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "example";

typedef struct
{
    char name[30];
    int std;
    int roll_num;
    char gender;
} student_t;

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

void create_write_student(const char *path, student_t *student)
{
    ESP_LOGI(TAG, "Opening file %s for writing", path);
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Name: %s\nStd: %d\nRoll Number: %d\nGender: %c\n", student->name, student->std, student->roll_num, student->gender);
    fclose(f);
    ESP_LOGI(TAG, "File written");
}

void append_file(const char *path, const char *text)
{
    ESP_LOGI(TAG, "Opening file %s for appending", path);
    FILE *f = fopen(path, "a");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for appending");
        return;
    }
    fprintf(f, "%s", text);
    fclose(f);
    ESP_LOGI(TAG, "File appended");
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
        ESP_LOGI(TAG, "Read from file: %s", line);
    }
    fclose(f);
}


void app_main(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount or format filesystem");
        return;
    }
    check_spiffs_info(conf.partition_label);

    student_t student;
    strcpy(student.name, "milan");
    student.gender = 'm';
    student.roll_num = 1;
    student.std = 16;

    create_write_student("/spiffs/file.txt", &student);

    append_file("/spiffs/file.txt", "hello milan!!!!!!!!.\n");

    read_file("/spiffs/file.txt");

    list_files("/spiffs");

    esp_vfs_spiffs_unregister(conf.partition_label);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}
