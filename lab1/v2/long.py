import time
import os

def main():
    print(f'[{os.getpid()}] start')
    time.sleep(5)
    print(f'[{os.getpid()}] end')

if __name__ == '__main__':
    main()