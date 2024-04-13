import asyncio
from bleak import BleakClient, BleakScanner
from bleak.backends.scanner import AdvertisementData
from packet import *
import base64
from PIL import Image
from io import BytesIO



CAMERA_UUID = 'somethign'

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

def convert_image(packets):
    all_data = b''.join(packet.data for packet in packets)
    decoded_data = base64.b85decode(all_data)
    image = Image.open(BytesIO(decoded_data))
    
    # We have to save our image in the folder with different name
    # using counter or time?
    # TODO


async def main():
    scanner = BleakScanner()
    device = await scanner.find_device_by_filter(blefilter)

    if device:
        async with BleakClient(device) as client:
            # pass
            # TODO
            # model_number = await client.read_gatt_char(MODEL_NBR_UUID)
            # print("Model Number: {0}".format("".join(map(chr, model_number))))
            packets = await packet_handler(client)
            convert_image(packets)
    else:
        print("No device found.")


if __name__ == '__main__':
    asyncio.run(main())
    

