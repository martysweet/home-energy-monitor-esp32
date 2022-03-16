import os
import time
import json
from neohub import NeoHub

# Get Environmental Variables
try:
    poll_delay = int(os.getenv("POLL_DELAY", 10))
    neohub_host = os.getenv("NEOHUB_HOST", "")
except Exception as e:
    print("Failed parsing environmental variables: {}".format(e))
    raise

def fetch_and_transmit():
    n = NeoHub(neohub_host)

    # Get the device | active | temp information
    data = n.get_info()
    for zone_name in data:
        values = data.get(zone_name, None)
        if values is not None:

            print(zone_name)
            # zone_name
            # heat_demand{}
            # values['active']

            # tempreature{}
            # float(values['current_temperature'])


    n.close()

# Loop, let Greengrass handle any errors
while True:
    fetch_and_transmit()
    time.sleep(poll_delay)
