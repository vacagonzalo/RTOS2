from serial import Serial

def main():
    with Serial('/dev/ttyUSB1', 115200, timeout=0.02) as s:
        msg = b'(0000SAbcdEfghIjklmnOpqrstUvwxyz91)'
        con = b'(0000Sabcd_efgh_ijklmn_opqrst_uvwxyz1D)'
        eCount = 0
        for i in range(100):
            s.write(msg)
            if s.readline() != con:
                eCount = eCount + 1
            print(f'Errores: {str(eCount).zfill(3)} de {str(i + 1).zfill(3)} msg enviados', end='\r')
    print("")
if __name__ == '__main__':
    main()