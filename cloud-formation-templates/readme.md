# AWS Zero-touch Kit CloudFormation Templates

These [CloudFormation](https://aws.amazon.com/cloudformation/) templates
provide an automated way to create the AWS resources required to demonstrate
the kit.

While the user manual gives instructions for creating these resources manually
using the AWS console, these automate the process for when you want to get it
running quicker.

These templates are typically deployed from an AWS account with administrative
privileges.


## aws-zero-touch-admin-setup.yaml

This script sets up the administrative resources (kit IAM user and lambda
role), but leaves the actual lambda function and IoT rules engine rule up to
the user.


## aws-zero-touch-full-setup.yaml

This script sets up all the resources required to demonstrate the kit.
