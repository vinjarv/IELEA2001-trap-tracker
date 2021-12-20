# Communication protocol

## Introduction

The communication between the submerged module and the transmitter is done over UTP CAT5 cable for testing. The transmitter acts as a client and always initiates communication. 

## Requesting sensor data

Client sends:
`readsensors\n`
Server responds with JSON-formatted data, i.e.:
`"lightlevel":"12.34", "pressure":"1234.0", "temperature":"1.23\n`


## Requesting image capture
Client sends:
`getimage\n`
Server responds with JSON-formatted data:
`"image":"<raw image data>"\n`
TODO:
Checksum ? 
Image format ?

