#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 初始化：读取现有的file_index，若没有则设置为初始值, 返回file_index是下次创建使用的序号
int get_file_index(const char* path);
// 更新 file_index 到文件
void update_file_index(const char* path, int file_index);
// 根据 file_index 创建并将文件名写入传入的 buffer 中
void create_new_file_name(const char* base, const char* prefix, int file_index, char* buffer, size_t buffer_len);

#ifdef __cplusplus
}
#endif
