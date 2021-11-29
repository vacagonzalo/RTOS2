from serial import Serial
import time


def main():
    with Serial('/dev/ttyUSB1', 115200) as s:
        indice = 0
        #for indice in range(100):
        #    #Paquete correcto 1
        #    s.write(b'(0000SAbc__dEfghIjklmnOpqrstUvwxyz91)')
        #    #s.write(b'(0097PSkynzaRxuUgwfidcohAqtwlfskdYyonemendSnycvQknayuyyWrteqjzeJcoseqonnRixDmezdjbupaGyczmsnwbtGhmkEA)')
        #    time.sleep(0.05)
        #    res = s.read_all()
        #    print(res)

        #Error de TimeOuts
        s.write(b'(0000Pq_unyjqyjj_x_nnbuuwqjsl_lkmkzcej_wmtuqow4E)')
        time.sleep(0.1)
        res = s.read_all()
        print(res)
        s.write(b'(0000Pq_unyjqyjj_x_nnb')
        time.sleep(0.005)
        s.write(b'uuwqjsl_lkmkzcej_wmtuqow4E)')
        time.sleep(0.1)
        res = s.read_all()
        print(res)
        s.write(b'(0000Pq_unyjqyjj_x_nnb')
        time.sleep(0.003)
        s.write(b'uuwqjsl_lkmkzcej_wmtuqow4E)')
        time.sleep(0.1)
        res = s.read_all()
        print(res)
        
        #Error en ID
        #s.write(b'(0x02alohaFF)')
        #time.sleep(0.1)
        #res = s.read_all()
        #print(res)
                
        #Error en CRC
        #s.write(b'(0003alohaxF)')
        #time.sleep(0.1)
        #res = s.read_all()
        #print(res)
        #s.write(b'(0004alohaFx)')
        #time.sleep(0.1)
        #res = s.read_all()
        #print(res)
             
        #Error en doble SOF
        #s.write(b'(0005alohaxF(0006TestDobleSOF00)')
        #time.sleep(0.1)
        #res = s.read_all()
        #print(res)
        
        # Error No SOF
        #s.write(b'0007alohaxF0006TestDobleSOF00)')
        #time.sleep(0.1)
        #res = s.read_all()
        #print(res)
        
        # Error Invalid Char
        #s.write(b'(0008Paloha?00)')
        #time.sleep(0.1)
        #res = s.read_all()
        #print(res)
        
        #Paquete correcto 2
        #s.write(b'(0009CChauchaFF)')
        #time.sleep(0.1)
        #res = s.read_all()
        #print(res)
        
if __name__ == '__main__':
    main()
