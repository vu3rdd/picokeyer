# Iambic keyer with Raspberry Pico

There are many Iambic keyer implementations on Arduino, but not many
with Raspberry Pico. This is a (incomplete) port of Steven Elliot's
(K1EL) keyer (which first appeared in [openqrp.org
website](https://web.archive.org/web/20110602165043/http://openqrp.org/?p=343)).

Iambic Mode B is not implemented yet (mainly because I don't use
it). Patches welcome.

# Implementation notes

The toughest part for me was maintaining the timing with
time_us_64(). Where we capture the free running timer matters. I
haven't measured the resulting waveforms, but I think the output
mostly matches the input WPM that is asked.

For now, changing WPM/Sidetone frequency/pins would need editing
`main.cpp` and recompiling the code.

Output sidetone goes into the PWM output. A low value resistor and an
electrolytic capacitor in series is recommended before connecting the
negative pin of the capacitor to the speaker. i.e.:

```
 GPIO pin |- ...\/\/\/\/.....)|---- 🎧
```
# License

The original code was licenses GPLv2.1 or above. Since we derive from
that code, this code too has the same license. Please share your
improvements and fixes.


