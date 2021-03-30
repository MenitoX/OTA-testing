#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "HTTPClient.h"
#include "HTTPUpdate.h"

static const char *ssid     = "Casa 123";  // your network SSID (name of wifi network)
static const char *password = "11248325"; // your network password

#define GHOTA_USER "MenitoX"
#define GHOTA_REPO "OTA-testing"
#define GHOTA_CURRENT_TAG "1.0.0"
#define GHOTA_BIN_FILE "firmware.bin"
#define GHOTA_ACCEPT_PRERELEASE 0
#define WIFI_TIMEOUT 10000

WiFiClientSecure client;
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


const char* test_client_key = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEogIBAAKCAQEA6qaQb1XaunmN3DLdzI0jhFJP9y3QD3buzjzZ3K4A3A33U+4A\n" \
"6g/2eytcx66WZGntoNcYSODDpT/wBfZ86Vc+AyDdZMMYrdN5p+vXK4f4QpNmJcAf\n" \  
"x/MQmvb9n0JaorvhGAeec8MQSLg/J7CaaflthX6u97xY+zmeGOMa12nQRxcUs98W\n" \
"e/vPSVlJZyacSWRQdEN2d27dev9WUIvm+DviDL1LxkzPPAzaQzD4rrNilINfIlOr\n" \
"SfAjdfQyDFxIJVPnizhP5BwerKse43ukoS6RCiLuQMXcRU0HdmbgpEj0kaTA8nlX\n" \
"LG86+6GWlwlYxZCd7jtnAijpPXEZhl4Ow0NSCwIDAQABAoIBAGcsdbV+en4jlEyN\n" \
"0Rhvtn4n8yEr85/ZG4dzGhf+KkKXoXg13hEMk8P0fQcPrPDZ4rIl4Xep3Fx8kH0O\n" \
"tD2lLo8RlEcJwYAszAEZobJyRqZ/SRqwL0krhtGcaEfuVQvYsFBwiSs9SZuv55T8\n" \
"Aek20wEvgp9LIZTU8L7gnJy62elnVRga0sb1bFlxroQWlqrDdO8i7vfluGa9NJom\n" \
"og+MMvx2jhQOYaYJpktDerBfqhBHzKlbVpp+FBg+kUpE5SVEV0o/kMqvt25s2FbV\n" \
"PPLAmCvUg7s3AOLaUkjeU1LLFsMU3Q0TsADnqqBUcIssAzF5LC7PrCn93ALGNmbu\n" \
"CyjMvwECgYEA910q6PteCrjdG1rpz6I7RzQ79ux3SxHG9f++VBOcEmOEtXCXoW8T\n" \
"AqeJLpmWv/a12gxhPUlw+pF0/daLhoQbAYP/f2foaw4z3RuYSEj8I9XELjs2Q1Bw\n" \
"E1sNlJ/E0RUp2y7cmKT5TbXQlgwjiEPfseYcBHtX5FcP7L53dhDIh7kCgYEA8tfF\n" \
"R8h3yUPjKbt0bI1XqWwqe3dzqQsSUr8mBUNzA3di2i9cSefh/qdd0m9H5WyazUjD\n" \
"YXEIVyzS+9fNxsx93l1xJa4W+Epr002GOSoFslFlk6zpCkybqgWYcT/N03nRdioX\n" \
"TDSF4velWzvBJ/JxZ6AcTWdQU3/f4puDOENlQeMCgYAVNVy/ewXWH4F9ne7cCYnR\n" \
"kg3uHtofipencdTsun2wKe4v7EZj5fUYTir53ttI/CsdVeBkc86G/T+tn954mw0B\n" \
"tUKLtun7e81AkOQelntfyy011YfSf6wMTMs+SfCDCLB9BWiOgd6KJiOLKeFGBqPA\n" \
"Sf1u+PBROJG5956gddHrqQKBgFOxz9lUmwwvrIwGgjKjaBIJKH8TBTa+jRsdrAHm\n" \
"ZC5OqDAgYINMOtRkZG6RUcsdaiDUMhpUNZat6S1JDC+27K6Fr+hXnq0GITFbJ6Zt\n" \
"+Daj3QlPb6XQD1pCwW5OmiaHdoxuJ0dpVreJuGDAMGsPIMyhHwACWmncyZG1HT13\n" \
"5N8XAoGAZZ604+x9o61Wu20YhlxCZ9Vw4rIW7QeSJMTRcS5hj1lDOXGUwJ5Gdqm4\n" \
"hYJ8xYza/WafDLCr/5uokb+k7UnjigAhFVGcW9SDADBBikL7k0lhu//xlA1ZZuVe\n" \
"5qovrsVMQn/KMlbe1HE6CZpngBxRBNXzoBU8XNEfWjupQ+1x8Uc=\n" \
"-----END RSA PRIVATE KEY-----\n";

const char* test_client_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIELTCCAxWgAwIBAgIUK+QWkdd7jjxqNJAAydAAMQlJfPkwDQYJKoZIhvcNAQEL\n" \
"BQAwZjELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkZha2UgU3RhdGUxFjAUBgNVBAcM\n" \
"DUZha2UgTG9jYWxpdHkxFTATBgNVBAoMDEZha2UgQ29tcGFueTETMBEGA1UEAwwK\n" \
"Z2l0aHViLmNvbTAeFw0yMTAzMjkyMTI0NTNaFw0yNjAzMjgyMTI0NTNaMGYxCzAJ\n" \
"BgNVBAYTAlVTMRMwEQYDVQQIDApGYWtlIFN0YXRlMRYwFAYDVQQHDA1GYWtlIExv\n" \
"Y2FsaXR5MRUwEwYDVQQKDAxGYWtlIENvbXBhbnkxEzARBgNVBAMMCmdpdGh1Yi5j\n" \
"b20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDqppBvVdq6eY3cMt3M\n" \
"jSOEUk/3LdAPdu7OPNncrgDcDfdT7gDqD/Z7K1zHrpZkae2g1xhI4MOlP/AF9nzp\n" \
"Vz4DIN1kwxit03mn69crh/hCk2YlwB/H8xCa9v2fQlqiu+EYB55zwxBIuD8nsJpp\n" \
"+W2Ffq73vFj7OZ4Y4xrXadBHFxSz3xZ7+89JWUlnJpxJZFB0Q3Z3bt16/1ZQi+b4\n" \
"O+IMvUvGTM88DNpDMPius2KUg18iU6tJ8CN19DIMXEglU+eLOE/kHB6sqx7je6Sh\n" \
"LpEKIu5AxdxFTQd2ZuCkSPSRpMDyeVcsbzr7oZaXCVjFkJ3uO2cCKOk9cRmGXg7D\n" \
"Q1ILAgMBAAGjgdIwgc8wgY0GA1UdIwSBhTCBgqFqpGgwZjELMAkGA1UEBhMCVVMx\n" \
"EzARBgNVBAgMCkZha2UgU3RhdGUxFjAUBgNVBAcMDUZha2UgTG9jYWxpdHkxFTAT\n" \
"BgNVBAoMDEZha2UgQ29tcGFueTETMBEGA1UEAwwKZ2l0aHViLmNvbYIUFQFDaLn/\n" \
"l1Mk6rr0dcojVwW1MOowCQYDVR0TBAIwADALBgNVHQ8EBAMCBPAwJQYDVR0RBB4w\n" \
"HIIKZ2l0aHViLmNvbYIOd3d3LmdpdGh1Yi5jb20wDQYJKoZIhvcNAQELBQADggEB\n" \
"ACGpJ460Q9MzqXCcBaf3/45VGA5BNcwdAt8Zp6VwV1YS1HtiuUKqjxAH6MjMUFRd\n" \
"0Js5W7SzzrYL8GkZyLZ3f4v2u6pBeyLK60k5K+v/jGlgbxBDUwbYadJ+7KZBgYpl\n" \
"qBCuaCW3UWAVQodW0NrUGGr9/t1CVR2jKfxXWA3v319pY99UEMkbucsz4g1QGU54\n" \
"kFoXRqvkkrQSD33uf/HDNTMPHvVs900+NlaF7XQ5ijjmymIAgs7Zo2ze+Y/+vJRr\n" \
"iuRX/+0+WSRCuiTDa+NS0dfA5qdk0jNc4FGIHgS48fhw5Ht/hzwjeGSuyW8wnxIZ\n" \
"FDJsfQJwEVL7sFLVcpHN+6A=\n" \
"-----END CERTIFICATE-----\n";

String upgradeURL = "https://github.com/MenitoX/OTA-testing/releases/download/3.0.0/firmware.bin";
//String upgradeURL = "https://github-releases.githubusercontent.com/343937274/4d519c80-8e64-11eb-82b4-76da03c1a7ec?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20210330%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20210330T145502Z&X-Amz-Expires=300&X-Amz-Signature=9bf9cbbd9b1bbcc31f6558a41b1f8e1400e8a620d17acdab6be8acdbb94640b3&X-Amz-SignedHeaders=host&actor_id=0&key_id=0&repo_id=343937274&response-content-disposition=attachment%3B%20filename%3Dfirmware.bin&response-content-type=application%2Foctet-stream";
//String upgradeURL = "https://github-releases.githubusercontent.com/343937274/4d519c80-8e64-11eb-82b4-76da03c1a7ec?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20210330%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20210330T142940Z&X-Amz-Expires=300&X-Amz-Signature=88a77c2a12c5a74c5297ec120ca11777bdc2cdc8b6ac6776052146c7d10f0ecf&X-Amz-SignedHeaders=host&actor_id=0&key_id=0&repo_id=343937274&response-content-disposition=attachment%3B%20filename%3Dfirmware.bin&response-content-type=application%2Foctet-stream";

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

void setup(){
    Serial.begin(115200);
    wifiConnection();
    client.setCACert(rootCACertificate);
    //client.setCertificate(test_client_cert);
    //client.setPrivateKey(test_client_key);
    httpUpdate.setLedPin(LED_BUILTIN, LOW);
    _resolveRedirects();
    httpUpdate.update(client,upgradeURL);
    
    /* This is the actual code to check and upgrade */
    /* End of check and upgrade code */
}

void loop(){
    Serial.println("PiC0");
    delay(5000);
}