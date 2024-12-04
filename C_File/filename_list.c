/* 文件命名按序增长 */

#include <string.h>
#include <stdio.h>
#include <dirent.h>

/* 遍历文件名的实现形式 */
/* config */
static const char *TAG =                   "FILE_NAME";
#define BASE_PATH                          "/data"
#define FILE_NAME_                         "adc_data_"
#define FILE_FORMAT(file, num)             file "%d.txt", num
#define FILE_PATH(path, file, num)         path "/" file "%d.txt", num
#define MAX_FILENAME_LENGTH                64
#ifdef DEBUG_LOG
#define LOGI                               ESP_LOGI
#define LOGE                               ESP_LOGE
#else
#define LOGI                               //
#define LOGE                               // 
#endif // DEBUG_LOG

// 当前文件序号
static int current_file_index = 0;

// 获取当前应该使用的文件序号
static int get_next_file_index(void)
{
    DIR *dir = opendir(BASE_PATH);
    if (dir == NULL)
    {
        LOGE(TAG, "Failed to open directory");
        return 0; // 如果无法访问文件系统，返回序号 0
    }

    struct dirent *entry;
    int max_index = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        int index;
        // 匹配文件名格式，例如 "<example_file_><index>.raw"
        if (sscanf(entry->d_name, FILE_FORMAT(BASE_PATH, FILE_NAME_, index)) == 1)
        {
            if (index > max_index)
            {
                max_index = index;
            }
        }
    }
    closedir(dir);

    return max_index + 1; // 返回下一个可用序号
}


/* example */
// 写入数据到 .raw 文件
static void write_to_raw_file(const uint8_t *data, size_t len, int index)
{
    char filename[MAX_FILENAME_LENGTH];
    snprintf(filename, MAX_FILENAME_LENGTH, FILE_FORMAT(FILE_NAME_), index);

    FILE *file = fopen(filename, "ab"); // 以二进制追加模式打开文件
    if (file == NULL) {
        LOGE(TAG, "Failed to open file: %s", filename);
        return;
    }

    fwrite(data, 1, len, file); // 写入数据
    fclose(file);

    LOGI(TAG, "Data written to file: %s, size: %d bytes", filename, len);
}



/* 建立隐藏文件专门存储文件序号的实现形式 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义文件路径
#define INDEX_FILE_PATH "/data/.IndexValue"  // 隐藏文件路径
#define MAX_FILENAME_LEN 64

// 将整数转换为三位数格式的字符串
static void int_to_str_with_padding(int num, char* str, int len)
{
    snprintf(str, len, "%03d", num);
}

// 初始化：读取现有的file_index，若没有则设置为初始值, 返回file_index是下次创建使用的序号
int get_file_index(const char* path) {
    FILE *f = fopen(path, "r");
    int file_index = 0;

    if (f == NULL) {
        // 如果文件不存在，返回默认值
        printf("File does not exist, initializing adc_current_file_index to 1\n");
        file_index = 1;
    } else {
        // 文件存在，读取值
        fscanf(f, "%d", &file_index);
        fclose(f);
        ESP_LOGI(TAG,"Get file_index: %d\n", file_index);
    }

    return file_index;
}

// 更新 file_index 到文件
void update_file_index(const char* path, int file_index) {
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        perror("Failed to open file for writing");
        return;
    }

    fprintf(f, "%d", file_index);
    fclose(f);
    printf("Updated file_index to: %d\n", file_index);
}

// 根据 file_index 创建并将文件名写入传入的 buffer 中
void create_new_file_name(const char* base, const char* prefix, int file_index, char* buffer, size_t buffer_len) {
    char index_str[4];

    // 将file_index转为三位数的字符串
    int_to_str_with_padding(file_index, index_str, sizeof(index_str));

    // 创建文件名并存入 buffer
    snprintf(buffer, buffer_len, "%s/%s%s.txt", base, prefix, index_str);
}

/* example */
// 更新 file_index 并创建新文件
static void update_and_create_file() {
    int current_file_index = get_file_index(INDEX_FILE_PATH);
    // 用户自定义文件名前缀
    const char* prefix = "test_data_";
    const char* base_path = "/data";
    char filename[MAX_FILENAME_LEN];

    // 调用函数生成文件名并填充到传入的 buffer 中
    create_new_file_name(base_path, prefix, current_file_index, filename, sizeof(filename));

    // 创建新文件并写入数据
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        perror("Failed to open new file for writing");
        return;
    }

    // 写入一些示例数据
    fprintf(f, "Test data: %d\n", current_file_index);
    fclose(f);

    printf("New file created: %s\n", filename);

    // 增加 current_file_index
    current_file_index++;

    // 更新 current_file_index 并写回文件
    update_file_index(INDEX_FILE_PATH, current_file_index);
}
