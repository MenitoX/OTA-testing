#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "HTTPClient.h"
#include "HTTPUpdate.h"
#define TINY_GSM_MODEM_SIM800      // SIM Module model is set
#define TINY_GSM_RX_BUFFER   1024  // RX Buffer memory is set to 1024 Bytes
#define CERT_FILE "C:\\USER\\CERT.CRT"
#include <TinyGsmClient.h>
#include <Update.h>
#include "SPIFFS.h"
//#include "DSTRootCAX3.h"
/*-------------------------------* Declarations *------------------------------------*/

TinyGsm modem(Serial1);
TinyGsmClient client(modem);

static const char *ssid     = "Elvis12345";  // your network SSID (name of wifi network)
static const char *password = "310710122812"; // your network password

struct MovilOperator 
{
  char *apn;
  char *user;
  char *pass;
};
//////////////////////
bool initGprsModem();
int waitSignal(int requiredCsq);
String codeToSignal(int code);
int signalStrength();
void sleepModem();
bool connectGprs(struct MovilOperator movilOperator);
bool disconnectGprs();
void createOperator(struct MovilOperator *movilOperator, String apn, String user, String pass);
bool beginMemory();
void performUpdate(Stream &updateSource, size_t updateSize);
void updateFromFS();
//////////////////////

bool initGprsModem(){
  modem.restart();
  bool state;
  int csq = 2;
  int Signal = waitSignal(csq);

  if(Signal >= csq){
    Serial.printf("%s %s ","Signal strength:", codeToSignal(Signal).c_str());
    state = modem.waitForNetwork();
  }else{
    state = false;
    Serial.printf("%s ","Signal strength too low");
  }
  return state;
}

int waitSignal(int requiredCsq){
  int csq = 0;
  int max_wait = 100000;
  long curMillis = 0;
  long preMillis = millis();

  while ((csq < requiredCsq) && (curMillis - preMillis < max_wait)){
    curMillis = millis();
    csq = signalStrength();
    Serial.print(".");
    delay(100);
  }

  if(csq < requiredCsq){
    Serial.println("Failed!");
    return 0;
  }else{
    Serial.println("Connected!");
  }

  return csq;
}

String codeToSignal(int code){

  String response = "";
  if(code >=2 && code <=9){
    response = "Marginal";
  }
  else if(code >=10 && code <=14){
    response = "OK";
  }
  else if(code >=15 && code <=19){
    response = "Good";
  }
  else if(code >=20 && code <=30){
    response = "Excellent";
  }
  return response;
}

int signalStrength(){

  int csq = modem.getSignalQuality();

  return csq;
}

/**
 * Init modem by restarting the device
*/


/**
 * Sleep modem
*/
void sleepModem(){
  modem.radioOff();
  modem.sleepEnable(); //SIM en modo deepsleep
}

/**
 * Establishes a GPRS connection with a mobile operator to a server
*/
bool connectGprs(struct MovilOperator movilOperator){
  return modem.gprsConnect(movilOperator.apn, movilOperator.user, movilOperator.pass);
}

/**
 * Disconnect GPRS connection
*/
bool disconnectGprs(){
  client.stop();
  return modem.gprsDisconnect();
}

/**
 * Create a new MovilOperator structure from the name, apn, user and its password.
*/
void createOperator(struct MovilOperator *movilOperator, String apn, String user, String pass){
  movilOperator->apn  = (char*) malloc((apn.length() + 1) * sizeof(char));
  movilOperator->user = (char*) malloc((user.length() + 1) * sizeof(char));
  movilOperator->pass = (char*) malloc((pass.length() + 1) * sizeof(char));
  apn.toCharArray(movilOperator->apn, apn.length() + 1);
  user.toCharArray(movilOperator->user, user.length() + 1);
  pass.toCharArray(movilOperator->pass, pass.length() + 1);
}

bool beginMemory(){
  return SPIFFS.begin(true);
}


#define GHOTA_USER "MenitoX"
#define GHOTA_REPO "OTA-testing"
#define GHOTA_CURRENT_TAG "1.0.0"
#define GHOTA_BIN_FILE "firmware.bin"
#define GHOTA_ACCEPT_PRERELEASE 0
#define WIFI_TIMEOUT 10000
#define GHOTA_HOST "https://github-releases.githubusercontent.com"
#define GHOTA_PORT 443

void performUpdate(Stream &updateSource, size_t updateSize)
{
    if (Update.begin(updateSize))
    {
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize)
        {
            Serial.println("Escritos : " + String(written) + " successfully");
        }
        else
        {
            Serial.println("Solamente escritos : " + String(written) + "/" + String(updateSize) + ". Retry?");
        }
        if (Update.end())
        {
            Serial.println("OTA realizado!");
            if (Update.isFinished())
            {
                Serial.println("Ota exitoso, reiniciando!");
                ESP.restart();
            }
            else
            {
                Serial.println("Ota no terminó? Algo salió mal!");
            }
        }
        else
        {
            Serial.println("Ocurrió Error #: " + String(Update.getError()));
        }
    }
    else
    {
        Serial.println("Sin espacio suficiente para hacer OTA");
    }
}


void updateFromFS()
{
    File updateBin = SPIFFS.open("/update.bin");
    if (updateBin)
    {
        if (updateBin.isDirectory())
        {
            Serial.println("Error, en el directorio");
            updateBin.close();
            return;
        }

        size_t updateSize = updateBin.size();

        if (updateSize > 0)
        {
            Serial.println("Intentando comenzar Actualización");
            performUpdate(updateBin, updateSize);
        }
        else
        {
            Serial.println("Error, archivo vacío");
        }

        updateBin.close();

        // whe finished remove the binary from sd card to indicate end of the process
        //fs.remove("/update.bin");
    }
    else
    {
        Serial.println("No se puede cargar el archivo");
    }
}



//WiFiClientSecure client;
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \
"+OkuE6N36B9K\n" \
"-----END CERTIFICATE-----\n";

//String upgradeURL = "https://github.com/MenitoX/OTA-testing/releases/download/1.0.0/firmware.bin";
String upgradeURL = "https://github-releases.githubusercontent.com/343937274/4d519c80-8e64-11eb-82b4-76da03c1a7ec?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20210421%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20210421T215451Z&X-Amz-Expires=300&X-Amz-Signature=ed6606f71eeef4cce404a213fa82f4db57ea43f39327d0ef1dc64f0bcea5ed12&X-Amz-SignedHeaders=host&actor_id=0&key_id=0&repo_id=343937274&response-content-disposition=attachment%3B%20filename%3Dfirmware.bin&response-content-type=application%2Foctet-stream";
//String upgradeURL = "https://github-releases.githubusercontent.com/343937274/4d519c80-8e64-11eb-82b4-76da03c1a7ec?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20210330%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20210330T145502Z&X-Amz-Expires=300&X-Amz-Signature=9bf9cbbd9b1bbcc31f6558a41b1f8e1400e8a620d17acdab6be8acdbb94640b3&X-Amz-SignedHeaders=host&actor_id=0&key_id=0&repo_id=343937274&response-content-disposition=attachment%3B%20filename%3Dfirmware.bin&response-content-type=application%2Foctet-stream";
//String upgradeURL = "https://github-releases.githubusercontent.com/343937274/4d519c80-8e64-11eb-82b4-76da03c1a7ec?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20210330%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20210330T142940Z&X-Amz-Expires=300&X-Amz-Signature=88a77c2a12c5a74c5297ec120ca11777bdc2cdc8b6ac6776052146c7d10f0ecf&X-Amz-SignedHeaders=host&actor_id=0&key_id=0&repo_id=343937274&response-content-disposition=attachment%3B%20filename%3Dfirmware.bin&response-content-type=application%2Foctet-stream";
const char resource[] = "/343937274/4d519c80-8e64-11eb-82b4-76da03c1a7ec?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20210421%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20210421T215451Z&X-Amz-Expires=300&X-Amz-Signature=ed6606f71eeef4cce404a213fa82f4db57ea43f39327d0ef1dc64f0bcea5ed12&X-Amz-SignedHeaders=host&actor_id=0&key_id=0&repo_id=343937274&response-content-disposition=attachment%3B%20filename%3Dfirmware.bin&response-content-type=application%2Foctet-stream"; 

typedef struct urlDetails_t {
    String proto;
    String host;
	int port;
    String path;
};

void wifiConnection(){
    Serial.print("Connecting to wifi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    unsigned long strartAttempTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - strartAttempTime < WIFI_TIMEOUT) {
        Serial.print(".");
        delay(100);
    }
  
    if(WiFi.status() != WL_CONNECTED){
        Serial.print("failed!\n");
        WiFi.mode(WIFI_OFF);
    }else{
        Serial.print("connected!");
        Serial.print(WiFi.localIP());
    }
}

urlDetails_t _urlDetails(String url) {
    String proto = "";
    int port = 80;
    if (url.startsWith("http://")) {
        proto = "http://";
        url.replace("http://", "");
    } else {
        proto = "https://";
        port = 443;
        url.replace("https://", "");
    }
    int firstSlash = url.indexOf('/');
    String host = url.substring(0, firstSlash);
    String path = url.substring(firstSlash);

    urlDetails_t urlDetail;

    urlDetail.proto = proto;
    urlDetail.host = host;
    urlDetail.port = port;
    urlDetail.path = path;

    return urlDetail;
}

bool _resolveRedirects() {
    urlDetails_t splitURL = _urlDetails(upgradeURL);
    String proto = splitURL.proto;
    const char* host = splitURL.host.c_str();
    uint16_t port = splitURL.port;
    String path = splitURL.path;
    bool isFinalURL = false;

    Serial.println(proto);
    Serial.println(host);
    Serial.println(port);
    Serial.println(path);
    //wifiConnection();

    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    client.setInsecure();
    while (!isFinalURL) {
        Serial.printf("Host1 is: %s\n", host);
        
        if (!client.connect(host, port)) {
            Serial.println("Connection failed (142)");
            return false;
        }

        client.print(String("GET ") + path + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "User-Agent: ESP_OTA_GitHubArduinoLibrary\r\n" +
            "Connection: close\r\n\r\n");

        while (client.connected()) {
            String response = client.readStringUntil('\n');
            if (response.startsWith("location: ") || response.startsWith("Location: ")) {
                isFinalURL = false;
                String location = response;
                Serial.printf("la response is: %s\n", response.c_str());
				if (response.startsWith("location: ")) {
					location.replace("location: ", "");
				} else {
					location.replace("Location: ", "");
				}
                location.remove(location.length() - 1);

                if (location.startsWith("http://") || location.startsWith("https://")) {
                    //absolute URL - separate host from path
                    Serial.println(location);
                    urlDetails_t url = _urlDetails(location);
                    proto = url.proto;
                    host = url.host.c_str();
                    Serial.printf("Host is: %s\n", host);
                    port = url.port;
                    path = url.path;

                    String finalURL = proto + host + path;
                    upgradeURL = finalURL;
                    return true;
                } else {
					//relative URL - host is the same as before, location represents the new path.
                    path = location;
                }
                //leave the while loop so we don't set isFinalURL on the next line of the header.
                break;
            } else {
                //location header not present - this must not be a redirect. Treat this as the final address.
                isFinalURL = true;
            }
            if (response == "\r") {
                break;
            }
        }
    }

    if(isFinalURL) {
        String finalURL = proto + host + path;
        upgradeURL = finalURL;
        return true;
    } else {
        Serial.println("CONNECTION FAILED (191)");
        return false;
    }
}

struct MovilOperator movilOperator;
String apn = "em" ; 
String user = ""; 
String pass = "";

uint32_t knownCRC32 = 0x6f50d767;
uint32_t knownFileSize = 1024; // In case server does not send it

void setup(){
    Serial.begin(115200);
    beginMemory();
    createOperator(&movilOperator, apn, user, pass);
    Serial.println("1 OK");
    if(initGprsModem()){
        Serial.println("Reboot success!");
    }
    Serial.println("2 OK");
    modem.sendAT(GF("+FSCREATE=" CERT_FILE));
    if (modem.waitResponse() != 1) return;

    const int cert_size = sizeof(rootCACertificate);

    modem.sendAT(GF("+FSWRITE=" CERT_FILE ",0,"), cert_size, GF(",10"));
    if (modem.waitResponse(GF(">")) != 1) {
        return;
    }

    for (int i = 0; i < cert_size; i++) {
        char c = pgm_read_byte(&rootCACertificate[i]);
        modem.stream.write(c);
    }

    modem.stream.write(GSM_NL);
    modem.stream.flush();

    if (modem.waitResponse(2000) != 1) return;

    modem.sendAT(GF("+SSLSETCERT=\"" CERT_FILE "\""));
    if (modem.waitResponse() != 1) return;
    if (modem.waitResponse(5000L, GF(GSM_NL "+SSLSETCERT:")) != 1) return;
    const int retCode = modem.stream.readStringUntil('\n').toInt();
    
    initGprsModem();
    if (!connectGprs(movilOperator))
    {
        Serial.println(" fail");
        delay(10000);
        return;
    }
    Serial.println(" OK");

    Serial.print("Connecting to ");
    Serial.print(GHOTA_HOST);

    // if you get a connection, report back via serial:
    if (!client.connect(GHOTA_HOST, GHOTA_PORT))
    {
        Serial.println(" fail");
        delay(10000);
        return;
    }
    Serial.println(" OK");
    // Make a HTTP request:
    client.print(String("GET ") + resource + " HTTP/1.0\r\n");
    client.print(String("Host: ") + GHOTA_HOST + "\r\n");
    client.print("Connection: close\r\n\r\n");

    long timeout = millis();
    while (client.available() == 0)
    {
        if (millis() - timeout > 5000L)
        {
            Serial.println(">>> Client Timeout !");
            client.stop();
            delay(10000L);
            return;
        }
    }

    Serial.println("Recibiendo Header");
    uint32_t contentLength = knownFileSize;

    File file = SPIFFS.open("/update.bin", FILE_APPEND);

    while (client.available())
    {
        String line = client.readStringUntil('\n');
        line.trim();
        //Serial.println(line);    // Uncomment this to show response header
        line.toLowerCase();
        if (line.startsWith("content-length:"))
        {
            contentLength = line.substring(line.lastIndexOf(':') + 1).toInt();
        }
        else if (line.length() == 0)
        {
            break;
        }
    }

    Serial.println("Recibiendo respuesta");
    timeout = millis();
    uint32_t readLength = 0;
    //CRC32 crc;

    unsigned long timeElapsed = millis();
    //printPercent(readLength, contentLength);

    while (readLength < contentLength && client.connected() && millis() - timeout < 10000L)
    {
        int i = 0;
        while (client.available())
        {

            if (!file.print(char(client.read())))
            {
                Serial.println("Fallo Append");
            }
            //Serial.print((char)c);       // Uncomment this to show data
            //crc.update(c);
            readLength++;

            if (readLength % (contentLength / 13) == 0)
            {
                //printPercent(readLength, contentLength);
            }
            timeout = millis();
        }
    }

    file.close();

    //printPercent(readLength, contentLength);
    timeElapsed = millis() - timeElapsed;
    Serial.println();

    client.stop();
    Serial.println("Desconectado del Servidor");

    modem.gprsDisconnect();
    Serial.println("GPRS Desconectado");
    Serial.println();

    float duration = float(timeElapsed) / 1000;
    /*
    Serial.print("Tamaño de Archivo: ");
    Serial.println(contentLength);
    Serial.print("Leido:  ");
    Serial.println(readLength);
    Serial.print("Calculado. CRC32:    0x");
    Serial.println(crc.finalize(), HEX);
    Serial.print("Conocido CRC32:    0x");
    Serial.println(knownCRC32, HEX);
    Serial.print("Bajado en:       ");
    Serial.print(duration);
    Serial.println("s");
    */
    Serial.println("Se genera una espera de 3 segundos");
    for (int i = 0; i < 3; i++)
    {
        Serial.print(String(i) + "...");
        delay(1000);
    }

    //readFile(SPIFFS, "/update.bin");

    updateFromFS();

    // Do nothing forevermore
    while (true)
    {
        delay(1000);
    }





    //wifiConnection();
    //client.setCACert(rootCACertificate);
    //client.setCertificate(test_client_cert);
    //client.setPrivateKey(test_client_key);
    //httpUpdate.setLedPin(LED_BUILTIN, LOW);
    //_resolveRedirects();
    //httpUpdate.update(client,upgradeURL);
    

    /* This is the actual code to check and upgrade */
    /* End of check and upgrade code */
}

void loop(){
    Serial.println("PiC0");
    delay(5000);
}