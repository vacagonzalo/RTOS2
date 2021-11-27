import serial
import random
import time
import sys
import signal

from crc8 import *

#agrega un crc. Si valido es False, entonces el CRC es invalido
def c2_add_crc(payload, valido = True):
    hash = crc8()
    hash.update(bytes(payload,'utf-8'))
    crc = hash.hexdigest()
    #print(type(crc))
    if valido:
        payload += str( crc ).upper()
    else:
        payload += str( "aa").upper()
    return payload

def c2_add_delimiters(payload,start_of_msg , end_of_msg):
    payload = start_of_msg + payload + end_of_msg
    return payload

def c2_add_seq(payload,sequence):
    payload = str(sequence).zfill(4) + payload
    return payload

#toma una lista de payloads y le formate ael frame y devuelve lista
def c2_create_frame(  payload , start_of_msg , end_of_msg , sec):
    valido = True
    cmd = c2_add_seq ( payload , sec )
    cmd = c2_add_crc( cmd , valido )
    cmd = c2_add_delimiters(cmd, start_of_msg ,end_of_msg)
    return cmd

'''
devuelve el overhead agregado por C2
2 de separadore4s mas 2 de crc mas 4 de secuencia
'''
def c2_overhead():
    return 8


'''
busca una secuencia en una lista de comando/respuesta, y si lo encuientra devuelve el comando/respuesta
#FIX mejorar la busqueda
'''
def c2_buscar_rta_con_secuencia(lista , seq):
    for rta in lista:
        try:
            sec_rx = int(rta[1:5])
        except:
            sec_rx = int(rta[2:6])

        if(sec_rx==seq):
            return rta
    return ""