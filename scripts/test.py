from serial import Serial

def main():
    with Serial('/dev/ttyUSB0', 115200, timeout=0.016) as s:
        msg = b'(0012CZstirIrmxzquvbYirkolpC0)'
        con = b'(0012CzstirIrmxzquvbYirkolp08)'
        eCount = 0
        for i in range(1000):
            s.write(msg)
            if s.readline() != con:
                eCount = eCount + 1
            print(f'Errores: {str(eCount).zfill(3)} de {str(i + 1).zfill(3)} msg enviados', end='\r')
    print("")
if __name__ == '__main__':
    main()