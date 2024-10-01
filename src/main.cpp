#include <Arduino.h>


#define		E10_SQLLITE_LITTLEFS

#ifdef E10_SQLLITE_LITTLEFS
	#include "E10_sqlite3_littlefs_001.h"
#endif


#define		E20_SQL_SDMMC

#ifdef E20_SQL_SDMMC
	#include "E20_sqlite3_sdmmc_001.h"
#endif



#define		E40_SQL_BULK_INSERT

#ifdef E40_SQL_BULK_INSERT
	#include "E40_sqlite3_bulk_data_insert_002.h"
#endif



#define		F10_SQLLITE_LOGGER

#ifdef F10_SQLLITE_LOGGER
	#include "F10_SqlLite_Logger_001.h"
#endif




void setup() {
	Serial.begin(115200);

	#ifdef E10_SQLLITE_LITTLEFS
		E10_init();
	#endif

	#ifdef E20_SQL_SDMMC
		E20_init();
	#endif

	#ifdef E40_SQL_BULK_INSERT
		E40_init();
	#endif


	#ifdef F10_SQLLITE_LOGGER
		F10_init();
	#endif
}

void loop() {
	#ifdef E10_SQLLITE_LITTLEFS
		E10_run();
	#endif

	#ifdef E20_SQL_SDMMC
		E20_run();
	#endif

	#ifdef E40_SQL_BULK_INSERT
		E40_run();
	#endif

	#ifdef F10_SQLLITE_LOGGER
		F10_run();
	#endif
}
