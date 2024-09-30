/*
	This creates two empty databases, populates values, and retrieves them back
	from the LITTLEFS file system
*/
#include <FS.h>
#include <SPI.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "LittleFS.h"


#define 	G_E10_DBG_PPROCESS_1
#define 	G_E10_DBG_ERROR

#ifdef G_E10_DBG_PPROCESS_1
#ifdef G_E10_DBG_ERROR

#endif
#endif

/* You only need to format LITTLEFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32fs-plugin/releases */

#define 	G_E10_FORMAT_LITTLEFS_IF_FAILED 		false

#define 	G_E10_SQLITE_DB_FILENAME_1				"/E10_test1.db"
#define 	G_E10_SQLITE_DB_FILENAME_2				"/E10_test2.db"

sqlite3 	*g_E10_Sqlite_DB_1;
sqlite3 	*g_E10_Sqlite_DB_2;


static int	E10_db_callback(void *data, int argc, char **argv, char **azColName) {

	int i;
	Serial.printf("%s: ", (const char *)data);
	
	for (i = 0; i < argc; i++) {
		Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	Serial.printf("\n");
	return 0;
}

int E10_db_open(const char *filename, sqlite3 **db) {
	int v_Sql_Result = sqlite3_open(filename, db);
	if (v_Sql_Result) {
		Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
		return v_Sql_Result;
	} else {
		Serial.printf("Opened database successfully\n");
	}
	return v_Sql_Result;
}


int	  E10_db_exec(sqlite3 *db, const char *sql) {

	const char *data = "Callback function called";

	char *zErrMsg = 0;

	Serial.println(sql);
	
	long start = micros();
	int  v_Sql_Result	 = sqlite3_exec(db, sql, E10_db_callback, (void *)data, &zErrMsg);
	if (v_Sql_Result != SQLITE_OK) {
		Serial.printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	} else {
		Serial.printf("Operation done successfully\n");
	}
	Serial.print(F("Time taken:"));
	Serial.println(micros() - start);
	return v_Sql_Result;
}


void E10_init() {

	int		 v_Sql_Result;

	if (!LittleFS.begin(G_E10_FORMAT_LITTLEFS_IF_FAILED, "/littlefs")) {
		Serial.println("Failed to mount file system");
		return;
	}

	// list LITTLEFS contents
	File root = LittleFS.open("/");
	if (!root) {
		Serial.println("- failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		Serial.println(" - not a directory");
		return;
	}
	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(file.name());
		} else {
			Serial.print("  FILE: ");
			Serial.print(file.name());
			Serial.print("\tSIZE: ");
			Serial.println(file.size());
		}
		file = root.openNextFile();
	}


	// remove existing file
	LittleFS.remove(G_E10_SQLITE_DB_FILENAME_1);
	LittleFS.remove(G_E10_SQLITE_DB_FILENAME_2);

	sqlite3_initialize();


	if (E10_db_open(G_E10_SQLITE_DB_FILENAME_1, &g_E10_Sqlite_DB_1)){
	//if (E10_db_open("/littlefs/test1.db", &g_E10_Sqlite_DB_1)){
		return;
	}
	if (E10_db_open(G_E10_SQLITE_DB_FILENAME_2, &g_E10_Sqlite_DB_2)){
	//if (E10_db_open("/littlefs/test2.db", &g_E10_Sqlite_DB_2)){
		return;
	}

	v_Sql_Result = E10_db_exec(g_E10_Sqlite_DB_1, "CREATE TABLE test1 (id INTEGER, content);");
	if (v_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_E10_Sqlite_DB_1);
		//sqlite3_close(g_E10_Sqlite_DB_2);
		return;
	}

	
	v_Sql_Result = E10_db_exec(g_E10_Sqlite_DB_2, "CREATE TABLE test2 (id INTEGER, content);");
	if (v_Sql_Result != SQLITE_OK) {
		//sqlite3_close(g_E10_Sqlite_DB_1);
		sqlite3_close(g_E10_Sqlite_DB_2);
		return;
	}

	v_Sql_Result = E10_db_exec(g_E10_Sqlite_DB_1, "INSERT INTO test1 VALUES (1, 'Hello, World from test1');");
	if (v_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_E10_Sqlite_DB_1);
		//sqlite3_close(g_E10_Sqlite_DB_2);
		return;
	}
	v_Sql_Result = E10_db_exec(g_E10_Sqlite_DB_2, "INSERT INTO test2 VALUES (1, 'Hello, World from test2');");
	if (v_Sql_Result != SQLITE_OK) {
		//sqlite3_close(g_E10_Sqlite_DB_1);
		sqlite3_close(g_E10_Sqlite_DB_2);
		return;
	}

	v_Sql_Result = E10_db_exec(g_E10_Sqlite_DB_1, "SELECT * FROM test1");
	if (v_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_E10_Sqlite_DB_1);
		//sqlite3_close(g_E10_Sqlite_DB_2);
		return;
	}
	v_Sql_Result = E10_db_exec(g_E10_Sqlite_DB_2, "SELECT * FROM test2");
	if (v_Sql_Result != SQLITE_OK) {
		//sqlite3_close(g_E10_Sqlite_DB_1);
		sqlite3_close(g_E10_Sqlite_DB_2);
		return;
	}

	sqlite3_close(g_E10_Sqlite_DB_1);
	sqlite3_close(g_E10_Sqlite_DB_2);
}

void E10_run() {
}
