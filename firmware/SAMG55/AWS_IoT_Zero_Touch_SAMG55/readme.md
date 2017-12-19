# Zero Touch Provisioning kit for AWS IoT Release B Firmware

This is the firmware for the Zero Touch Provisioning Kit for AWS IoT Release B
(AT88CKECC-AWS-XSTK-B). More information and the latest firmware can be found
at the following web page:

http://www.microchip.com/developmenttools/productdetails.aspx?partno=at88ckecc-aws-xstk-b
https://github.com/MicrochipTech/aws-iot-zero-touch-secure-provisioning-kit

## Change Log

### 2.2.4
 - Added unconfigured device auto-detection and pre-configuration. Also brought in support for the ATECC608A.

### 2.2.3
- Fixed memory leak when parsing json from shadow update delta topic

### 2.2.2 (2017-9-26)
- Resolved DNS lookup happening too soon

### 2.2.1 (2017-9-18)
- Now reports LED status to AWS shadow
- Added support for open WiFi access points

### 2.2.0 (2017-7-10)
- Initial release of firmware for AT88CKECC-AWS-XSTK-B
