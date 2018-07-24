#include <WiFi.h>
#include <string.h>



String wifiSSID;
String wifiPasswd;

int status = WL_IDLE_STATUS;
int wifiConnectCount = 0;


WiFiServer server(80);

void setup() {

  wifiSSID = "";
  wifiPasswd = "";
 
  Serial.begin(115200);

  const char *ssid = "MyESP32AP";
  const char *password = "testpassword";
  Serial.println("AP starting");
  WiFi.softAP(ssid, password);

  server.begin();
}
 
void loop() {
 

   if (wifiSSID == "" || wifiPasswd == "") {
      handleSetupWizard();
   } else if (status != WL_CONNECTED){
      Serial.println("Connect wifi!");
      
      server.close();
      WiFi.softAPdisconnect(true);
      
      while ( status != WL_CONNECTED) { 
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(wifiSSID);
        char ssid[64];
        wifiSSID.toCharArray(ssid, sizeof(ssid));
        char passwd[64];
        wifiPasswd.toCharArray(passwd, sizeof(passwd));
        status = WiFi.begin(ssid, passwd);
        
        wifiConnectCount++;
        if (wifiConnectCount >= 3) {
          wifiConnectCount = 0;
          wifiPasswd = "";   // Reset password
        }
        delay(10000);
      }
    } else {
      Serial.println("ESP32 Connected");
      //WiFi.localIP();   //ToDo: Make conditional for Wifi status check
      delay(1000);
    }
 
}

void handleSetupWizard () {

  

  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");
    String currentLine = "";    
 
    while (client.connected()) {
       if (client.available()) {                         
        char c = client.read();
        Serial.write(c);
        if (c == '\n') { 
          if (currentLine.length() == 0) {

            String requestBody;

            while (client.available()) {
              requestBody += (char)client.read();
            }

            if (requestBody.length()) {
              parseRequestBody(requestBody);
              generateHtmlMessagePage(client, "Wifi registered");
            } else {
                generateHtmlLoginForm(client);
            }
            break;
          } else {
            if (currentLine.startsWith("GET /?")) {
              /* parse the get request data here*/
              break;
            }
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   
          currentLine += c; 
        }
      }
    }

    delay(1);
    

    client.stop();
    Serial.println("client disonnected");
  }
}


void parseRequestBody (String requestBody) {
  char str[256];
  requestBody.toCharArray(str, sizeof(str));

  
  char *tokenMem1;

  char *token = strtok_r(str, "&", &tokenMem1);
  while (token != NULL) {
    char *tokenMem2;
    char *param = token;
    char *k = strtok_r(token, "=", &tokenMem2);
    char *v = strtok_r(NULL, "=", &tokenMem2);
    String key = String(k);
    String value = String(v);
    if (key == "ssid") {
      wifiSSID = value; 
    } else if(key == "password"){
      wifiPasswd = String(value);
    }
    token = strtok_r(NULL, "&", &tokenMem1);
  }
}

