import asyncio
import bleak

# Callback function to handle incoming notifications
def handle_notification(sender: int, data: bytearray):

    d0_bin = bin(data[0])
    print(f"\033[F Value: [{data[1]}, {data[2]}, {data[3]}, {data[4]}, {data[5]}], Button Pressed: {d0_bin}  ")      

    # TODO: Handle data

async def connect_and_read():
    device_name         = 'SRAT+:001'
    service_uuid        = "be30f8d4-4711-11ee-be56-0242ac120002"
    characteristic_uuid = "be30f8d4-4711-11ee-be56-0242ac120003"

    # Discover devices
    devices = await bleak.discover()

    print('\033c', end='') # Clear console
    
    device = None
    print("Trying to connect")
    for dev in devices:
        if dev.name == device_name:            
            device = dev
            break

    if device is None:
        print(f"Device '{device_name}' not found.")
    else:
        print("Device found connecting ...")
        async with bleak.BleakClient(device) as client:
            print("BLE init complete")
            print("")
            # Enable notifications for the desired characteristic
            await client.start_notify(characteristic_uuid, handle_notification)
    
            # Run the script until interrupted
            while True:
                await asyncio.sleep(1)

# Run the connection and reading loop
asyncio.run(connect_and_read())