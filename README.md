# linkplay-radio
Audio System based on Linkplay streaming module with ESP32 based control (AirPlay, Spotify Connect, TIDAL ...)

This Audio system is based on [Up2Stream](https://www.arylic.com/collections/diy-audio) Streaming modules based on [LinkPlay](https://linkplay.com/) technology. These modules support a lot of streaming services and protocols like:
- Airplay
- DLNA
- Spotify Connect
- Qplay
- UPnP
- Deezer
- Tidal
- Qobuz
- iHeartRadio
- Napster
- TuneIn

There is also an app for Android and IOS available. With other devices based on Linkplay you can create also a multi room system for your house. Overall it is something like Sonos but for less money. Up2Stream is also offering some boards and devices with its own power amplifier. Since I wanted to have a desktop solution which covers all my audio needs there were some shortcomings which this project should overcome.
- The streaming modules do require the use of an app to play anything. Sometimes I just want to play an internet radio station without using my phone. 
- There are no physical controls like a volume knob possible

On GitHub I found out that the LinkPlay modules do have an [API](https://github.com/AndersFluur/IEastMediaRoom) which does allow to control the functionality over the network. I am using an [ESP32 module](https://docs.zerynth.com/latest/official/board.zerynth.doit_esp32/docs/index.html) to talk to that API. Since the LinkPlay module can act as an access point the ESP32 can connect to the module directly using that access point. 

In addition the ESP32 module is connected to a rotary encoder for volume control and to the front panel buttons of an old Sony stereo Tuner. This allows things like next song, paly/pause etc . A 16x2 I2C character LCD is used as a display.

I my case I also added additional audio sources using a custom circuit board.

- USB audio device for my PC
- SPDIF input for a CD player or TV
- Linie In 
- Bluetooth module

The build in DAC of the LinkPlay module is OK but not great. Since the LinkPlay module does have an I2S interface it is possible to do the DA convesion in a separate DC which in my case is a AD1865 based R2R DAC. But you can also use any other DAC module using an I2S input. The other digital audio sources ( Bluetooth, SPDIF and USB) do also produce an I2S signal. The switching is done on the custom circuit board and is controlled by the ES32. For volume control I am using a high performance [volume control module](http://store.sure-electronics.com/product/AA-AB41148) using the [PGA2311](http://www.ti.com/lit/ds/symlink/pga2311.pdf) chip ftom TI.

As a power amplifier I am using a 2x125W [ICEPOWER 125ASX2](https://icepower.dk/products/amplifier-power-modules/asx-series/) module.

Over all that project does cover a lot of different audio sources and streaming services and does provide a very high quality sound for almost all speakers. 
