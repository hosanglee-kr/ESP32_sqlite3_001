/*
	This example demonstrates how SQLite behaves
	when memory is low.
	It shows how heap defragmentation causes
	out of memory and how to avoid it.
	At first it asks how much memory to occupy
	so as not be available to SQLite.  Then
	tries to insert huge number of records
	and shows how much free memory available
	after each insert.
*/
#include <FS.h>
#include <SPI.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "SD_MMC.h"
#include "SPIFFS.h"



const char *g_E40_random_strings[] = {
				  "Hello world"
				, "Have a nice day"
				, "Testing memory problems"
				, "This should work"
				, "ESP32 has 512k RAM"
				, "ESP8266 has only 36k user RAM"
				, "A stitch in time saves nine"
				, "Needle in a haystack"
				, "Too many strings"
				, "I am done"
			};
			
//char		  sql[1024];
sqlite3		 *g_E40_db_1;
sqlite3_stmt *g_E40_db_Prep_Statement;
const char	 *g_E40_db_tail;
int			  g_E40_Sql_Result;




char *g_E40_dat = NULL;
void  E40_block_heap(int times) {
	 while (times--) {
		 g_E40_dat = (char *)malloc(4096);
	 }
}


static int	E40_db_callback(void *data, int argc, char **argv, char **azColName) {
	 int i;
	 Serial.printf("%s: ", (const char *)data);
	 for (i = 0; i < argc; i++) {
		 Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	 }
	 Serial.printf("\n");
	 return 0;
}

int E40_db_open(const char *filename, sqlite3 **db) {
	int rc = sqlite3_open(filename, db);
	if (rc) {
		Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
		return rc;
	} else {
		Serial.printf("Opened database successfully\n");
	}
	return rc;
}


int	  E40_db_exec(sqlite3 *db, const char *sql) {
    char *zErrMsg = 0;
    const char *data = "Callback function called";

    Serial.println(sql);
    long start = micros();
    int  rc	 = sqlite3_exec(db, sql, E40_db_callback, (void *)data, &zErrMsg);
    if (rc != SQLITE_OK) {
        Serial.printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        Serial.printf("Operation done successfully\n");
    }
    Serial.print(F("Time taken:"));
    Serial.println(micros() - start);
    return rc;
}

int E40_input_string(char *str, int max_len) {
	max_len--;
	int ctr	 = 0;
	str[ctr] = 0;
	while (str[ctr] != '\n') {
		if (Serial.available()) {
			str[ctr] = Serial.read();
			if (str[ctr] >= ' ' && str[ctr] <= '~')
				ctr++;
			if (ctr >= max_len)
				break;
		}
	}
	str[ctr] = 0;
	Serial.println(str);

    return ctr;
}

int E40_input_num() {
	char in[20];
	int	 ctr = 0;
	in[ctr]	 = 0;
	while (in[ctr] != '\n') {
		if (Serial.available()) {
			in[ctr] = Serial.read();
			if (in[ctr] >= '0' && in[ctr] <= '9')
				ctr++;
			if (ctr >= sizeof(in))
				break;
		}
	}
	in[ctr] = 0;
	int ret = atoi(in);
	Serial.println(ret);
	return ret;
}

void E40_displayPrompt(const char *title) {
	Serial.print(F("Enter "));
	Serial.println(title);
}

void E40_displayFreeHeap() {
	Serial.printf("\nHeap size: %d\n", ESP.getHeapSize());
	Serial.printf("Free Heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	Serial.printf("Min Free Heap: %d\n", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
	Serial.printf("Max Alloc Heap: %d\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}




void E40_init() {


	if (!SPIFFS.begin(true)) {
		Serial.println(F("Failed to mount file Serial"));
		return;
	}

	randomSeed(analogRead(0));

	SPI.begin();
	SD_MMC.begin();

	E40_displayFreeHeap();
	E40_displayPrompt("No. of 4k heap to block:");
	E40_block_heap(E40_input_num());
	E40_displayFreeHeap();

	sqlite3_initialize();
}

void E40_run() {

	// Open database 1
	if (E40_db_open("/sdcard/bulk_ins.db", &g_E40_db_1))
		return;

	E40_displayFreeHeap();

	g_E40_Sql_Result = E40_db_exec(g_E40_db_1, "CREATE TABLE IF NOT EXISTS test (c1 INTEGER, c2, c3, c4, c5 INTEGER, c6 INTEGER, c7, c8, c9 DATETIME, c10 DATETIME, c11 INTEGER )");
	if (g_E40_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_E40_db_1);
		return;
	}

	E40_displayFreeHeap();

	int rec_count;
	E40_displayPrompt("No. of records to insert:");
	rec_count = E40_input_num();

	const char *sql = "INSERT INTO test VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	g_E40_Sql_Result		  = sqlite3_prepare_v2(g_E40_db_1, sql, strlen(sql), &g_E40_db_Prep_Statement, &g_E40_db_tail);
	if (g_E40_Sql_Result != SQLITE_OK) {
		Serial.printf("ERROR preparing sql: %s\n", sqlite3_errmsg(g_E40_db_1));
		sqlite3_close(g_E40_db_1);
		return;
	}
	const char *value;
	while (rec_count--) {
		sqlite3_bind_int(   
                              g_E40_db_Prep_Statement
                            , 1                                 // 바인딩할 파라메터 위치 번호
                            , random(65535)
                            );

		value = g_E40_random_strings[random(10)];
		sqlite3_bind_text(
                              g_E40_db_Prep_Statement
                            , 2
                            , value
                            , strlen(value)
                            , SQLITE_STATIC                     // 바인팅 메모리 해재 방법 
                            );
		value = g_E40_random_strings[random(10)];
		sqlite3_bind_text(g_E40_db_Prep_Statement, 3, value, strlen(value), SQLITE_STATIC);

		value = g_E40_random_strings[random(10)];
		sqlite3_bind_text(g_E40_db_Prep_Statement, 4, value, strlen(value), SQLITE_STATIC);
		
        sqlite3_bind_int(g_E40_db_Prep_Statement, 5, random(65535));
		sqlite3_bind_int(g_E40_db_Prep_Statement, 6, random(65535));
		
        value = g_E40_random_strings[random(10)];
		sqlite3_bind_text(g_E40_db_Prep_Statement, 7, value, strlen(value), SQLITE_STATIC);
		
        value = g_E40_random_strings[random(10)];
		sqlite3_bind_text(g_E40_db_Prep_Statement, 8, value, strlen(value), SQLITE_STATIC);
		
        sqlite3_bind_int(g_E40_db_Prep_Statement, 9, random(100000000L));
		sqlite3_bind_int(g_E40_db_Prep_Statement, 10, random(100000000L));
		sqlite3_bind_int(g_E40_db_Prep_Statement, 11, random(65535));


		if (sqlite3_step(g_E40_db_Prep_Statement) != SQLITE_DONE) {
			Serial.printf("ERROR executing stmt: %s\n", sqlite3_errmsg(g_E40_db_1));
			sqlite3_close(g_E40_db_1);
			return;
		}

		sqlite3_clear_bindings(g_E40_db_Prep_Statement);

		g_E40_Sql_Result = sqlite3_reset(g_E40_db_Prep_Statement);
		if (g_E40_Sql_Result != SQLITE_OK) {
			sqlite3_close(g_E40_db_1);
			return;
		}
		E40_displayFreeHeap();
	}
	sqlite3_finalize(g_E40_db_Prep_Statement);
	Serial.write("\n");

	g_E40_Sql_Result = E40_db_exec(g_E40_db_1, "Select count(*) from test");
	if (g_E40_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_E40_db_1);
		return;
	}

	sqlite3_close(g_E40_db_1);
	E40_displayFreeHeap();

	E40_displayPrompt("Press enter to continue:");
	E40_input_num();
}
