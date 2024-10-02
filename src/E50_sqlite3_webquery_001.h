/*
	This example shows how to retrieve data from Sqlite3 databases from SD Card
	through the Web Server and display in the form of HTML page.
	It also demonstrates query filtering by parameter passing and chunked encoding.
	Before running please copy following files to SD Card:

	data/babyname.db

	This database contains around 30000 baby names and corresponding data.

	For more information, visit https://github.com/siara-cc/esp32_arduino_sqlite3_lib

	Copyright (c) 2018, Siara Logics (cc)
*/

/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
	 list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
	 list of conditions and the following disclaimer in the documentation and/or
	 other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
	 contributors may be used to endorse or promote products derived from
	 this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ESPmDNS.h>
#include <FS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <sqlite3.h>

#include "SD_MMC.h"

const char *g_E50_Wifi_ssid	 = "Nokia1";
const char *g_E50_Wifi_password = "nokiafour";

WebServer g_E50_WebServer(80);

const int g_E50_LED1_PIN = 13;

void E50_webHandle_Root() {
	digitalWrite(g_E50_LED1_PIN, 1);
	String temp;
	int	   sec = millis() / 1000;
	int	   min = sec / 60;
	int	   hr  = min / 60;

const char index_html[] PROGMEM = R"rawliteral(
<html>
<head>
    <title>ESP32 Demo(aa)</title>
</head>
<body>
    <h1>Hello, ESP32!</h1>
    <p>aaa</p>
</body>
</html>
)rawliteral";

    const char *indexHtml = R"--x--(
<html>
<head>
    <title>ESP32 Demo(aa)</title>
</head>
<body>
    <h1>Hello, ESP32!</h1>
    <p>aaa</p>
</body>
</html>
)--x--";




	temp =
		"<html><head>\
      <title>ESP32 Demo</title>\
      <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: large; Color: #000088; }\
      </style>\
  </head>\
  <body>\
      <h1>Hello from ESP32!</h1>\
      <p>Uptime: ";
	temp += hr;
	temp += ":";
	temp += min % 60;
	temp += ":";
	temp += sec % 60;
	temp +=
		"</p>\
      <h2>Query gendered names database</h2>\
      <form name='params' method='GET' action='query_db'>\
      Enter from: <input type=text style='font-size: large' value='Bob' name='from'/> \
      <br>to: <input type=text style='font-size: large' value='Bobby' name='to'/> \
      <br><br><input type=submit style='font-size: large' value='Query database'/>\
      </form>\
  </body>\
  </html>";

	g_E50_WebServer.send(200, "text/html", temp.c_str());
	digitalWrite(g_E50_LED1_PIN, 0);
}

void E50_webHandle_NotFound() {
	digitalWrite(g_E50_LED1_PIN, 1);
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += g_E50_WebServer.uri();
	message += "\nMethod: ";
	message += (g_E50_WebServer.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += g_E50_WebServer.args();
	message += "\n";

	for (uint8_t i = 0; i < g_E50_WebServer.args(); i++) {
		message += " " + g_E50_WebServer.argName(i) + ": " + g_E50_WebServer.arg(i) + "\n";
	}

	g_E50_WebServer.send(404, "text/plain", message);
	digitalWrite(g_E50_LED1_PIN, 0);
}

sqlite3		 *g_E50_Sqlite_DB_1;
int			  g_E50_Sql_Result;
sqlite3_stmt *g_E50_Sql_Prep_Statement;
int			  g_E50_Sql_Record_count = 0;
const char	 *g_E50_Sql_tail;

int E50_db_open(const char *filename, sqlite3 **db) {

	int v_Sql_Result = sqlite3_open(filename, db);
	if (v_Sql_Result) {
		Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
		return v_Sql_Result;
	} else {
		Serial.printf("Opened database successfully\n");
	}
	return v_Sql_Result;
}

void E50_Wifi_connect(){
	WiFi.mode(WIFI_STA);
	WiFi.begin(g_E50_Wifi_ssid, g_E50_Wifi_password);
	Serial.println("");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(g_E50_Wifi_ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	if (MDNS.begin("esp32")) {
		Serial.println("MDNS responder started");
	}
}

void E50_init(void) {

	pinMode(g_E50_LED1_PIN, OUTPUT);
	digitalWrite(g_E50_LED1_PIN, 0);
	
    //Serial.begin(115200);

    E50_Wifi_connect();

	SD_MMC.begin();
	sqlite3_initialize();

	// Open database
	if (E50_db_open("/sdcard/babyname.db", &g_E50_Sqlite_DB_1))
		return;

	g_E50_WebServer.on("/", E50_webHandle_Root);
    
	g_E50_WebServer.on("/query_db", []() {
		String v_sql = "Select count(*) from gendered_names where name between '";
		v_sql += g_E50_WebServer.arg("from");
		v_sql += "' and '";
		v_sql += g_E50_WebServer.arg("to");
		v_sql += "'";
		g_E50_Sql_Result = sqlite3_prepare_v2(g_E50_Sqlite_DB_1, v_sql.c_str(), -1, &g_E50_Sql_Prep_Statement, &g_E50_Sql_tail);
		if (g_E50_Sql_Result != SQLITE_OK) {
			String v_resp = "Failed to fetch data: ";
			v_resp += sqlite3_errmsg(g_E50_Sqlite_DB_1);
			v_resp += ".<br><br><input type=button onclick='location.href=\"/\"' value='back'/>";
			g_E50_WebServer.send(200, "text/html", v_resp.c_str());
			Serial.println(v_resp.c_str());
			return;
		}
		while (sqlite3_step(g_E50_Sql_Prep_Statement) == SQLITE_ROW) {
			g_E50_Sql_Record_count = sqlite3_column_int(g_E50_Sql_Prep_Statement, 0);
			if (g_E50_Sql_Record_count > 5000) {
				String v_resp = "Too many records: ";
				v_resp += g_E50_Sql_Record_count;
				v_resp += ". Please select different range";
				v_resp += ".<br><br><input type=button onclick='location.href=\"/\"' value='back'/>";
				g_E50_WebServer.send(200, "text/html", v_resp.c_str());
				Serial.println(v_resp.c_str());
				sqlite3_finalize(g_E50_Sql_Prep_Statement);
				return;
			}
		}
		sqlite3_finalize(g_E50_Sql_Prep_Statement);

		v_sql = "Select year, state, name, total_babies, primary_sex, primary_sex_ratio, per_100k_in_state from gendered_names where name between '";
		v_sql += g_E50_WebServer.arg("from");
		v_sql += "' and '";
		v_sql += g_E50_WebServer.arg("to");
		v_sql += "'";
		g_E50_Sql_Result = sqlite3_prepare_v2(g_E50_Sqlite_DB_1, v_sql.c_str(), -1, &g_E50_Sql_Prep_Statement, &g_E50_Sql_tail);
		if (g_E50_Sql_Result != SQLITE_OK) {
			String v_resp = "Failed to fetch data: ";
			v_resp += sqlite3_errmsg(g_E50_Sqlite_DB_1);
			v_resp += "<br><br><a href='/'>back</a>";
			g_E50_WebServer.send(200, "text/html", v_resp.c_str());
			Serial.println(v_resp.c_str());
			return;
		}

		g_E50_Sql_Record_count = 0;
		g_E50_WebServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
		String v_resp =
			"<html><head><title>ESP32 Sqlite local database query through web server</title>\
          <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: large; Color: #000088; }\
          </style><head><body><h1>ESP32 Sqlite local database query through web server</h1><h2>";
		v_resp += v_sql;
		v_resp += "</h2><br><table cellspacing='1' cellpadding='1' border='1'><tr><td>Year</td><td>State</td><td>Name</td><td>Total babies</td><td>Primary Sex</td><td>Ratio</td><td>Per 100k</td></tr>";
		g_E50_WebServer.send(200, "text/html", v_resp.c_str());
		while (sqlite3_step(g_E50_Sql_Prep_Statement) == SQLITE_ROW) {
			v_resp = "<tr><td>";
			v_resp += sqlite3_column_int(g_E50_Sql_Prep_Statement, 0);
			v_resp += "</td><td>";
			v_resp += (const char *)sqlite3_column_text(g_E50_Sql_Prep_Statement, 1);
			v_resp += "</td><td>";
			v_resp += (const char *)sqlite3_column_text(g_E50_Sql_Prep_Statement, 2);
			v_resp += "</td><td>";
			v_resp += sqlite3_column_int(g_E50_Sql_Prep_Statement, 3);
			v_resp += "</td><td>";
			v_resp += (const char *)sqlite3_column_text(g_E50_Sql_Prep_Statement, 4);
			v_resp += "</td><td>";
			v_resp += sqlite3_column_double(g_E50_Sql_Prep_Statement, 5);
			v_resp += "</td><td>";
			v_resp += sqlite3_column_double(g_E50_Sql_Prep_Statement, 6);
			v_resp += "</td></tr>";
			g_E50_WebServer.sendContent(v_resp);
			g_E50_Sql_Record_count++;
		}
		v_resp = "</table><br><br>Number of records: ";
		v_resp += g_E50_Sql_Record_count;
		v_resp += ".<br><br><input type=button onclick='location.href=\"/\"' value='back'/>";
		g_E50_WebServer.sendContent(v_resp);
		sqlite3_finalize(g_E50_Sql_Prep_Statement);
	});
	g_E50_WebServer.onNotFound(E50_webHandle_NotFound);
	g_E50_WebServer.begin();
	Serial.println("HTTP server started");
}

void E50_run(void) {
	g_E50_WebServer.handleClient();
}
