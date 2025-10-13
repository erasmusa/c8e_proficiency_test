#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <sqlite3.h>

void generate_csv(int num_records);
void import_csv(const char* filename);

#endif
