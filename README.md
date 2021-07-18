# microPython for microByte


This repository is a fork of microPython + LVGL repository. You can find the next improvments on this repo:

- Integrated microByte port as a C module. This integration provides native screen, storage and controls ussage, using directly the microPython calls. **Not necessary to add new modules.**
- Integration of Stage library to create simple games with microPython. This library was developed by [deshipu](https://github.com/python-ugame/micropython-stage)
- Integration of LVGL graphic user interface library with the microByte's port, avoiding to configure and initalize the screen.

In the next list of contents you can see some tutorials and important ussage information:

- Important notice.
- Build & flash the binary.
    - Prepare the workspace
    - Build the firmware.
    - Flash it!
- Tutorials.
    - Previous considerations.
    - Basic ussage.
    - LVGL
    - Stage Game Library



## Important notice

This library was one of the main points of the microByte project, but unfortnately, some features was not possible to implement and maybe the usuability is not so good as it was intendend on the beginning. 
The reason of that is basically hardware limitation, the microByte has a CH340G chip to handle the USB communications, this chip requires a connection with DC and RESET pin of the ESP32. So, when you connect the device to the PC and open the UART port, the PC sends a signal over USB which activate the DC and RESET signal giving as result a reset of the microcontroller. This is something good if you want to flash the microcontroller, but if you just want "interact" with UART, is not very good.

So, we have the next limitations:
- No boot menu to select if you want to execute an application or open REPL.
- It was necessary a workaround to use the file system. Now it's working but you can't see for example on MU or other microPython interpreter.
- It's necessary to run an script to use REPL or manually open the port (I.E. With Arduino IDE open the serial console).

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

# WIP