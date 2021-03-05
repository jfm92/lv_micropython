import gc
import uos
import machine
from flashbdev import bdev

try:
    #if bdev:
    #    uos.mount(bdev, "/")
    sd = machine.SDCard(slot=2, width=1, cd=None, wp=None, sck=None, miso=None, mosi=None, cs=5, freq=20000000)
    uos.mount(sd,"/")
except OSError:
    import inisetup

    vfs = inisetup.setup()

gc.collect()
