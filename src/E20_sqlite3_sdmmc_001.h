/*
	This example opens Sqlite3 databases from SD Card and
	retrieves data from them.
	Before running please copy following files to SD Card:
	data/mdr512.db
	data/census2000names.db
*/
#include <FS.h>
#include <SPI.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "SD_MMC.h"


static int	E20_db_callback(void *data, int argc, char **argv, char **azColName) {
	 int i;
	 Serial.printf("%s: ", (const char *)data);
	 for (i = 0; i < argc; i++) {
		 Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	 }
	 Serial.printf("\n");
	 return 0;
}

int E20_db_open(const char *filename, sqlite3 **db) {
	int v_Sql_Result = sqlite3_open(filename, db);
	if (v_Sql_Result) {
		Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
		return v_Sql_Result;
	} else {
		Serial.printf("Opened database successfully\n");
	}
	return v_Sql_Result;
}


int	  E20_db_exec(sqlite3 *db, const char *in_sql) {
	const char *data = "Callback function called";

	char *zErrMsg = 0;

	Serial.println(in_sql);
	long start = micros();
	int  v_Sql_Result	 = sqlite3_exec(db, in_sql, E20_db_callback, (void *)data, &zErrMsg);
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

void E20_init() {


	sqlite3 *g_E20_db_1;
	sqlite3 *g_E20_db_2;
	char	*zErrMsg = 0;
	int		 v_Sql_Result;

	SPI.begin();
	SD_MMC.begin();

	sqlite3_initialize();

	// Open database 1
	if (E20_db_open("/sdcard/census2000names.db", &g_E20_db_1))
		return;
	if (E20_db_open("/sdcard/mdr512.db", &g_E20_db_2))
		return;

	v_Sql_Result = E20_db_exec(g_E20_db_1, "Select * from surnames where name = 'MICHELLE'");
	if (v_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_E20_db_1);
		sqlite3_close(g_E20_db_2);
		return;
	}
	v_Sql_Result = E20_db_exec(g_E20_db_2, "Select * from domain_rank where domain between 'google.com' and 'google.com.z'");
	if (v_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_E20_db_1);
		sqlite3_close(g_E20_db_2);
		return;
	}
	v_Sql_Result = E20_db_exec(g_E20_db_1, "Select * from surnames where name = 'SPRINGER'");
	if (v_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_E20_db_1);
		sqlite3_close(g_E20_db_2);
		return;
	}
	v_Sql_Result = E20_db_exec(g_E20_db_2, "Select * from domain_rank where domain = 'zoho.com'");
	if (v_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_E20_db_1);
		sqlite3_close(g_E20_db_2);
		return;
	}

	sqlite3_close(g_E20_db_1);
	sqlite3_close(g_E20_db_2);
}

void E20_run() {
}
