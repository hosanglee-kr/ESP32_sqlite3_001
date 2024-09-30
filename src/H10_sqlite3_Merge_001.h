

#include <FS.h>
#include <SPI.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>


#include <time.h>


//#define 	G_H10_DBG_PRO_1
//#define 	G_H10_DBG_PRO_2
#define 	G_H10_DBG_ERR


enum {
	E_H10_STORAGE_LITTLEFS = 1,
    E_H10_STORAGE_SDMMC,    
};

//#define G_H10_STORAGE_TYPE  E_H10_STORAGE_LITTLEFS
#define G_H10_STORAGE_TYPE  E_H10_STORAGE_SDMMC

#if G_H10_STORAGE_TYPE == E_H10_STORAGE_LITTLEFS        //E_H10_STORAGE_LITTLEFS, E_H10_STORAGE_SDMMC
    #include "LittleFS.h"
    
    #define 	G_H10_FORMAT_LITTLEFS_IF_FAILED 		false
    #define 	G_H10_SQLITE_DB_FILENAME_1				"/E10_test1.db"

#elif G_H10_STORAGE_TYPE == E_H10_STORAGE_SDMMC
    #include "SD_MMC.h"
    #define 	G_H10_SQLITE_DB_FILENAME_1				"/sdcard/census2000names.db"
#endif 


sqlite3 	*g_H10_Sqlite_DB_1;



int H10_db_open(const char *filename, sqlite3 **db) {
	int v_Sql_Result = sqlite3_open(filename, db);
    
	if (v_Sql_Result) {
        #ifdef G_H10_DBG_ERR
		    Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
        #endif 

		return v_Sql_Result;
	} else {
        #ifdef G_H10_DBG_PRO_1
		    Serial.printf("Opened database successfully\n");
        #endif
	}
	return v_Sql_Result;
}


static int	H10_db_callback(void *data, int argc, char **argv, char **azColName) {

	int i;
	Serial.printf("%s: ", (const char *)data);
	
	for (i = 0; i < argc; i++) {
		Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	Serial.printf("\n");
	return 0;
}

int	  E10_db_exec(sqlite3 *in_db, const char *in_sql) {

	const char *v_data = "Callback function called";

	char *v_zErrMsg = 0;

	Serial.println(in_sql);
	
	long v_time_start_ms = micros();
	
    int  v_Sql_Result	 = sqlite3_exec(in_db, in_sql, NULL, NULL, &v_zErrMsg);
    //int  v_Sql_Result	 = sqlite3_exec(in_db, in_sql, H10_db_callback, (void *)v_data, &v_zErrMsg);
	if (v_Sql_Result != SQLITE_OK) {
        #ifdef G_H10_DBG_ERR
		    Serial.printf("SQL error: %s\n", v_zErrMsg);
        #endif 

		sqlite3_free(v_zErrMsg);
	} else {
        #ifdef G_H10_DBG_PRO_1
		    Serial.printf("Operation done successfully\n");
        #endif
	}
    #ifdef G_H10_DBG_PRO_1
        Serial.print(F("Time taken:"));         Serial.println(micros() - v_time_start_ms);
    #endif 
	return v_Sql_Result;
}


void H10_DB_init(){

    #if G_H10_STORAGE_TYPE == E_H10_STORAGE_LITTLEFS        //E_H10_STORAGE_LITTLEFS, E_H10_STORAGE_SDMMC
        if (!LittleFS.begin(G_H10_FORMAT_LITTLEFS_IF_FAILED, "/littlefs")) {
            #ifdef G_H10_DBG_ERR
                Serial.println("Failed to mount file system");
            #endif 

            return;
        }

        //LittleFS.remove(G_H10_SQLITE_DB_FILENAME_1);

        if (H10_db_open(G_H10_SQLITE_DB_FILENAME_1, &g_H10_Sqlite_DB_1)){
            return;
        }


    #elif G_H10_STORAGE_TYPE == E_H10_STORAGE_SDMMC
        SPI.begin();
        SD_MMC.begin();
    #endif

	sqlite3_initialize();
}

void H10_DB_Table_Create(){
    int		 v_Sql_Result;

    //ANY, BLOB, INT, INTEGER, REAL, TEXT
    //   id INTEGER PRIMARY KEY AUTOINCREMENT,


    v_Sql_Result = E10_db_exec(
                  g_H10_Sqlite_DB_1
                , "CREATE TABLE IF NOT EXISTS test1 (id INTEGER PRIMARY KEY AUTOINCREMENT, content TEXT, ts DATETIME DEFAULT CURRENT_TIMESTAMP);"
                //  , "CREATE TABLE test1 (id INTEGER, content);"
                );
	if (v_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_H10_Sqlite_DB_1);
		return;
	}    
}


// 현재 날짜와 시간을 가져오는 함수
String H10_getCurrentDateTime() {
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    time(&now);                     // 현재 시간을 time_t 형식으로 가져오기
    localtime_r(&now, &timeinfo);   // 현재 시간을 struct tm에 저장

    // "YYYY-MM-DD HH:MM:SS" 형식으로 문자열 변환
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(strftime_buf);
}

void H10_Insert_data(){

    String currentDateTime = H10_getCurrentDateTime();

    // INSERT 구문 작성
    String insertQuery = "INSERT INTO test1 (content, ts) VALUES ('Hello, World from test1', '" + currentDateTime + "');";

    //String insertQuery = "INSERT INTO test1 (id, content, ts) VALUES (1, 'Hello, World from test1', '" + currentDateTime + "');";
    int v_Sql_Result = E10_db_exec(g_H10_Sqlite_DB_1, insertQuery.c_str());

	//v_Sql_Result = E10_db_exec(g_H10_Sqlite_DB_1, "INSERT INTO test1 VALUES (1, 'Hello, World from test1');");
	if (v_Sql_Result != SQLITE_OK) {
		sqlite3_close(g_H10_Sqlite_DB_1);
		return;
	}
}

void H10_Select_Data() {
    int		 v_Sql_Result;
    
    // 1시간 전의 날짜와 시간 가져오기

    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    time(&now);                     // 현재 시간을 time_t 형식으로 가져오기
    now -= 3600;                    // 1시간(3600초) 빼기
    localtime_r(&now, &timeinfo);   // 1시간 전 시간을 struct tm에 저장

    // "YYYY-MM-DD HH:MM:SS" 형식으로 문자열 변환
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    // SELECT 구문 작성
    String selectQuery = "SELECT id, content, ts FROM test1 WHERE ts > '" + String(strftime_buf) + "';";

    // 실행
    //Serial.println(selectQuery); // 쿼리 출력
    //v_Sql_Result = E10_db_exec(g_H10_Sqlite_DB_1, selectQuery.c_str());


    //const char *sql = "SELECT id, content, ts FROM test1;";
    sqlite3_stmt *v_stmt;

    // Prepare the SQL statement
    
    int v_Sql_Result = sqlite3_prepare_v2(g_H10_Sqlite_DB_1, selectQuery.c_str(), -1, &v_stmt, NULL);
    //int v_Sql_Result = sqlite3_prepare_v2(g_H10_Sqlite_DB_1, sql, -1, &stmt, NULL);
    if (v_Sql_Result != SQLITE_OK) {
        Serial.printf("Failed to prepare statement: %s\n", sqlite3_errmsg(g_H10_Sqlite_DB_1));
        return;
    }

    Serial.println("id, content, ts");

    // Execute the statement and fetch results
    // SQLITE_ROW : sqlite3_step() has another row ready
    while ((v_Sql_Result = sqlite3_step(v_stmt)) == SQLITE_ROW) {
        int         id      = sqlite3_column_int(v_stmt, 0);                    // 첫 번째 컬럼 (id)
        const char *content = (const char *)sqlite3_column_text(v_stmt, 1);     // 두 번째 컬럼 (content)
        const char *ts      = (const char *)sqlite3_column_text(v_stmt, 2);     // 세 번째 컬럼 (ts)

        // Print the results
        Serial.printf("%d, %s, %s\n", id, content, ts);
        //Serial.printf("id = %d, content = %s, ts = %s\n", id, content, ts);
    }

    // Finalize the statement to release resources
    sqlite3_finalize(v_stmt);
}


void H10_init(){
	int		 v_Sql_Result;

    H10_DB_init();

    H10_DB_Table_Create();

    H10_Insert_data();
    
    


    H10_Select_Data();

    sqlite3_close(g_H10_Sqlite_DB_1);

}

void H10_run(){

}
