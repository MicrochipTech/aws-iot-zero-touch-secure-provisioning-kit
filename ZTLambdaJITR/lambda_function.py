import os
import base64
import binascii
import json
import boto3
import botocore

iot = boto3.client('iot')

ZT_THING_TYPE_NAME = 'microchip-zero-touch-kit'

def lambda_handler(event, context):
    
    # Get evironment and event data
    region = os.environ['AWS_DEFAULT_REGION']
    account_id = event['awsAccountId']
    certificate_id = event['certificateId']
    
    # Get device certificate information
    response = iot.describe_certificate(certificateId=certificate_id)
    certificate_arn = response['certificateDescription']['certificateArn']
    
    # Convert the device certificate from PEM to DER format
    pem_lines = response['certificateDescription']['certificatePem'].split('\n') # split PEM into lines
    pem_lines = list(filter(None, pem_lines))      # Remove empty lines
    raw_pem = ''.join(pem_lines[1:-1])             # Remove PEM header and footer and join base64 data
    cert_der = base64.standard_b64decode(raw_pem)  # Decode base64 (PEM) data into DER certificate
    
    # Find the subjectKeyIdentifier (quicker than a full ASN.1 X.509 parser)
    subj_key_id_prefix = b'\x30\x1D\x06\x03\x55\x1D\x0E\x04\x16\x04\x14'
    subj_key_id_index = cert_der.index(subj_key_id_prefix) + len(subj_key_id_prefix)
    subj_key_id = binascii.b2a_hex(cert_der[subj_key_id_index:subj_key_id_index+20]).decode('ascii')
    print('Certificate Subject Key ID: {}'.format(subj_key_id))
    
    # Thing name and MQTT client ID will be the subject key ID
    thing_name = subj_key_id
    client_id = subj_key_id
    
    # Create thing type (no error if it already exists)
    response = iot.create_thing_type(thingTypeName=ZT_THING_TYPE_NAME)
    
    # Create a thing (no error if it already exists)
    response = iot.create_thing(
        thingName=thing_name,
        thingTypeName=ZT_THING_TYPE_NAME)
    
    # Create a policy for the certificate to limit it only to its own topics and shadow
    policy_name = 'ZTPolicy_{}'.format(certificate_id)
    policy_document = {
        'Version': '2012-10-17',
        'Statement': [
            {
                'Effect': 'Allow',
                'Action': [
                    'iot:Connect'
                ],
                'Resource': [
                    # Can only connect to the MQTT broker using its subject key id as its client ID
                    'arn:aws:iot:{}:{}:client/{}'.format(region, account_id, client_id)
                ]
            },
            {
                'Effect': 'Allow',
                'Action': [
                    'iot:Publish',
                    'iot:Receive'
                ],
                'Resource': [
                    'arn:aws:iot:{}:{}:topic/${{iot:ClientId}}/*'.format(region, account_id),
                    'arn:aws:iot:{}:{}:topic/$aws/things/${{iot:ClientId}}/shadow/*'.format(region, account_id)
                ]
            },
            {
                'Effect': 'Allow',
                'Action': [
                    'iot:Subscribe'
                ],
                'Resource': [
                    'arn:aws:iot:{}:{}:topicfilter/${{iot:ClientId}}/#'.format(region, account_id),
                    'arn:aws:iot:{}:{}:topicfilter/$aws/things/${{iot:ClientId}}/shadow/*'.format(region, account_id)
                ]
            },
            {
                'Effect': 'Allow',
                'Action': [
                    'iot:UpdateThingShadow',
                    'iot:GetThingShadow'
                ],
                'Resource': [
                    'arn:aws:iot:{}:{}:topic/$aws/things/${{iot:ClientId}}/shadow/*'.format(region, account_id)
                ]
            }
        ]
    }
    try:
        response = iot.create_policy(
            policyName=policy_name,
            policyDocument=json.dumps(policy_document, indent=4)) # policy document string formatted as json data
        print('Created policy {}'.format(policy_name))
    except botocore.exceptions.ClientError as e:
        if e.response['Error']['Code'] == 'ResourceAlreadyExistsException':
            print('Policy {} already exists'.format(policy_name))
        else:
            raise
    
    # Attach policy to device certificate. Certificates must have a policy
    # before they can be activated.
    iot.attach_principal_policy(
        policyName=policy_name,
        principal=certificate_arn)
    
    # Activate the certificate to allow connections from that device
    response = iot.update_certificate(
        certificateId=certificate_id,
        newStatus='ACTIVE')
    