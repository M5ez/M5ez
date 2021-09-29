## Over The Air (OTA) firmware updates via https

OTA stands for Over The Air, and in the context of the M5Stack usually means we're upgrading compiled firmware via the WiFi network, without hooking the USB port to a computer and running the Arduino IDE. The ez.wifi.update routine that we will be using to update the firmware uses https to transport the firmware, because code that is going to be running on all sorts of devices should be encrypted.

![](../../images/ezProgressBar.png)

### Using the demo

Load this program in your Arduino IDE and load it to your M5Stack. The menu will allow you to get the larger M5ez-demo program from the `/compiled_binaries` directory on the M5ez Github repository and replaces this `OTA_http` demo sketch with it. There's a menu item that points to M5ez's built-in WiFi menu in case you haven't set up any autoconnecting networks with M5ez before.

(As it happens the M5ez-demo program allows you to load a compiled version of this demo program again, so you can Over-The-Air update back and forth. Not because that's a useful things to do, but it does serve to demonstrate the mechanism.)

### Using `ez.wifi.update` in your own code

The steps below may seem a bit complicated. Most of that is due to the need to include a root certyificate for the server we will be connecting to. This is to prevent evildoers on the internet from swapping their malicious firmware for yours, somewhere along the way. And that was the whole point of using https over some unencrypted method in the first place. 

In case you want to use it from your own code, follow the follwing steps:

* Export your firmare binary by selecting "Sketch / Export compiled binary in the Arduino IDE. The binary will be in the sketch directory once compilation is done.

* Put it on the web at some server that supports https. Someplace on Github works well.

* Let's assume we want our code to get the `M5ez-demo.bin` image from the `/compiled_binaries` directory of this repository. We then [browse](https://github.com/M5ez/M5ez/blob/master/compiled_binaries/M5ez-demo.bin) to that, and get a download link called "[view raw](https://github.com/M5ez/M5ez/blob/master/compiled_binaries/M5ez-demo.bin?raw=true)" which, if we right-click and hit "save link as" points to `https://github.com/M5ez/M5ez/blob/master/compiled_binaries/M5ez-demo.bin?raw=true`.

* Download the `get_cert` script from the `/tools` directory on this repository to a MacOS or Linux computer, or a linux-like shell on your windows machine.

* Make the script executable with `chmod a+x get_cert`.

* Make sure you have the `opensll` command-line utility, or download and install it if you don't. 

* Run `./get_cert https://github.com/M5ez/M5ez/blob/master/compiled_binaries/M5ez-demo.bin?raw=true`, or in other words, supply the URL that you want to download from.

```
your_prompt$ ./get_cert https://github.com/M5ez/M5ez/blob/master/compiled_binaries/M5ez-demo.bin?raw=true
The effective download URL (after resolving forwards) is:
    https://raw.githubusercontent.com/M5ez/M5ez/master/compiled_binaries/M5ez-demo.bin

The root certificate include file is saved as:
    raw_githubusercontent_com.h
```

* As you can see the script tells you two things. First it will tell you what the "real" URL is. That is: what site and path does the first URL evetually redirect to. That is the URL you will want to use in your code, and that is the domain you want the root certificate for.

* It will also have saved a `.h` include file named after the domain (with dots replaced by underscores) that holds the root certificate in a format ready to be used by our code. Including that anywhere in your code will be replaced by a definition of a `const char *` variable called `root_cert` that can be passed to `ez.wifi.update`. Here's what that file looks like:

```
your_prompt$ cat raw_githubusercontent_com.h 
// This is the root certificate include file for raw.githubusercontent.com
// as obtained by the get_cert script on: Wed Aug 15 21:34:20 CEST 2018
//
//
// Certificate info:
//     issuer= /C=US/O=DigiCert Inc/OU=www.digicert.com/CN=DigiCert High Assurance EV Root CA
//     notAfter=Oct 22 12:00:00 2028 GMT
//

const char* root_cert = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIEsTCCA5mgAwIBAgIQBOHnpNxc8vNtwCtCuF0VnzANBgkqhkiG9w0BAQsFADBs\n" \
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
  "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
  "ZSBFViBSb290IENBMB4XDTEzMTAyMjEyMDAwMFoXDTI4MTAyMjEyMDAwMFowcDEL\n" \
  "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
  "LmRpZ2ljZXJ0LmNvbTEvMC0GA1UEAxMmRGlnaUNlcnQgU0hBMiBIaWdoIEFzc3Vy\n" \
  "YW5jZSBTZXJ2ZXIgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC2\n" \
  "4C/CJAbIbQRf1+8KZAayfSImZRauQkCbztyfn3YHPsMwVYcZuU+UDlqUH1VWtMIC\n" \
  "Kq/QmO4LQNfE0DtyyBSe75CxEamu0si4QzrZCwvV1ZX1QK/IHe1NnF9Xt4ZQaJn1\n" \
  "itrSxwUfqJfJ3KSxgoQtxq2lnMcZgqaFD15EWCo3j/018QsIJzJa9buLnqS9UdAn\n" \
  "4t07QjOjBSjEuyjMmqwrIw14xnvmXnG3Sj4I+4G3FhahnSMSTeXXkgisdaScus0X\n" \
  "sh5ENWV/UyU50RwKmmMbGZJ0aAo3wsJSSMs5WqK24V3B3aAguCGikyZvFEohQcft\n" \
  "bZvySC/zA/WiaJJTL17jAgMBAAGjggFJMIIBRTASBgNVHRMBAf8ECDAGAQH/AgEA\n" \
  "MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIw\n" \
  "NAYIKwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2Vy\n" \
  "dC5jb20wSwYDVR0fBEQwQjBAoD6gPIY6aHR0cDovL2NybDQuZGlnaWNlcnQuY29t\n" \
  "L0RpZ2lDZXJ0SGlnaEFzc3VyYW5jZUVWUm9vdENBLmNybDA9BgNVHSAENjA0MDIG\n" \
  "BFUdIAAwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQ\n" \
  "UzAdBgNVHQ4EFgQUUWj/kK8CB3U8zNllZGKiErhZcjswHwYDVR0jBBgwFoAUsT7D\n" \
  "aQP4v0cB1JgmGggC72NkK8MwDQYJKoZIhvcNAQELBQADggEBABiKlYkD5m3fXPwd\n" \
  "aOpKj4PWUS+Na0QWnqxj9dJubISZi6qBcYRb7TROsLd5kinMLYBq8I4g4Xmk/gNH\n" \
  "E+r1hspZcX30BJZr01lYPf7TMSVcGDiEo+afgv2MW5gxTs14nhr9hctJqvIni5ly\n" \
  "/D6q1UEL2tU2ob8cbkdJf17ZSHwD2f2LSaCYJkJA69aSEaRkCldUxPUd1gJea6zu\n" \
  "xICaEnL6VpPX/78whQYwvwt/Tv9XBZ0k7YXDK/umdaisLRbvfXknsuvCnQsH6qqF\n" \
  "0wGjIChBWUMo0oHjqvbsezt3tkBigAVBRQHvFwY+3sAzm2fTYS5yh+Rp/BIAV0Ae\n" \
  "cPUeybQ=\n" \
  "-----END CERTIFICATE-----\n";
```

* Place this certificate include file in the directory of your sketch. In your code, you can now use something like below to do the actual upgrade.

```
ezProgressBar pb("OTA update in progress", "Downloading ...", "Abort");
String url = "https://raw.githubusercontent.com/M5ez/M5ez/master/compiled_binaries/M5ez-demo.bin";
#include "raw_githubusercontent_com.h"
if (ez.wifi.update(url, root_cert, &pb)) {
    ez.msgBox("Over The Air updater", "OTA download successful. Reboot to new firmware", "Reboot");
    ESP.restart();
} else {
    ez.msgBox("OTA error", ez.wifi.updateError(), "OK");
}
```

* And presto: you have Over The Air updates using https.

### Finer points

* The above example is actually from the `OTA_https` demo. Naturally you would replace the certificate and URL with your own.

* Over the Air updates only work if your partition table allows for two concurrent firmwares to be present. This means that if you do not plan to use OTA, you can have twice as much space for your programs. (Tools / Partition Scheme / No OTA in the Arduino IDE.)

* As you can see, the `ez.wifi.update` function takes three arguments: the first one is the https url for the firmware, the second one is the root certificate and the (optional) third one is a pointer to the ezProgressBar instance that will show progress for this download. (Don't forget the ampersand - `&` - in front.)

* Should `ez.wifi.update` return `false`: the human-readable error is provided when you call `ez.wifi.updateError()`.
 
* If your download is much more sneaky and silent, you can also just do something like:

```
#include "updates_com.h"
if (ez.wifi.update("https://updates.com/path/firmware.bin", root_cert)) {
  ESP.restart;
}
```

* For simplicity's sake, this ignores the obvious need to figure out whether there actually are any new updates that the user hasn't installed yet. This will just get the file, put it in flash and switch to it.

* If the certificate on the server is signed by a different root certificate - which may happen before the cert expiration date - updates will not work anymore. If you plan to deploy lots of IoT appliances using this update method it is probably best to control your own server and have a root certificate on it that expires sometime well after you will have sold your startup.

* `ez.wifi.update` cannot be used to pass basic auth credentials with the user:pass@host notation.

* Various other limitations of the WifiClientSecure class apply: this is IoT land...