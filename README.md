# matrix-temperature-logger
A simple temperature logger using Matrix as a storage mechanism. This requires a valid Matrix ID and a WiFi connection in order to operate.

The ESP8266 used in this project sends events of the type `io.t2l.matrix.weather`. A weather event looks similar to this:
```
{
  "type": "io.t2l.matrix.weather",
  "content": {
    "temp_c": 22.43,
    "temp_f": 72.37,
    "rel_humidity": 22.80
  }
}
```

# Components

* [SI7021 Breakout](https://www.sparkfun.com/products/13763) from Sparkfun
* An ESP8266 ([SparkFun ESP8266 Thing - Dev](https://www.sparkfun.com/products/13711) used in testing)

# Setup

1. Wire the SI7021 breakout board to the ESP8266.

   If you're using the ESP8266 Thing Dev from Sparkfun:

   |SI7021|ESP8226 Thing Dev|
   |------|-----------------|
   |+     |3.3V             |
   |-     |GND              |
   |DA    |2                |
   |CL    |14               |

2. Change the constants in the code before uploading it to the ESP8266
   Ie: Your wifi settings and Matrix information

3. Upload the code and watch it spam weather data about once per second
   *actually you won't see anything without a client to read the custom events. Use `!weather` to get the current weather.*

# References

* [Matt Williams' matrix-esp8266](https://github.com/matt-williams/matrix-esp8266) - Served as a base for how to interact with Matrix on the ESP8266