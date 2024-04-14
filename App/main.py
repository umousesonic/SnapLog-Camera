import asyncio
from bleak import BleakClient, BleakScanner
from bleak.backends.scanner import AdvertisementData
from packet import *
import base64
from PIL import Image
from io import BytesIO
from datetime import datetime
from os import path



CAMERA_UUID = 'somethign'
IMG_PATH = 'img'

async def blefilter(device, data: AdvertisementData) -> bool:
    if CAMERA_UUID in data.service_uuids:
        return True
    return False

async def packet_handler(client):
    packets = []
    while True:
        packet_bin = await client.read_gatt_char(CAMERA_UUID)
        packet = Datapacket()
        packet.from_bin(packet_bin)
        packets.append(packet)
        ack = Ackpacket(packet.id)
        await client.write_gatt_char(CAMERA_UUID, ack.get_bin())
        if packet.id == packet.totalpkts:
            break
    return packets

def save_img(packets):
    filename = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    filename += '.jpg'
    if path.isfile(filename):
        print(f'Warning: {filename} exists. Skipping.')
        return False
    with open(filename, '+w') as f:
        for i in packets:
            f.write(i.data)
    
    return True


async def main():
    scanner = BleakScanner()
    device = await scanner.find_device_by_filter(blefilter)

    if device:
        async with BleakClient(device) as client:
            packets = await packet_handler(client)
            save_img(packets)
    else:
        print("No device found.")


if __name__ == '__main__':
    asyncio.run(main())
    

