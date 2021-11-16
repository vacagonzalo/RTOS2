import random, string, re

CANT_PALABRAS_MIN = 1
CANT_PALABRAS_MAX = 15
CANT_LETRAS_MIN = 1
CANT_LETRAS_MAX = 10

def randomword(length):
    letters =  string.ascii_letters.lower()
    return ''.join(random.choice(letters) for i in range(length))

#rompe el payload, en distintas palabras.
def case_to_list(word):
    list = []

    tipo = "camel"

    if '_' in word:
        tipo = "snake"

    if word[0] == word[0].upper():
        tipo = "pascal"

    if tipo == "camel":
        return re.sub( r"([A-Z])", r" \1",word).split()
    if tipo == "pascal":
        return re.sub( r"([A-Z])", r" \1",word).split()
    if tipo == "snake":
        return re.split( '_+',word)


def PascalCase(words):
    return ''.join(''.join([w[0].upper(), w[1:].lower()]) for w in words)

def snake_case(words):
    return '_'.join(''.join([w[0].lower(), w[1:].lower()]) for w in words)

def camelCase(words):
    aux = PascalCase(words)
    return aux[0].lower() + aux[1:]

words = []

N = random.randint(CANT_PALABRAS_MIN,CANT_PALABRAS_MAX)
for i in range(N):
    sim_chars = random.randint(CANT_LETRAS_MIN,CANT_LETRAS_MAX)
    word = randomword(sim_chars)
    words.append(word)

'''
print (f'[{len(words)}]:{words}')
print (f'camelCase: {camelCase(words)}')
print (f'PascalCase: {PascalCase(words)}')
print (f'snake_case: {snake_case(words)}')

print (f'P_to_C: {camelCase(case_to_list(PascalCase(words)))}')
print (f'P_to_S: {snake_case(case_to_list(PascalCase(words)))}')

print (f'S_to_C: {camelCase(case_to_list(snake_case(words)))}')
print (f'S_to_P: {PascalCase(case_to_list(snake_case(words)))}')

print (f'C_to_P: {PascalCase(case_to_list(camelCase(words)))}')
print (f'C_to_S: {snake_case(case_to_list(camelCase(words)))}')
'''
