# config -------------------------------------------------------------------
DEFAULT_BAURDATE = 115200
SOM = '('
EOM = ')'

CANT_PALABRAS_MIN   = 1
CANT_PALABRAS_MAX   = 15
CANT_LETRAS_MIN     = 1
CANT_LETRAS_MAX     = 10

#tiempo del thread d recepcion para dejar de esperar frames y dar por terminado el test.
RX_TIMEOUT = 1

#tiempo de espera entre frames enviados. En segundos.
#puede ser 0
<<<<<<< HEAD
T_INTER_FRAME = 0.001#0.003 #SEGUNDOS
=======
T_INTER_FRAME = 0.005#0.003 #SEGUNDOS
>>>>>>> parent of 2ef02fc... Se agrega control de letras maximas por palabras.
