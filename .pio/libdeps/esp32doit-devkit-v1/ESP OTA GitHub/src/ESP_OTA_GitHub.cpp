/*
  ESP_OTA_GitHub.cpp - ESP library for auto updating code from GitHub releases.
  Created by Gavin Smalley, November 13th 2019.
  Released under the LGPL v2.1.
  It is the author's intention that this work may be used freely for private
  and commercial use so long as any changes/improvements are freely shared with
  the community under the same terms.
*/

#include "ESP_OTA_GitHub.h"

ESPOTAGitHub::ESPOTAGitHub(const char* user, const char* repo, const char* currentTag, const char* binFile, bool preRelease) {
    _user = user;
    _repo = repo;
    _currentTag = currentTag;
    _binFile = binFile;
    _preRelease = preRelease;
    _lastError = "";
    _upgradeURL = "";
}

/* Private methods */

urlDetails_t ESPOTAGitHub::_urlDetails(String url) {
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

bool ESPOTAGitHub::_resolveRedirects() {
    urlDetails_t splitURL = _urlDetails(_upgradeURL);
    String proto = splitURL.proto;
    const char* host = splitURL.host.c_str();
    uint16_t port = splitURL.port;
    String path = splitURL.path;
    bool isFinalURL = false;

    WiFiClient client;

    while (!isFinalURL) {
        if (!client.connect(host, port)) {
            _lastError = "Connection Failed.";
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
				if (response.startsWith("location: ")) {
					location.replace("location: ", "");
				} else {
					location.replace("Location: ", "");
				}
                location.remove(location.length() - 1);

                if (location.startsWith("http://") || location.startsWith("https://")) {
                    //absolute URL - separate host from path
                    urlDetails_t url = _urlDetails(location);
                    proto = url.proto;
                    host = url.host.c_str();
                    port = url.port;
                    path = url.path;
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
        _upgradeURL = finalURL;
        return true;
    } else {
        _lastError = "CONNECTION FAILED";
        return false;
    }
}

// Set time via NTP, as required for x.509 validation
void ESPOTAGitHub::_setClock() {
	configTime(0, 0, GHOTA_NTP1, GHOTA_NTP2);  // UTC

	time_t now = time(nullptr);
	while (now < 8 * 3600 * 2) {
		yield();
		delay(500);
		now = time(nullptr);
	}

	struct tm timeinfo;
	gmtime_r(&now, &timeinfo);
}

/* Public methods */

bool ESPOTAGitHub::checkUpgrade() {
	
	_setClock(); // Clock needs to be set to perform certificate checks
	
    WiFiClient client;
    if (!client.connect(GHOTA_HOST, GHOTA_PORT)) {
        _lastError = "Connection failed";
        return false;
    }else {
        Serial.println("Connected to host");
    }
	  
    String url = "/repos/";
    url += _user;
    url += "/";
    url += _repo;
    url += "/releases/latest";
    Serial.println(url);
    client.printf("%s %s %s\r\n", "GET", url, "HTTP/1.1");
    client.printf("%s %s\r\n",         "Host:", GHOTA_HOST);
    client.printf("%s\r\n",         "User-Agent: ESP_OTA_GitHubArduinoLibrary"); 
    client.printf("\n%s\n",         "Content-Type: application/json");
    //client.printf("%s\n",         "Connection: close");
    String response;
    while (client.connected()){
        while (client.available()) {
            response = client.readString();
            Serial.print(response);  
        }
    }
    client.printf("%s\n",         "Connection: close");
    Serial.println(response.length());
    //client->stop();
	  
	// --- ArduinoJSON v6 --- //
	 
	// Get from https://arduinojson.org/v6/assistant/
	const size_t capacity = JSON_ARRAY_SIZE(3) + 3*JSON_OBJECT_SIZE(13) + 5*JSON_OBJECT_SIZE(18) + 5560;

	DynamicJsonDocument doc(capacity);
	  
	DeserializationError error = deserializeJson(doc, response);
	  
	if (!error) {
		if (doc.containsKey("tag_name")) {
			const char* release_tag = doc["tag_name"];
            const char* release_name = doc["name"];
            bool release_prerelease = doc["prerelease"];
			if (strcmp(release_tag, _currentTag) != 0) {
				if (!_preRelease) {
					if (release_prerelease) {
						_lastError = "Latest release is a pre-release and GHOTA_ACCEPT_PRERELEASE is set to false.";
						return false;
					}
				}
				JsonArray assets = doc["assets"];
				bool valid_asset = false;
				for (auto asset : assets) {
					const char* asset_type = asset["content_type"];
					const char* asset_name = asset["name"];
					const char* asset_url = asset["browser_download_url"];
					  
					if (strcmp(asset_type, GHOTA_CONTENT_TYPE) == 0 && strcmp(asset_name, _binFile) == 0) {
						_upgradeURL = asset_url;
						valid_asset = true;
					} else {
						valid_asset = false;
					}
				}
				if (valid_asset) {
					return true;
				} else {
					_lastError = "No valid binary found for latest release.";
					return false;
				}
			} else {
				_lastError = "Already running latest release.";
                return false;
			}
		} else {
			_lastError = "JSON didn't match expected structure. 'tag_name' missing.";
            return false;
		}
	} else {
		_lastError = "Failed to parse JSON."; // Error was: " + error.c_str();
        return false;
	}
	// --- END ArduinoJSON v6 --- //
}

bool ESPOTAGitHub::doUpgrade() {
    _upgradeURL = "https://github.com/MenitoX/OTA-testing/releases/download/1.0.0/firmware.bin";
    /*
    if (_upgradeURL == "") {
        //_lastError = "No upgrade URL set, run checkUpgrade() first.";
        //return false;
		
		if(!checkUpgrade()) {
			return false;
		}
    } else {
		_setClock(); // Clock needs to be set to perform certificate checks
		// Don't need to do this if running check upgrade first, as it will have just been done there.
	}

	_resolveRedirects();
	
    urlDetails_t splitURL = _urlDetails(_upgradeURL);
	*/
    const char *cert = \
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
    "-----END CERTIFICATE-----\n";

    WiFiClientSecure client;
    client.setCACert(cert);
    httpUpdate.setLedPin(LED_BUILTIN, LOW);
    t_httpUpdate_return ret = httpUpdate.update(client, _upgradeURL);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            _lastError = httpUpdate.getLastErrorString();
            return false;

        case HTTP_UPDATE_NO_UPDATES:
            _lastError = "HTTP_UPDATE_NO_UPDATES";
            return false;

        case HTTP_UPDATE_OK:
            _lastError = "HTTP_UPDATE_OK";
            return true;
    }
}

String ESPOTAGitHub::getLastError() {
    return _lastError;
}

String ESPOTAGitHub::getUpgradeURL() {
    return _upgradeURL;
}
