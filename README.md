# Arduino Music Lights

This is a small project using an Arduino to synchronize a LED stripe to music in realtime. It circles through 12 different colors and 8 different modes. Every mode reacts somehow to the music and they are cycled through after a random amount of time when it fits the music (peaks, drops etc.).

## Functionality

A microphone is connected to the Arduino to record the surrounding sound. It's volume is then used to sync the LEDs to the sound. To do so most modes compare the current volume to the average volume of the last ~0.5 seconds. Whenever the current volume is significant louder then the recent average it reacts somehow to it depending on the current mode.

Additionally the peaks are also used to change color or change mode (after a randomly calculated minimum time). This way the lights feel even more responsive to the music.

## Parts

* Arduino Nano
* GY-MAX4466 with amplifier for adjustable gain
* WS2812b LED stripe
* 5V Power supply
* Some wires

>**Warning:** Be careful and do not connect more then 5V power supply or the WS2812b will break!

## Setup

![Image of the final circuit](./circuit.png)
*(Sketch created with [Fritzing](https://fritzing.org/))*

Connect everything as outlined in the sketch, upload the code and it should work.
>**Note:** For some reason (*that I do not completly understand*) is it necessary to connect the microphone's 5V VCC to the Arduino's 5V output (*as done in the sketch*) and not directly to the 5V power supply. Otherwise the LED stripe might have some weird behaviour.

## Conclusion

Let me know if you have some troubles, questions, feedback or improvements on my code!