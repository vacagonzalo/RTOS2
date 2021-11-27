from serial import Serial

def main():
    with Serial('/dev/ttyUSB1', 115200, timeout=0.02) as s:
        msg = b'(9886SdeqovNqeuIKnnddiegLezEjqpCsoplxnyHyujwasojCZszdSacthoyocYglnqkfc95)'
        con = b'(9886Sdeqov_nqeu_i_knnddieg_lez_ejqp_csoplxny_hyujwasoj_c_zszd_sacthoyoc_yglnqkfcEA)'
        rta = b''
        eCount = 0
        for i in range(1000):
            s.write(msg)
            rta = s.readline()
            if rta != con:
                eCount = eCount + 1
                rta_w = rta
            print(f'Errores: {str(eCount).zfill(3)} de {str(i + 1).zfill(3)} msg enviados', end='\r')
    print(rta_w)
    print("")
if __name__ == '__main__':
    main()