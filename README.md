# microPython for microByte


This repository is a fork of the microPython + LVGL repository. You can find the next improvements on this repo:

- Integrated microByte port as a C module. This integration provides a native screen, storage, and controls usage, using directly the microPython calls. **Not necessary to add new modules.**
- Integration of Stage library to create simple games with microPython. This library was developed by [deshipu](https://github.com/python-ugame/micropython-stage)
- Integration of LVGL graphic user interface library with the microByte's port, avoiding configure and initialize the screen.

In the next list of contents you can see some tutorials and important usage information:

- Important notice.
- Build & flash the binary.
    - Prepare the workspace
    - Build the firmware.
    - Flash it!
- Tutorials.
    - Previous considerations.
    - Getting started
    - LVGL
    - Stage Game Library



## Important notice

This library was one of the main points of the microByte project, but unfortunately, some features were not possible to implement, and maybe the usability is not so good as it was intended at the beginning. 
The reason for that is a hardware limitation, the microByte has a CH340G chip to handle the USB communications, this chip requires a connection with DC and RESET pin of the ESP32. So, when you connect the device to the PC and open the UART port, the PC sends a signal over USB which activates the DC and RESET signal giving as result a reset of the microcontroller. This is something good if you want to flash the microcontroller, but if you just want to "interact" with UART, is not very good.

So, we have the next limitations:
- No boot menu to select if you want to execute an application or open REPL.
- It was necessary a workaround use the file system. Now it's working but you can't see for example on MU or another microPython interpreter.
- It's necessary to run a script to use REPL or manually open the port (I.E. With Arduino IDE open the serial console).

## Build the binary

You can build the binary file on any O.S. but, I highly recommend to use **Ubuntu 18.04** or Windows with WSL and Ubuntu 18.04, install on it. Anyways this guide will be focus Ubuntu 18.04, I think that Ubuntu 20.04 should also work.

[Windows WSL installation tutorial](https://docs.microsoft.com/en-us/windows/wsl/install-win10).

### Prepare the workspace.

- Install the depencies:
```
sudo apt update
```

```
sudo apt-get install build-essential libreadline-dev libffi-dev git pkg-config libsdl2-2.0-0 libsdl2-dev python3.8 python-pip -y
```

- Download the repository:
```
git clone --recurse-submodules https://github.com/jfm92/mb_micropython.git
```

- Compile mpy-cross tool:
```
cd mb_micropython
make -C mpy-cross
```

- Set-up ESP-IDF toolchain.
```
cd ports/microByte
export ESPIDF=$HOME/src/github.com/espressif/esp-idf
export MB_PATH="$(pwd)"
mkdir -p $ESPIDF
cd $ESPIDF
git clone https://github.com/espressif/esp-idf.git $ESPIDF
git checkout 9e70825d1e1cbf7988cf36981774300066580ea7
git submodule update --init --recursive
```

- Install Python dependencies.
```
cd $MB_PATH
pip install 'pyserial>=3.0'
pip install 'pyparsing>=2.0.3,<2.4.0'
sudo apt-get install python3-venv -y
python3 -m venv build-venv
source build-venv/bin/activate
pip install --upgrade pip
pip install -r $ESPIDF/requirements.txt
```

- IDF-Tool workaround.

Sometimes there is an issue when you try to install the ESP-IDF on virtual enviorement with the "--user" flag, so you need to do a little modifcation.

    - Go to `$ESPIDF/tools`
    - Open `idf_tools.py`
    - Go to the line 1157 and delete '--user'

It should look like this:
```
subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'virtualenv'],
                                  stdout=sys.stdout, stderr=sys.stderr)
```

- Install ESP-IDF.

```
cd $ESPIDF
./install.sh

```

- Source the export script.

```
cd $MB_PATH
source $ESPIDF/export.sh 
```

### Build the firmware

Now it's everything ready to work with microPython, no execute the next commands:
```
cd $MB_PATH/../..
 make -C ports/microByte BOARD=GENERIC_SPIRAM
```

If everything goes fine it should return something like this:
```bash
LINK build-GENERIC_SPIRAM/application.elf
   text    data     bss     dec     hex filename
1624104  605480   41300 2270884  22a6a4 build-GENERIC_SPIRAM/application.elf
Create build-GENERIC_SPIRAM/application.bin
esptool.py v2.8
Create build-GENERIC_SPIRAM/firmware.bin
bootloader     23024
partitions      3072
application  2229712
total        2295248
```

### Flash it!

Connect your microByte to the PC and check which serial port was assigned. You can modify the port doing the next:

- Open `ports/microByte/Makefile`
- On the line 45, you will find `PORT ?= /dev/ttyS3`
- Change the serial port to your serial port.

Once you've set your serail port, on the root folder, execute the next command:
```
 make -C ports/microByte BOARD=GENERIC_SPIRAM deploy
```

This should return something like this:
```
Writing build-GENERIC_SPIRAM/firmware.bin to the board
esptool.py v2.8
Serial port /dev/ttyS3
Connecting....
Chip is ESP32D0WDQ5 (revision 3)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
Crystal is 40MHz
MAC: c4:dd:57:92:cb:94
Uploading stub...
Running stub...
Stub running...
Changing baud rate to 2000000
Changed.
Configuring flash size...
Auto-detected Flash size: 16MB
Compressed 2291152 bytes to 1366321...
Wrote 2291152 bytes (1366321 compressed) at 0x00001000 in 22.4 seconds (effective 817.1 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
```

That's all folks!

## Tutorials

### Previous considerations

You can use any microPython "IDE" or even just use a serial connection with a simple terminal like puTTY, but in most case you'll find this issue:

```
rst:0x1 (POWERON_RESET),boot:0x3 (DOWNLOAD_BOOT(UART0/UART1/SDIO_REI_REO_V2))
waiting for download
```

The device is on boot mode and you can't use the UART peripheral. In this case, you just need to execute the next:

- Find the serial number of the microByte.
- Go to the mb_micropython folder.
- Execute the next command: `python3 mb_reset.py <microByte_serial_port>`

### Getting started

As a first step, I recommend getting familiar with microPython. This first point will be a very basic one, you can find tons of information on the Internet.

- Download an editor, I recommend [MU](https://codewith.mu/en/). It's minimalistic but very useful.

- Connect the device and push `REPL` button. (I recommend checking the previous point because maybe that issue will raise)

- This should open a console and return something like this:

```
MicroPython v1.14-458-g44c31c66f-dirty on 2021-07-18; ESP32 module (spiram) with ESP32
Type "help()" for more information.
>>> 
```
- You can write commands on the console or write a script on the edit screen on the top and send it by pushing the execute button.

- To finalize lets create a simple script which clean the screen

```python
import machine

display = machine.display()
display.clear()

```

After this the display should be white.

You can find a good quantity of microPython tutorials and documentations [here](https://docs.micropython.org/en/latest/esp32/quickref.html#)

### Create games with Stage library

This is tutorial is to execute a demo, the code is commented on and gives more details.

- First, we need to copy the file [demo.bmp]() to the root folder of the microSD card.

- Then on your favorite editor execute the next code:

```python
import machine
import stage

#Initialize screen
display = machine.display()
#Initialize the buttons
btn_right = machine.gamepad(3)
btn_left = machine.gamepad(1)
btn_a = machine.gamepad(9)

#Load sprites file
bank = stage.Bank.from_bmp16("ball3.bmp")

#Create the background
background = stage.Grid(bank,15,15)
#Paint the sky
y=0
while y<13:
    x = 0
    while x<15:
        background.tile(x,y,6)
        x += 1
    y += 1

#Paint some clouds
background.tile(2,4,5)
background.tile(5,6,5)
background.tile(8,5,5)
background.tile(11,7,5)

#Create ball sprite
ball = stage.Sprite(bank, 1, 100, 192)

#Initialize game engine with a limit of 60FPS
game = stage.Stage(display, 60)

#Set the layers and render all the screen
game.layers = [ball] + [background] 
game.render_block()

dx = 5 # Increment on x axis of the ball
dy = -10 #Increment on y axis of the ball
while True:
    if btn_right.pressed(): #If right button was pressed move to right
        if ( 0 < ball.x < 220):
            dx = 5
            ball.set_frame(ball.frame % 4 + 1)
            ball.move(ball.x + dx, ball.y)
    if btn_left.pressed(): #If left button was pressed move to left
        if( 0 < ball.x < 220):
            dx = -5
            ball.set_frame(ball.frame % 4 + 1)
            ball.move(ball.x + dx, ball.y)
    if btn_a.pressed(): #If button a was pressed, the ball jump creating an animation
        while ball.y > 130:
            ball.set_frame(ball.frame % 4 + 1)
            ball.move(ball.x, ball.y+dy)
            game.render_block()
        while ball.y < 188:
            dy += 1
            ball.set_frame(ball.frame % 4 + 1)
            ball.move(ball.x, ball.y + dy)
            game.render_block()
        dy = -10

    ball.update()
    game.render_block()
    game.tick()
```

This will execute a simple demo where you can move a ball over the screen. On that demo, you can see how to load a background, sprites, and render it.

To create your sprites, it's very important to use GIMP and save them as a BMP file with 16 depth colors. I wasn't able to make it work with other programs.
Another important point to know is that only 8bit colors will work.

You can find more information on the official wiki of the Stage project, which is not very deep but has some interesting info:
[Stage wiki](https://circuitpython-stage.readthedocs.io/en/latest/README.html)


### LVGL

It's necessary to rethink a little bit how it's implement. We can considerer still.. a work in progress. 