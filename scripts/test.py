from serial import Serial

def main():
<<<<<<< HEAD
    with Serial('/dev/ttyUSB0', 115200, timeout=0.0157) as s:
        msg = b'(0034SethardzpIDsPbvbxaayxwJnisbIvztOrhqyhsxdNwttkIaoXxVsfcfsHdcwyyxptZhczenriFpZqk29)'
        con = b'(0034Sethardzp_i_ds_pbvbxaayxw_jnisb_ivzt_orhqyhsxd_nwttk_iao_xx_vsfcfs_hdcwyyxpt_zhczenri_fp_zqk86)'
        rta = b''
=======
    with Serial('/dev/ttyUSB0', 115200, timeout=0.016) as s:
        msg = b'(0012CZstirIrmxzquvbYirkolpC0)'
        con = b'(0012CzstirIrmxzquvbYirkolp08)'
>>>>>>> dbb571a1338d2bdcc1ff99e0088c09adf62f1cb1
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