import serial
import sys

if len(sys.argv) < 2 :
    print("Not enough arguments, please execute like this:")
    print("python3 mp_reset.py <serial_port>")
    exit
else:
    port = sys.argv[1]
    ser = serial.serial_for_url(port, 115200, rtscts = False, dsrdtr = False, do_not_open=True)
    ser.rts = 0
    ser.dtr = 0
    ser.open()
    ser.close()