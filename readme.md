# Zerynth Microchip Zero Touch Secure Provisioning Kit for AWS IoT

## Quick Setup

This section serves as a quick reference for the setup required.

### Hardware Setup

The central hub of the kit is the SAMG55 Xplained Pro board.

1. Plug **WINC1500 Xplained Pro** into **EXT1** on the SAMG55 Xplained Pro.
2. Plug **OLED1 Xplained Pro** into **EXT3** on the SAMG55 Xplained Pro.
3. Plug **CryptoAuth Xplained Pro** into **EXT4** on the SAMG55 Xplained Pro.
4. Plug USB cable from PC into **EDBG USB** port on the SAMG55 Xplained Pro.
   This port is how the firmware is loaded/updated and also exposes a serial
   port (COM port, 115200 baud).
   
### Firmware Setup

1. Update the **WINC1500 firmware to 19.5.4**. Use Atmel Studio to find create
   a new ASF Example Project for the **WINC1500 Firmware Update Project
   (v19.5.4) - SAMG55 Xplained Pro**. Run the
   **samg55_xplained_pro_firmware_update.bat** batch script from the src
   folder to update.
   
### Software Setup

1. Install [**Zerynth Studio**](https://www.zerynth.com/zerynth-studio/).
2. Install [**AWS CLI**](https://aws.amazon.com/cli/). Used to configure AWS
   credentials for the python scripts.
3. Install [**Python 3**](https://www.python.org). Make sure to include
   **pip** and **tcl/tk**. PC side work is done from the Zerynth Toolchain but also from a few
   custom Python scripts.
4. Install **Python packages (```pip install –r requirements.txt```)**
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

### Provision the ATECC508A on the kit

1. Register and virtualize the device.
2. Run ```ztc device discover --matchdb``` to retrieve the device id.
3. Run ```ztc device alias put RETRIEVEDID my_g55 xplained_samg55```  to assign the alias ```my_g55``` to the device.
4. Run ```ztc provisioning uplink-config-firmware my_g55 --i2caddr 0x0``` to prepare the device for provisioning (reset the device when asked to).
5. Run ```ztc provisioning crypto-scan my_g55``` to obtain the address of the crypto element.
6. Run ```ztc provisioning write-config my_g55 configuration.bin --lock True``` to write desired configuration to the device. **This command LOCKS the crypto element and sets the address to 0x58, this procedure is IRREVERSIBLE**
7. Manually reset the device and run again ```ztc provisioning crypto-scan my_g55``` to check if the new address has been assigned.
8. Run ```ztc provisioning gen-private my_g55 2``` to generate a private key inside slot 2 of the crypto element.
9. Run ```ztc provisioning get-csr my_g55 2 'C=IT,L=Pisa,O=Zerynth' -o device.csr``` to generate device CSR.

10. Run ```kit_provision.py --ssid wifi-name --password wifi-password``` to start provisioning the crypto element.
11. Run ```ztc provisioning store-public my_g55 13 root-ca.crt.public``` to store root-ca public key onto the crypto element.
12. Run ```ztc provisioning store-certificate my_g55 device device.crt``` to store device certificate.
13. Run ```ztc provisioning store-certificate my_g55 signer signer-ca.crt``` to store signer certificate.

### Uplink Zerynth Project

Open Zerynth Studio.
**From the Device Management Widget switch to Advanced Mode and then back to Auto mode to force alias refresh.**
Open ZeroTouchDemo project (firmware/ZeroTouchDemo).
Open the serial monitor to see, after uplink, if the device successfully connects to the WiFi and to the cloud.
Uplink the project.
   
### Interact with the Board via AWS

1. Run ```aws_interact_gui.py``` to interact with the board and toggle LEDs.
   Pressing the buttons on the board will also update their state in the GUI.
   
## Releases

### 2018-07-20
- Initial release
