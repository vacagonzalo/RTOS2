import serial
import sys
import glob

""" Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
"""
def serial_ports():
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    #print(str(ports))

    for port in ports:
        try:
            ser = serial.Serial(port)
            #ser.port = port
            #ser.open()
            ser.close()
            result.append(port)
        except (OSError, serial.SerialException ):
            pass

    return result


def serial_choose():
    #serial port
    available_serial = serial_ports()   #levanto la lista de los serial que se pueden abrir.

    if(len(available_serial)==0):
        print("No existen puertos serie disponibles para correr el script")
        return

    if(len(available_serial)==1):
        chosen_serial = available_serial[0]

    else:
        print( "Puerto serie disponibles: ")

        for i in range( len(available_serial)) :
            print( str(i) + ": " + str( available_serial[i] ) )

        ser = input( "Elija un puerto serie para operar:" )

        chosen_serial = available_serial[int(ser)]

    print("Puerto serie elegido : " + chosen_serial)
    return chosen_serial
