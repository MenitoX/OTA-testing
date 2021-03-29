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
"MIIEGzCCAwOgAwIBAgIQBmcDW7sU/WOvwNaoU07+FjANBgkqhkiG9w0BAQsFADBs\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
"ZSBFViBSb290IENBMB4XDTIwMTIxNzAwMDAwMFoXDTMwMTIxNjIzNTk1OVowZzEL\n" \
"MAkGA1UEBhMCVVMxFzAVBgNVBAoTDkRpZ2lDZXJ0LCBJbmMuMT8wPQYDVQQDEzZE\n" \
"aWdpQ2VydCBIaWdoIEFzc3VyYW5jZSBUTFMgSHlicmlkIEVDQyBTSEEyNTYgMjAy\n" \
"MCBDQTEwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARnvW/xPOudvtC252wTq9ef\n" \
"6fbdFeWPkOscfpRTkciuHj7UcumQSH3lzkPEIx0KpesWa8epsks7QwkZ4fU/Tkf9\n" \
"o4IBhzCCAYMwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUUGGmoNI1xBEq\n" \
"II0fD6xC8M0pz0swHwYDVR0jBBgwFoAUsT7DaQP4v0cB1JgmGggC72NkK8MwDgYD\n" \
"VR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjB/Bggr\n" \
"BgEFBQcBAQRzMHEwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNv\n" \
"bTBJBggrBgEFBQcwAoY9aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29tL0RpZ2lD\n" \
"ZXJ0SGlnaEFzc3VyYW5jZUVWUm9vdENBLmNydDBLBgNVHR8ERDBCMECgPqA8hjpo\n" \
"dHRwOi8vY3JsMy5kaWdpY2VydC5jb20vRGlnaUNlcnRIaWdoQXNzdXJhbmNlRVZS\n" \
"b290Q0EuY3JsMDAGA1UdIAQpMCcwCAYGZ4EMAQICMAgGBmeBDAECAzAHBgVngQwB\n" \
"ATAIBgZngQwBAgEwDQYJKoZIhvcNAQELBQADggEBAHMQH8hhiBfNbxwEwxbbTAnu\n" \
"jPyUh/oi0JrfZI3u9JuiLqca720D6foS/AB5+4EIxpm7CMG4MdN/l7oAiDipaCPv\n" \
"mOmpYUpnT7A63Cr0q4g84rI1ZmdqA40lVUUf6qC6E34tC73qDQF8TJSrfscWFdCl\n" \
"RXR9J4QGrkZ2VNMSDzlDRzWCaA95MfO8x01l+ZdopdE8FvM78gGd4zxeWb8v991+\n" \
"mBxTDepqKuy/jF5Rm6Bhfxr33ADRs60s1t16dtZ3pOYLALBTPD5KhZ6a+/dk5dnh\n" \
"6c4PaeZQYBUAh+GuxfaBlU4qQ8EtjBMCQHreMIwXHYHW5FRYGjgR4NMuaIw2jD0=\n" \
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
    httpUpdate.update(client, upgradeURL);
    /* This is the actual code to check and upgrade */
    /* End of check and upgrade code */
}

void loop(){
    Serial.println("PiC0");
    delay(5000);
}