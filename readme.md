# Microchip Zero Touch Secure Provisioning Kit for AWS IoT

This package contains all the files to run the Microchip Zero Touch Secure
Provisioning Kit for AWS IoT.

Latest product information can be found at
http://www.microchip.com/developmenttools/productdetails.aspx?partno=at88ckecc-aws-xstk-b

The full user guide can be found at http://microchipdeveloper.com/iot:ztpk

## Quick Setup

This section serves as a quick reference for the setup required. The full user
manual referenced above will give more detailed instructions.

### Hardware Setup

The central hub of the kit is the SAMG55 Xplained Pro board.

1. Plug **WINC1500 Xplained Pro** into **EXT1** on the SAMG55 Xplained Pro.
2. Plug **OLED1 Xplained Pro** into **EXT3** on the SAMG55 Xplained Pro.
3. Plug **CryptoAuth Xplained Pro** into **EXT4** on the SAMG55 Xplained Pro.
   Please note, depending on when you purchased your kit, your kit may have come
   with CryptoAuth Xplained Pro **Rev A** boards or **Rev B** boards.  Rev B
   boards have an ATECC608A device attached and do not come pre-configured.  Extra
   steps need to be followed to initialize the crypto-device on the board.  Begin
   the initialization process by **running the firmware without the WINC1500 Xplained
   Pro board attached**.  The firmware will automatically guide you through this
   process with instructions from EDBG serial port output messages.4. Plug USB cable from PC into **Target USB** port on the SAMG55 Xplained Pro.
   Once the firmware is loaded, the board communicates with the scripts on the
   PC via this port as an HID device.
5. Plug USB cable from PC into **EDBG USB** port on the SAMG55 Xplained Pro.
   This port is how the firmware is loaded/updated and also exposes a serial
   port (COM port) that outputs debug/status information (115200 baud).
   
### Firmware Setup

While the revision B kit comes with the appropriate firmware loaded, the
original (rev A) kit will need firmware updates to work.  Additionally, new
firmware updates may be released.

1. Update the **WINC1500 firmware to 19.5.2**. Use Atmel Studio to find create
   a new ASF Example Project for the **WINC1500 Firmware Update Project
   (v19.5.2) - SAMG55 Xplained Pro**. Run the
   **samg55_xplained_pro_firmware_update.bat** batch script from the src
   folder to update.
2. Update the **SAMG55 firmware** to the latest version in the firmware
   folder of this package.
   
### Software Setup

1. Install [**AWS CLI**](https://aws.amazon.com/cli/). Used to configure AWS
   credentials for the python scripts.
2. Install serial terminal emulator, like [**PuTTY**](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html).
   Used to view status/debug information from the SAMG55.
3. Install [**Visual C++ 2015 Build Tools**](http://landinghub.visualstudio.com/visual-cpp-build-tools).
   This is required for one of the python packages (hidapi) installed later.
4. Install [**Python 3**](https://www.python.org). Make sure to include
   **pip** and **tcl/tk**. PC side work is all done from python scripts.
5. Install **Python packages (```pip install –r requirements.txt```)**
   required for the kit python scripts.
   
### AWS Setup

#### Manual Setup

1. Log into the AWS Console for your account and select the region you want to
   run the kit from.

2. Create an IAM user to demo/run the kit from:
    - User name: **ZTUser**
    - Enable **Programmatic access** and **AWS Management Console access**
    - Add AWS managed policies **AWSIoTFullAccess** and
      **AWSLambdaFullAccess**
    - Run ```aws configure``` from the command line on your PC to configure
      the AWS credentials for the ZTUser account. Make sure the enter the same
      region as selected in the previous step.
       
3. Create JITR Lambda Function Policy
    - Name: **ZTLambdaJITRPolicy**
    - Policy Document:
        ```json
        {
            "Version": "2012-10-17",
            "Statement": [
                {
                    "Effect": "Allow",
                    "Action": [
                        "iot:UpdateCertificate",
                        "iot:CreatePolicy",
                        "iot:AttachPrincipalPolicy",
                        "iot:CreateThing",
                        "iot:CreateThingType",
                        "iot:DescribeCertificate",
                        "iot:DescribeCaCertificate",
                        "iot:DescribeThing",
                        "iot:DescribeThingType",
                        "iot:GetPolicy"
                    ],
                    "Resource": "*"
                }
            ]
        }
        ```
        
4. Create JITR Lambda Function Role
     - Role type: **AWS Service Role > AWS Lambda**
     - Name: **ZTLambdaJITRRole**
     - Add policies **AWSLambdaBasicExecutionRole**,
       **AWSXrayWriteOnlyAccess**, and **ZTLambdaJITRPolicy**.
       
#### Automated Setup

See AWS CloudFormation templates and documentation in the
cloud-formation-templates folder.

## Quick Demo

This section serves as a quick reference for the steps required. The full user
manual referenced above will give more detailed instructions.

### Configure AWS for Just In Time Registration (JITR)

These steps will be performed from the IAM user, ZTUser, created for
demonstrating this kit.

1. Create the **Just In Time Registration (JITR) Lambda Function**
    - Name: **ZTLambdaJITR**
    - Runtime: **Python 3.6**
    - Copy and paste the code found in ```ZTLambdaJITR/lambda_function.py```
      into the code entry area.
	- Existing Role: **ZTLambdaJITRRole**
      
2. Create **IoT Rules Engine Rule** for triggering the JITR lambda function.
    - Name: **ZeroTouchJustInTimeRegistration**
    - SQL version: **2016-03-23**
    - Attribute: **\***
    - Topic filter: **$aws/events/certificates/registered/#**
    - Condition:
    - **Add action** to invoke the ZTLambdaJITR lambda function.
    
### Create and Register the Certificate Authorities (CAs)

1. Run ```ca_create_root.py``` to create a root CA.
2. Run ```ca_create_signer_csr.py``` to create a CSR for a signer.
3. Run ```ca_create_signer.py``` to sign the signer CSR with the root CA.
4. Run ```aws_register_signer.py``` to register the signer with AWS IoT.

### Provision the ATECC508A on the kit.

1. Run ```kit_set_wifi.py --ssid wifi-name --password wifi-password``` to
   configure wifi settings on the board. This network must have internet
   access with ports 123 (UDP, time server) and 8883 (TCP, secure MQTT) open.

2. Run ```kit_provision.py``` to provision the ATECC508A on the board for AWS
   IoT. After this command, the board will automatically attempt to connect to
   AWS IoT.
   
### Interact with the Board via AWS

1. Run ```aws_interact_gui.py``` to interact with the board and toggle LEDs.
   Pressing the buttons on the board will also update their state in the GUI.
   
## Releases

### 2018-03-19
- Updated aws_register_signer.py to account for new datetime fields from AWS

### 2017-12-19
- Updated CA scripts to use fixed set of extensions for CSR and certificate
- Set fixed version of pyasn1_modules as new version broke cert2certdef.py

### 2017-11-17
- Updated firmware to v2.2.4 to bring in ATECC608A support with CryptoAuthLib
  release 3.
- Firmware now supports automatic pre-configuration of new ATECC508A and
  ATECC608A devices.
- Fixed a memory leak in the JSON parsing.

### 2017-9-26
- Updated firmware to v2.2.2 to resolve DNS lookup issue

### 2017-9-18
- Initial release of software and firmware v2.2.1