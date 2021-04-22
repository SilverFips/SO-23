


#ifndef CONFIGUTARION_H_GUARD
#define CONFIGURATION_H_GUARD


struct file_log {
    FILE* f_log;
    char* log_name;
};

int open_begin_file(char* name_file, struct main_data* data);

#endif