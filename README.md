# Rintento

## Introduction

The service is intended for running an action upon client intent. Recognizing intent is made by configured backend.
Upon client request service initiates recognizing and running previously configured actions.

![](asset/misc/overview.png)

## Purpose

The main purpose of the service is running an action upon client requests.
Where each client request represented by:
* text message (e.g. "turn on the light")
* human speech in particular audio format

After recognizing intent the service perform matching obtained intents and previously configured action.
After matching procedure the service runs particular actions.

## Setup

The service requires additional setup (see [setup.md](doc/setup.md)).

## API

The service provides following public API (see [api.md](docs/api.md)).

## Audio

The service requires humar speech in specific format (see [audio.md](doc/audio.md)).
