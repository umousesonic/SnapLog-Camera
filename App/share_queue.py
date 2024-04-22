# shared_queue.py
from queue import Queue

# Create a shared queue that will be used to pass the time interval between the GUI and BLE operations.
shared_queue = Queue(maxsize=1)