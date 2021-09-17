# Run a long-running task to test multiple clients access the server at the same
# time. 

import time
import os

def main():
    print(f'LONG.PY [{os.getpid()}] start')
    time.sleep(5)
    print(f'LONG.PY [{os.getpid()}] end')

if __name__ == '__main__':
    main()