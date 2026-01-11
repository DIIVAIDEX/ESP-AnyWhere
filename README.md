# ESP-AnyWhere
## Overview
This project provides a low cost solution to set up remote monitoring and dispatching of a field site using an ESP32 microcontroller paired with the 4G modem. The goal is to enable users to quickly deploy a secure, web-accessible system for tracking and controlling remote assets with minimal investment.

## Key Features
**Affordability**: Leverages low-cost hardware (ESP32 + A7670) and open-source software.

**Simplicity**: Built on ESP-IDF examples with minimal code modifications — ideal for developers familiar with ESP-IDF.

**Security**: Uses WireGuard VPN for encrypted communication over 4G networks.

**Web Accessibility**: Exposes the device’s local web interface to the public internet via an NGINX forward proxy.
	
**Scalability**: Designed to be replicated for multiple remote sites with minimal effort.

## Architecture
### The system operates as follows:
1) ESP32-S3 + Simcom A7670 connects to the 4G cellular network.
2) WireGuard VPN tunnel is established between the ESP32 and a cloud NGINX server. This allows forward the WEB interface to the Internet and secure it.
3) NGINX Forward Proxy routes incoming HTTP**S** requests from the internet to the ESP32’s local web server with  basic HTTP authentication.
4) End-users access the ESP32 web interface via a public URL managed by the NGINX proxy.

## Requirements
Waveshare [ESP32-S3-A7670E-4G](https://www.waveshare.com/wiki/ESP32-S3-A7670E-4G) board

![ESP32-S3-A7670E-4G](https://github.com/user-attachments/assets/c1192f4c-6738-4e2b-b229-f654498a13b3)

NGINX server as forward proxy (can be hosted on cloud, VPS, or local machine) with Wireguard VPN

Basic familiarity with ESP-IDF framework

##
![3](https://github.com/user-attachments/assets/fca9bbdb-facb-4ca0-acf2-72cd6845a252)
