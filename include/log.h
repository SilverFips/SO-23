#ifndef LOG_H_GUARD
#define LOG_H_GUARD

void file_log_begin(char* name);

void file_destroy_log();

void write_file_log(char* data, char* instruction, int argument);

#endif