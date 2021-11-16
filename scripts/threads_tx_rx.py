import serial, random, time, sys, signal, crc8, string, threading, logging
from os import system
from config import *


#tuto: https://www.studytonight.com/python/python-threading-lock-object
lock_port       = threading.Lock()  #mutex para acceso a puerto serie

'''
Thread de transmision
arg[0] - instancia del puerto serie
arg[1] - Lista con paquetes a enviar
arg[2] - tiempo de inicio de tx/rx
'''
def TransmitThread(arg):
    Port = arg[0]
    paquetes_a_enviar = arg[1]
    init_time = arg[2]

    if len(paquetes_a_enviar) > 0:

        if Port:
            for paquete in paquetes_a_enviar:
                print( "@" + "{:.3f}".format( time.perf_counter() - init_time ) + "---> " + str(paquete) )

                paqete_enviado_encoded = str( paquete ).encode()

                lock_port.acquire()
                Port.write( paqete_enviado_encoded )
                lock_port.release()

                if( T_INTER_FRAME> 0  ):
                    time.sleep( T_INTER_FRAME )
    else:
        print("no hay paquetes para mandar")

'''
Thread de recepcion
arg[0] - instancia del puerto serie
arg[1] - cantiad de mensajes a recibir
arg[2] - referencia a una lista vacia donde se guarda la rta.
arg[3] - tiempo de inicio de tx/rx
'''
def ReceiveThread(arg):
    Port = arg[0]

    cantidad_a_recibir = arg[1]
    respuestas = arg[2]
    init_time = arg[3]
    current_rta = ""

    last_rx_time = time.perf_counter()

    while cantidad_a_recibir>0 and (time.perf_counter()-last_rx_time)<RX_TIMEOUT :
        c = ""

        lock_port.acquire()
        if( Port.inWaiting() > 0 ):
            c = Port.read( Port.in_waiting ).decode('ascii')
        lock_port.release()

        if( len(c) > 0 ):
            current_rta=current_rta+c
            seguir = 1

            while(seguir):
                ini = current_rta.find(SOM)
                end = current_rta.find(EOM)
                #print( str(ini) + " " + str(end) )

                if( ini != -1 and end != -1 ):
                    #hay un paquete en current_rta, lo extraigo
                    rta = current_rta[ini:end+1]
                    current_rta = current_rta.replace(rta,"")
                    #print( "recibido " + rta )
                    print( "@" + "{:.3f}".format( time.perf_counter() - init_time ) + "<--- " + str(rta) )
                    respuestas.append(rta)
                    cantidad_a_recibir = cantidad_a_recibir-1

                    #update time
                    last_rx_time = time.perf_counter()
                else:
                    seguir = 0
            #print(cantidad_a_recibir)
            #print(f'recibido:{recibido}')
        else:
            time.sleep( 0.001 )


    if(current_rta!=""):
        print("thread_rx: buffer que quedo sin procesar:" + current_rta)
