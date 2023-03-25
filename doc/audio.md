
# Input audio format

Input audio characteristics:
* encoding=signed-integer;
* bits=16;
* rate=16000;
* endian=little.

# How-To

## How-To: Recording raw audio from input device

Find input device name:
```shell
$ pactl list | grep -A3 'Source #'
Source #0
    State: SUSPENDED
    Name: alsa_output.pci-0000_01_00.1.hdmi-stereo-extra1.monitor
    Description: Monitor of GM204 High Definition Audio Controller Digital Stereo (HDMI 2)
--
Source #1
    State: SUSPENDED
    Name: alsa_input.usb-046d_1080P_Pro_Stream_A9EDF8DF-02.analog-stereo
    Description: CrystalCam Analog Stereo
--
Source #2
    State: SUSPENDED
    Name: alsa_output.pci-0000_00_1f.3.iec958-stereo.monitor
    Description: Monitor of Built-in Audio Digital Stereo (IEC958)
--
Source #3
    State: SUSPENDED
    Name: alsa_input.pci-0000_00_1f.3.analog-stereo
    Description: Built-in Audio Analog Stereo
```

Writing RAW audio file:
```shell
$ gst-launch-1.0 -e pulsesrc device=<device-name-from-pactl-output> \
! queue \
! audioresample ! audioconvert \
! audio/x-raw,format=S16LE,rate=16000,channels=1,quality=10 \
! filesink location=audio.raw
```
    
Reading RAW audio file:
```shell
$ gst-launch-1.0 -e filesrc location = "audio.raw" \
! rawaudioparse use-sink-caps=false format=pcm pcm-format=s16le sample-rate=16000 num-channels=1 \
! audioconvert \
! audioresample \
! autoaudiosink
```