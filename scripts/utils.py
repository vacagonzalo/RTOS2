
import   random,  string

def randomword(length):
    letters =  string.ascii_letters
    return ''.join(random.choice(letters) for i in range(length))


'''
genera una lista de un numero aleatorio de cadenas con cantidad de caracteres tambien random
'''
def generar_cadenas(min_word_count , max_word_count , min_char_count , max_char_count):
    words = [ ]
    
    N = random.randint( min_word_count , max_word_count )

    for i in range(N):
        sim_chars = random.randint( min_char_count , max_char_count )
        word = randomword(sim_chars)
        #print(word)
        #print(type(randomword(sim_chars)))
        words.append(word)

    #print(lista)
    return words
