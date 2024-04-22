import asyncio
from bleak import BleakClient, BleakScanner
from bleak.exc import BleakError 
from bleak.backends.scanner import AdvertisementData
from packet import *
from datetime import datetime
from os import path, makedirs
import time
from camGUI import PictureCaptureApp
import threading
from share_queue import shared_queue
import struct
# once found the device, we set this global for other function to use
# temp = time_interval

# main code
CAMERA_SERVICE_UUID = "018ed48a-a65c-718d-a58e-dae287ec41fd" 
IMAGE_CHAR_UUID = "018ed48c-1ea0-73d5-ab67-af89e0d89179"
ACK_CHAR_UUID = "018ed48d-2f98-76de-bf56-ef87e9d89281"
TIME_CHAR_UUID = "018ef40c-5951-7243-8487-46b3dd687d74"
IMG_PATH = 'img'

def blefilter(device, data: AdvertisementData) -> bool:
    # print(data.service_uuids)

    if CAMERA_SERVICE_UUID in data.service_uuids:
        return True
    return False


async def packet_handler(client, new_time_interval):
    packets = []
    # service = client.get_service(CAMERA_SERVICE_UUID)
    image_char = None
    ack_char = None
    time_char = None
    
    # services = await client.get_services()
    for s in client.services:
        # print(s.uuid)
        if s.uuid == CAMERA_SERVICE_UUID:
            print('Found service')
            
            image_char = s.get_characteristic(IMAGE_CHAR_UUID)
            ack_char = s.get_characteristic(ACK_CHAR_UUID)
            time_char = s.get_characteristic(TIME_CHAR_UUID)
        
    if new_time_interval is not None:
        # TODO
        # Here you would normally set the time interval on the device
        await client.write_gatt_char(time_char, struct.pack('<I', new_time_interval))

    while True:
        packet_bin = await client.read_gatt_char(image_char)
        packet = Datapacket()
        # print(f"Data looks like {packet_bin}, len={len(packet_bin)}")
        packet.from_bin(packet_bin)                 
        packets.append(packet)
        print(f"Got datapacket id={packet.id}")
        ack = Ackpacket(packet.id)
        await client.write_gatt_char(ack_char, ack.get_bin())
        if packet.id == packet.totalpkts:
            break
    
    await client.write_gatt_char(ack_char, ack.get_bin())



    return packets

def save_img(packets):
    filename = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    filename += '.jpg'
    if path.isfile(filename):
        print(f'Warning: {filename} exists. Skipping.')
        return False
    if not path.exists(IMG_PATH): 
        makedirs(IMG_PATH) 
    with open(IMG_PATH+'/'+filename, '+wb') as f:
        for i in packets:
            f.write(i.data)
    
    return True


def bluetooth_task():
    async def run_ble_operations():
        global shared_queue
        new_time_interval = None
        
        while True:
            try:
                # Proceed with BLE operations
                scanner = BleakScanner()
                print("Finding device")
                device = None
                while device is None:
                    device = await scanner.find_device_by_filter(blefilter)
                
                if device:
                    try:
                        async with BleakClient(device) as client:
                            print("Connected to device.")
                            # Attempt to get the new time interval if available
                            if not shared_queue.empty():
                                new_time_interval = shared_queue.get_nowait()
                            print(f"new time to capture is {new_time_interval}")
                            packets = await packet_handler(client, new_time_interval)
                            save_img(packets)
                            print("Saving image")
                    except BleakError as e:
                        print(e)
                else:
                    print("No device found.")
                await asyncio.sleep(1)
            except Exception as e:
                print(f"Error during BLE operations: {e}")
    asyncio.run(run_ble_operations())




def main():
    # Start the GUI in the main thread
    app = PictureCaptureApp()
    app.run()

if __name__ == '__main__':
    ble_thread = threading.Thread(target=bluetooth_task)
    ble_thread.start()

    main()
    

