#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3.h>
#include "data_manager.h"

#define DB_PATH "data.db"
#define CSV_PATH "output.csv"

// Names and Surnames for random generation
const char* first_names[] = {
    "Adriaan", "Petrus", "Sasha", "Alex", "Jordan", "Taylor",
    "Jamie", "Casey", "Morgan", "Ryan", "Liam", "Oliver",
    "Elijah", "James", "William", "Benjamin", "Lucas", "Henry",
    "Theodore", "Noah"
};
const char* surnames[] = {
    "Erasmus", "Cohen", "Smith", "Jones", "Williams", "Brown",
    "Davis", "Wilson", "Taylor", "Van Wyk", "Theron", "Johnson",
    "Garcia", "Miller", "Rodriguez", "Martinez", "Hernandez",
    "Lopez", "Gonzalez", "Anderson"
};
#define NUM_NAMES 20
#define NAME_BUFFER_SIZE 100
#define INITIALS_BUFFER_SIZE 5
#define LINE_BUFFER_SIZE 1024

// Function to handle SQLite errors
void handle_sqlite_error(sqlite3* db, const char* msg) {
    fprintf(stderr, "%s: %s\n", msg, sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(EXIT_FAILURE);
}

// Function to generate a random first name, which may be a double name
void generateRandomFirstNames(char *buffer, size_t bufferSize) {
    // 50% chance for a double name
    if (rand() % 2 == 0) {
        // Generate a single first name
        int firstIndex = rand() % NUM_NAMES;
        snprintf(buffer, bufferSize, "%s", first_names[firstIndex]);
    } else {
        // Generate a double first name
        int firstIndex = rand() % NUM_NAMES;
        int secondIndex;
        do {
            secondIndex = rand() % NUM_NAMES;
        } while (firstIndex == secondIndex);
        snprintf(buffer, bufferSize, "%s %s", first_names[firstIndex], first_names[secondIndex]);
    }
}

// Function to generate initials from the first name(s) only
void generateInitials(const char *name, char *initials, size_t bufferSize) {
    if (name == NULL || initials == NULL || bufferSize < 2) return;

    // Reset buffer
    initials[0] = '\0';

    // Add the first initial
    initials[0] = name[0];
    initials[1] = '\0';

    // Find a space for a double name
    const char* second_name_part = strchr(name, ' ');
    if (second_name_part != NULL && strlen(initials) + 1 < bufferSize) {
        // Add the second initial
        strncat(initials, second_name_part + 1, 1);
    }
}

// Function to generate and save a CSV file
void generate_csv(int num_records) {
    FILE *fp = fopen(CSV_PATH, "w");
    if (fp == NULL) {
        perror("Error opening CSV file");
        exit(EXIT_FAILURE);
    }

    // Header with double quotes
    fprintf(fp, "\"Id\",\"Name\",\"Surname\",\"Initials\",\"Age\",\"DateOfBirth\"\n");

    srand(time(NULL)); // Seed the random number generator
    time_t now = time(NULL);
    struct tm *today = localtime(&now);

    for (int i = 1; i <= num_records; ++i) {
        char name[NAME_BUFFER_SIZE];
        char initials[INITIALS_BUFFER_SIZE];
        const char *surname;

        // Generate random first name(s) and single surname
        generateRandomFirstNames(name, sizeof(name));
        surname = surnames[rand() % NUM_NAMES];

        // Generate initials
        generateInitials(name, initials, sizeof(initials));

        // Random Age between 10 and 80
        int age = 10 + (rand() % 71);

        // Calculate realistic Date of Birth
        int birth_year = today->tm_year + 1900 - age;
        int birth_month = 1 + (rand() % 12);
        int birth_day = 1 + (rand() % 28);

        // Write to CSV with double quotes around each value
        fprintf(fp, "\"%d\",\"%s\",\"%s\",\"%s\",\"%d\",\"%02d/%02d/%04d\"\n",
                i, name, surname, initials, age, birth_day, birth_month, birth_year);
    }

    fclose(fp);
    printf("Successfully generated %d records in '%s'\n", num_records, CSV_PATH);
}

// Function to import CSV into SQLite
void import_csv(const char* filename) {
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open(DB_PATH, &db);

    if (rc != SQLITE_OK) {
        handle_sqlite_error(db, "Cannot open database");
    }

    const char *sql_create =
        "CREATE TABLE IF NOT EXISTS csv_import ("
        "Id INTEGER PRIMARY KEY,"
        "Name TEXT,"
        "Surname TEXT,"
        "Initials TEXT,"
        "Age INTEGER,"
        "DateOfBirth TEXT"
        ");";
    rc = sqlite3_exec(db, sql_create, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        handle_sqlite_error(db, "Failed to create table");
    }

    rc = sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        handle_sqlite_error(db, "Failed to begin transaction");
    }

    sqlite3_stmt *stmt;
    const char *sql_insert = "INSERT INTO csv_import VALUES (?, ?, ?, ?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        handle_sqlite_error(db, "Failed to prepare statement");
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening CSV file for import");
        sqlite3_finalize(stmt);
        sqlite3_exec(db, "ROLLBACK", 0, 0, NULL);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }

    char line[LINE_BUFFER_SIZE];
    fgets(line, sizeof(line), fp); // Skip header row

    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        char *id_str, *name_val, *surname_val, *initials_val, *age_str, *dob_val;
        char *ptr = line;

        // Skip leading quote and tokenize
        if (*ptr == '"') ptr++;
        id_str = strtok(ptr, "\",");
        name_val = strtok(NULL, "\",");
        surname_val = strtok(NULL, "\",");
        initials_val = strtok(NULL, "\",");
        age_str = strtok(NULL, "\",");
        dob_val = strtok(NULL, "\"\n");

        if (id_str && name_val && surname_val && initials_val && age_str && dob_val) {
            sqlite3_bind_int(stmt, 1, atoi(id_str));
            sqlite3_bind_text(stmt, 2, name_val, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, surname_val, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, initials_val, -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 5, atoi(age_str));
            sqlite3_bind_text(stmt, 6, dob_val, -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                fprintf(stderr, "Error inserting row: %s\n", sqlite3_errmsg(db));
            }
            sqlite3_reset(stmt);
            count++;
        }
    }

    rc = sqlite3_exec(db, "COMMIT", 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        handle_sqlite_error(db, "Failed to commit transaction");
    }

    printf("Successfully imported %d records from '%s' into '%s'\n", count, filename, DB_PATH);

    fclose(fp);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Main function to parse command-line arguments
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s --generate <num_records> OR %s --import <csv_file>\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "--generate") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s --generate <num_records>\n", argv[0]);
            return EXIT_FAILURE;
        }
        int num_records = atoi(argv[2]);
        if (num_records <= 0) {
            fprintf(stderr, "Invalid number of records.\n");
            return EXIT_FAILURE;
        }
        generate_csv(num_records);
    } else if (strcmp(argv[1], "--import") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s --import <csv_file>\n", argv[0]);
            return EXIT_FAILURE;
        }
        import_csv(argv[2]);
    } else {
        fprintf(stderr, "Invalid command. Use --generate or --import.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
