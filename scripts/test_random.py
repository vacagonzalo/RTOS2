
import serial, random, time, sys, signal, crc8, string, threading, logging
from config import *
from os import system
from serial_init import *
from c2 import *
from c3 import *
from cases import *
from utils import *
from threads_tx_rx import *

#los o tros dos errores no se pueden forzar de manera determinista.
Errors =  [ "INVALID_DATA", "OPCODE" ,  "NO_MEM" ] # [ "OPCODE" , "INVALID_DATA"  , "UNKNOWN"]
INVALID_ERROR = len(Errors)
ERR_DATA    =   Errors.index("INVALID_DATA")
ERR_OPCODE  =   Errors.index("OPCODE")
ERR_MEM     =   Errors.index("NO_MEM")


#configuracion particular del test
DEFAULT_CMD_NRO = 10000
DEFAULT_ERRORS  = round(0.1 * DEFAULT_CMD_NRO)   #Asi falla el 10%

Port = None
WaitTime = 0.01
t_repeat = 0.1

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
def create_pkt( operacion , payload , som , eom , sequence , error , rta = False):

    l = len(payload)

    if(error !=INVALID_ERROR and rta == False):
        #romper algo del paquete
        if(error==ERR_OPCODE):
            operacion = 'W' #operacion invalida
        if(error==ERR_DATA):
            payload = '?'*l #cualquier dato

    if(error !=INVALID_ERROR and rta == True):
        if(error==ERR_DATA): #operaton error TODO no se puede extraer el indice del array ?
            operacion = ''
            payload = 'E00'
        if(error==ERR_OPCODE): #data error TODO no se puede extraer el indice del array ?
            operacion = ''
            payload = 'E01'
        if(error==ERR_MEM): #nomeme error TODO no se puede extraer el indice del array ?
            operacion = ''
            payload = 'E02'

    temp = c3_add_op( payload , operacion  )
    #print( temp)
    pkt =  c2_create_frame( temp , som , eom , sequence  )
    return pkt

'''
Crea un aray de comandos y sus respectivas respuestas correctas.
'''
def create_pkts( operaciones , payloads , som , eom , num_brokens ):
    global sec
    lista = []
    respuestas = []
    added_errors = 0

    if(len(operaciones)!=len(payloads) ):
        print("Error")
        return lista

    for i in range(len(operaciones) ):
        sec = i
        sequence = str(sec).zfill(4)

        op_error        = False
        data_error      = False
        mem_error       = False
        unknown_error   = False

        e = INVALID_ERROR

        if num_brokens>0:
            ee = random.randint(0,99)
            if(ee>50):   #con prob 0.5 genero o o no error en esta ronda.
                num_brokens  = num_brokens  - 1
                added_errors = added_errors + 1

                #genero un error random
                e = random.randint(0 , 1)   #random entre los primeros 2 errores, el resto no se puede forzar
                #e = 0          #Aca fuerzo un error en especifico

                print(f'Paquete roto aproposito con error: {Errors[e]}')

        #print(f'{op_error}|{data_error}|{mem_error}|{unknown_error}')
        cmd = create_pkt( operaciones[i] , payloads[i] , som , eom , sequence , e , False )
        #print(cmd)
        rta = create_pkt( cmd[5] , c3_get_correct_response(operaciones[i] , payloads[i], op_error) , som , eom , sequence , e , True)
        #print(rta)

        lista.append(cmd)
        respuestas.append(rta)

    return lista , respuestas , added_errors

#genera todos los mensajes y sus respuestas correctas
def modo_random():

    mensajes = []

    #genero 1 operacio por comado
    operaciones = c3_generar_operaciones( DEFAULT_CMD_NRO , C3_OPERATIONS )

    #print(operaciones)
    # Operaciones = [parar generar mensaje , convertir a]

    for i in range(DEFAULT_CMD_NRO):
        #palabras de 1 comando
        palabras = generar_cadenas( CANT_PALABRAS_MIN , CANT_PALABRAS_MAX , CANT_LETRAS_MIN , CANT_LETRAS_MAX )

        #aqui formateo la cadena en OTRO tipo de case del que se genero en el array, asi la educiaa siempre hace algo.
        #Se puede mejorar

        mensaje = c3_get_other_response( operaciones[i] , palabras )
        mensajes.append(mensaje)

    #genero los frames de todos los comandos y sus respuestas esperadas
    frames , respuestas , added_errors = create_pkts( operaciones , mensajes , SOM , EOM , DEFAULT_ERRORS )

    return frames , respuestas , added_errors

def main():
    signal.signal(signal.SIGINT, signal_handler)

    global Port

    print("## TEST PARAMETERS ###############################################################")
    #platform
    print("Plataforma: " + sys.platform )

    #seleccion de puerto serie
    puerto = serial_choose()

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


    comandos_a_enviar , respuestas_esperadas , total_errors = modo_random()

    global_init_time = time.perf_counter()

    #info de la simulacion
    print("Se enviaran: " + str(DEFAULT_CMD_NRO) + " paquetes ")
    print("Cantidad de errores que se inyectaran: " + str(total_errors) +"/"+ str(DEFAULT_CMD_NRO))
    print("##################################################################################")
    print("")

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

    err_count = 0
    err_count_skipped = 0
    errores = 0
    sinerror = 0
    mensajes_recibidos      = len(respuestas)
    mensajes_sin_recibir    = len(respuestas_esperadas )  - mensajes_recibidos

    print("")
    print("## TEST RESULTS ##################################################################")
    
    print("Cantidad respuestas recibidas: "     + str(mensajes_recibidos))     
    print("Cantidad respuestas sin recibir: "   + str(mensajes_sin_recibir))     
    
    for i in range(mensajes_recibidos):

        sec_esp = int(respuestas_esperadas[i][1:5])

        rta = c2_buscar_rta_con_secuencia( respuestas , sec_esp )

        if( rta != "" ):
            if( rta != respuestas_esperadas[i]):
                #respuesta incorrecta
                print( "                esperado: " + respuestas_esperadas[i])
                print( "                recibido: " + rta)
                err_count=err_count + 1
            else:
                #respuesta correcta
                sinerror= sinerror + 1
        else:
            #sin respuesta
            print( "                esperado sin rta: " + respuestas_esperadas[i])
            err_count_skipped=err_count_skipped+1
  
    errores = err_count + err_count_skipped

    #if(err_count_skipped>0):
    #    print(f'Se perdieron: {err_count_skipped} respuestas')

    if(errores > 0):
        print(f'Hubo {errores} errores en {DEFAULT_CMD_NRO} paquetes')

    print(f'Procesados con exito el {(sinerror)/DEFAULT_CMD_NRO*100}% de los paquetes')

    print("##################################################################################")

if __name__ == "__main__":
    main()