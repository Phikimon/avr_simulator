### Graphical avr simulator

Unfinished and as for now very unstable graphical parallel simulator
of two AVR ATtiny13A microcontrollers.  Powered by gtk+ 3.

#### Features
1. Step-by-step execution
2. Registers's values and pins states are dumped to the screen
at every cycle
3. Interrupts(unstable)
4. Full instruction set
5. Visualization of waveforms via `gtkwave`(will be pushed in a few days)
6. Currently executed line is highlighted in disassembly
7. Programs for different microcontrollers are simulated in parallel

#### Installation of dependencies
```sh
$ sudo apt install libgtk-3-dev ctags gtkwave
```

#### Execution
```sh
$ ./main
```

![Screenshot](https://github.com/phikimon/avr_simulator/raw/master/resources/interface_screenshot.png)
