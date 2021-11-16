from cases import *
import  random

C3_OPERATIONS = ['C','P','S']


'''
devuelve el overhead agregado por C3
por ahora solo es el codigo del comando
'''
def c3_overhead():
    return 1


def c3_add_op( msg , op  ):
    return op + msg


def c3_process(msg,op):
    if(op=='m'):
        msg = msg.lower()
    if(op=='M'):
        msg = msg.upper()
    return msg

#genera N operaciones del dictionario, devuelve lista
#  c3_generar_operaciones( 10 , C3_OPERATIONS ) -> genera lista de 10 elementos random tomados de C3_OPERATIONS
def c3_generar_operaciones(N , dict):
    lista = []
    #print(len(dict))
    for i in range(0,N):
        index = random.randint(0,len(dict)-1)
        lista.append(dict[index])
    return lista


#pasandole un payload y una operacion, genera la respuesta correcta
def c3_get_correct_response( operacion , payload, op_error ):
    if op_error:
        return 'E00'

    #C3_OPERATIONS = ['C','P','S']
    if operacion == 'C':
        mensaje = camelCase(case_to_list(payload))
    if operacion == 'P':
        mensaje = PascalCase(case_to_list(payload))
    if operacion == 'S':
        mensaje = snake_case(case_to_list(payload))

    return mensaje

#pasandole una lista de palabras y una operacion, genera una respuesta diferente a la solilcitada por la operacion
#se puede mejorar FIX
def c3_get_other_response( operacoin , palabras ):

    if operacoin == 'C':
        mensaje = PascalCase(palabras)
    if operacoin == 'P':
        mensaje = snake_case(palabras)
    if operacoin == 'S':
        mensaje = camelCase(palabras)

    return mensaje
