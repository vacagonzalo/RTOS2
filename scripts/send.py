from serial import Serial
import time


def main():
    with Serial('/dev/ttyUSB1', 115200) as s:
        s.write(b'(0000alohaFF)')
        time.sleep(1)
        res = s.read_all()
        print(res)
        s.write(b'(0001al')
        time.sleep(0.005)
        s.write(b'ohaFF)')
        time.sleep(1)
        res = s.read_all()
        print(res)


if __name__ == '__main__':
    main()
