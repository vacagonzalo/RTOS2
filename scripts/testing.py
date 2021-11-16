import serial
import threading
import time

Port = None
WaitTime = 0.1


def TransmitThread():
    while Port:   
        for n in range(ord("A"),ord("Z")+1):
            if Port:
                Port.write(chr(n).encode())
                time.sleep(WaitTime)

def ReceiveThread():
    while Port:
        if Port.inWaiting() > 0:
            c = Port.read(1).decode('ascii')
            print( c )
        else:
            time.sleep(WaitTime)

def LoopbackTest(PortName):
    global Port

    Port = serial.Serial(port=PortName,
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )

    threading.Thread(target=TransmitThread).start()
    threading.Thread(target=ReceiveThread).start()

    try:
        while True:
            time.sleep(WaitTime)
    except:
        Port = None

if __name__ == "__main__":
    LoopbackTest("/dev/ttyUSB1")