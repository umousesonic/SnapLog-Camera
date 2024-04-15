import asyncio
from bleak import BleakClient, BleakScanner
from bleak.backends.scanner import AdvertisementData
from packet import *
from datetime import datetime
from os import path, makedirs
import time



CAMERA_SERVICE_UUID = "018ed48a-a65c-718d-a58e-dae287ec41fd" 
IMAGE_CHAR_UUID = "018ed48c-1ea0-73d5-ab67-af89e0d89179"
ACK_CHAR_UUID = "018ed48d-2f98-76de-bf56-ef87e9d89281"
IMG_PATH = 'img'

def blefilter(device, data: AdvertisementData) -> bool:
    # print(data.service_uuids)

    if CAMERA_SERVICE_UUID in data.service_uuids:
        return True
    return False

async def packet_handler(client):
    packets = []
    # service = client.get_service(CAMERA_SERVICE_UUID)
    image_char = None
    ack_char = None
    
    # services = await client.get_services()
    for s in client.services:
        # print(s.uuid)
        if s.uuid == CAMERA_SERVICE_UUID:
            print('Found service')
            image_char = s.get_characteristic(IMAGE_CHAR_UUID)
            ack_char = s.get_characteristic(ACK_CHAR_UUID)
        
    
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


async def main():
    scanner = BleakScanner()
    print("Finding device")

    device = await scanner.find_device_by_filter(blefilter)
    
    if device:
        async with BleakClient(device) as client:
            print("Connected to device.")
            packets = await packet_handler(client)
            save_img(packets)
    else:
        print("No device found.")


if __name__ == '__main__':
    asyncio.run(main())
    

