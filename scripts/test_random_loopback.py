
import serial, random, time, sys, signal, crc8, string, threading, logging
from config import *
from os import system
from serial_init import *
from c2 import *
from utils import *
from threads_tx_rx import *

#configuracion particular del test
DEFAULT_CMD_NRO =  2000

Port = None
WaitTime = 0.01
t_repeat = 1

sec = 0
mensajes_sin_recibir = 0

#tuto: https://www.studytonight.com/python/python-threading-lock-object
lock_port       = threading.Lock()


#SIGNAL TRAP
def signal_handler(sig, frame):
    print('Se pulso Ctrl+C!, saliendo del programa!')
    try:
        Port.close()
    except:
        pass
    else:
        print('Se cerro el puerto OK.')
    sys.exit(0)


'''
crea un paquete completo
'''
def create_pkt_c2( payload , som , eom , sequence , valido = False ):
    pkt =  c2_create_frame( payload , som , eom , sequence    )
    return pkt

'''
Crea un aray de comandos y sus respectivas respuestas correctas.
'''
def create_pkts_c2( payloads , som , eom , valido = False ):
    global sec
    lista = []
    respuestas = []

    for i in range(0 , len(payloads) ):
        sec = i
        sequence = str(sec).zfill(4)
        cmd = create_pkt_c2( payloads[i] , som , eom , sequence , valido )
        rta = cmd

        lista.append(cmd)
        respuestas.append(rta)

    return lista , respuestas

#genera todos los mensajes y sus respuestas correctas
def modo_random():

    mensajes = []

    for i in range(DEFAULT_CMD_NRO):
        sim_chars = random.randint(CANT_LETRAS_MIN,CANT_LETRAS_MAX)
        sim_words = random.randint(CANT_PALABRAS_MIN , CANT_PALABRAS_MAX)
        mensaje = randomword(sim_chars*sim_words)
        mensajes.append(mensaje)

    #genero los frames de todos los comandos y sus respuestas esperadas
    frames , respuestas = create_pkts_c2( mensajes , SOM , EOM , True )

    return frames , respuestas

'''
main program
'''
def main():
    signal.signal(signal.SIGINT, signal_handler)

    global Port

    #platform
    print("Plataforma: " + sys.platform )

    #seleccion de puerto serie
    puerto = serial_choose()
    print("Puerto serie elegido : " + puerto)

    if(not puerto):
        return

    #declaro el objeto de puerto serie (sin abrirlo)
    Port = serial.Serial()

    #parametros del puerto serie
    Port.port       = puerto
    Port.baudrate   = DEFAULT_BAURDATE
    Port.bytesize   = serial.EIGHTBITS      # number of bits per bytes # SEVENBITS
    Port.parity     = serial.PARITY_NONE    # set parity check: no parity # PARITY_ODD
    Port.stopbits   = serial.STOPBITS_ONE   # number of stop bits # STOPBITS_TWO

    try:
        Port.open()

    except Exception as e:
        print("Error abriendo puerto serie.\n" + str(e) + '\nFin de programa.')
        exit()

    logging.basicConfig(
        level=logging.DEBUG,
        format="%(relativeCreated)6d %(threadName)s %(message)s"
    )

    Port.flushInput()       # flush input buffer, discarding all its contents
    Port.flushOutput()      # flush output buffer, aborting current output
                            # and discard all that is in buffer

    #lista vacia para almacenar respuestas
    respuestas = []

    #genero comandos y respuestas esperadas
    comandos_a_enviar , respuestas_esperadas = modo_random()

    global_init_time = time.perf_counter()

    #parametros de los threads
    info_tx = [ Port , comandos_a_enviar , global_init_time ]                      # comandos + time ref
    info_rx = [ Port , len(comandos_a_enviar) , respuestas , global_init_time ]    # numero de comandos a recibir + timeref

    global th_tx
    th_tx = threading.Thread( target=TransmitThread , args=( info_tx ,) )

    global th_rx
    th_rx = threading.Thread( target=ReceiveThread , args=( info_rx ,) )

    th_tx.start()
    th_rx.start()

    th_tx.join()
    th_rx.join()

    err_count               = 0
    err_count_skipped       = 0
    errores                 = 0
    mensajes_recibidos      = len(respuestas)
    mensajes_sin_recibir    = len(respuestas_esperadas ) - mensajes_recibidos

    for i in range(mensajes_recibidos):

        #FIX mejorar la busqueda

        sec_esp = int(respuestas_esperadas[i][1:5])

        rta = c2_buscar_rta_con_secuencia( respuestas , sec_esp )

        if(rta!=""):
            if( rta != respuestas_esperadas[i]):
                print( "                esperado: " + respuestas_esperadas[i])
                print( "                recibido: " + rta)
                err_count=err_count+1
                print(f'{len(respuestas_esperadas[i])}|{len(rta)}')
        else:
            print( "                esperado sin rta: " + respuestas_esperadas[i])
            err_count_skipped=err_count_skipped+1

    err_count_skipped =    err_count_skipped+DEFAULT_CMD_NRO-mensajes_recibidos

    errores = err_count + err_count_skipped

    if(err_count_skipped>0):
        print(f'Se perdieron: {err_count_skipped} respuestas')

    if(errores > 0):
        print(f'Hubo {errores} errores en {DEFAULT_CMD_NRO} paquetes')

    print(f'Procesados con exito el {(DEFAULT_CMD_NRO-errores)/DEFAULT_CMD_NRO*100}% de los paquetes')


if __name__ == "__main__":
    main()