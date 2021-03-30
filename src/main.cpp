#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "HTTPClient.h"
#include "HTTPUpdate.h"

static const char *ssid     = "Elvis12345";  // your network SSID (name of wifi network)
static const char *password = "310710122812"; // your network password

#define GHOTA_USER "MenitoX"
#define GHOTA_REPO "OTA-testing"
#define GHOTA_CURRENT_TAG "1.0.0"
#define GHOTA_BIN_FILE "firmware.bin"
#define GHOTA_ACCEPT_PRERELEASE 0

WiFiClientSecure client;
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFBjCCBK2gAwIBAgIQDovzdw2S0Zbwu2H5PEFmvjAKBggqhkjOPQQDAjBnMQsw\n" \
"CQYDVQQGEwJVUzEXMBUGA1UEChMORGlnaUNlcnQsIEluYy4xPzA9BgNVBAMTNkRp\n" \
"Z2lDZXJ0IEhpZ2ggQXNzdXJhbmNlIFRMUyBIeWJyaWQgRUNDIFNIQTI1NiAyMDIw\n" \
"IENBMTAeFw0yMTAzMjUwMDAwMDBaFw0yMjAzMzAyMzU5NTlaMGYxCzAJBgNVBAYT\n" \
"AlVTMRMwEQYDVQQIEwpDYWxpZm9ybmlhMRYwFAYDVQQHEw1TYW4gRnJhbmNpc2Nv\n" \
"MRUwEwYDVQQKEwxHaXRIdWIsIEluYy4xEzARBgNVBAMTCmdpdGh1Yi5jb20wWTAT\n" \
"BgcqhkjOPQIBBggqhkjOPQMBBwNCAASt9vd1sdNJVApdEHG93CUGSyIcoiNOn6H+\n" \
"udCMvTm8DCPHz5GmkFrYRasDE77BI3q5xMidR/aW4Ll2a1A2ZvcNo4IDOjCCAzYw\n" \
"HwYDVR0jBBgwFoAUUGGmoNI1xBEqII0fD6xC8M0pz0swHQYDVR0OBBYEFCexfp+7\n" \
"JplQ2PPDU1v+MRawux5yMCUGA1UdEQQeMByCCmdpdGh1Yi5jb22CDnd3dy5naXRo\n" \
"dWIuY29tMA4GA1UdDwEB/wQEAwIHgDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYB\n" \
"BQUHAwIwgbEGA1UdHwSBqTCBpjBRoE+gTYZLaHR0cDovL2NybDMuZGlnaWNlcnQu\n" \
"Y29tL0RpZ2lDZXJ0SGlnaEFzc3VyYW5jZVRMU0h5YnJpZEVDQ1NIQTI1NjIwMjBD\n" \
"QTEuY3JsMFGgT6BNhktodHRwOi8vY3JsNC5kaWdpY2VydC5jb20vRGlnaUNlcnRI\n" \
"aWdoQXNzdXJhbmNlVExTSHlicmlkRUNDU0hBMjU2MjAyMENBMS5jcmwwPgYDVR0g\n" \
"BDcwNTAzBgZngQwBAgIwKTAnBggrBgEFBQcCARYbaHR0cDovL3d3dy5kaWdpY2Vy\n" \
"dC5jb20vQ1BTMIGSBggrBgEFBQcBAQSBhTCBgjAkBggrBgEFBQcwAYYYaHR0cDov\n" \
"L29jc3AuZGlnaWNlcnQuY29tMFoGCCsGAQUFBzAChk5odHRwOi8vY2FjZXJ0cy5k\n" \
"aWdpY2VydC5jb20vRGlnaUNlcnRIaWdoQXNzdXJhbmNlVExTSHlicmlkRUNDU0hB\n" \
"MjU2MjAyMENBMS5jcnQwDAYDVR0TAQH/BAIwADCCAQUGCisGAQQB1nkCBAIEgfYE\n" \
"gfMA8QB2ACl5vvCeOTkh8FZzn2Old+W+V32cYAr4+U1dJlwlXceEAAABeGq/vRoA\n" \
"AAQDAEcwRQIhAJ7miER//DRFnDJNn6uUhgau3WMt4vVfY5dGigulOdjXAiBIVCfR\n" \
"xjK1v4F31+sVaKzyyO7JAa0fzDQM7skQckSYWQB3ACJFRQdZVSRWlj+hL/H3bYbg\n" \
"IyZjrcBLf13Gg1xu4g8CAAABeGq/vTkAAAQDAEgwRgIhAJgAEkoJQRivBlwo7x67\n" \
"3oVsf1ip096WshZqmRCuL/JpAiEA3cX4rb3waLDLq4C48NSoUmcw56PwO/m2uwnQ\n" \
"prb+yh0wCgYIKoZIzj0EAwIDRwAwRAIgK+Kv7G+/KkWkNZg3PcQFp866Z7G6soxo\n" \
"a4etSZ+SRlYCIBSiXS20Wc+yjD111nPzvQUCfsP4+DKZ3K+2GKsERD6d\n" \
"-----END CERTIFICATE-----\n" \
"";


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

void setup(){
    Serial.begin(115200);
    Serial.print("Connecting to WiFi... ");
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if ((WiFi.status() != WL_CONNECTED)) {
		Serial.print("... ");
	}
	Serial.println();
    const char*upgradeURL = "https://github.com/MenitoX/OTA-testing/releases/download/1.0.0/firmware.bin";
    client.setCACert(rootCACertificate);
    //client.setCertificate(test_client_cert);
    //client.setPrivateKey(test_client_key);
    httpUpdate.setLedPin(LED_BUILTIN, LOW);
    //httpUpdate.update(client, upgradeURL);
    if(!client.connect("https://github.com", 443)){
        Serial.println("No conection to host");
    }
    httpUpdate.update(client,upgradeURL);
    
    /* This is the actual code to check and upgrade */
    /* End of check and upgrade code */
}

void loop(){
    Serial.println("PiC0");
    delay(5000);
}