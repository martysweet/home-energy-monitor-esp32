import os
import time
from neohub import NeoHub
from prometheus_client import start_http_server, Gauge
from datetime import datetime


# Get Environmental Variables
try:
    poll_delay = int(os.getenv("POLL_DELAY", 15))
    neohub_host = os.getenv("NEOHUB_HOST", "")
except Exception as e:
    print("Failed parsing environmental variables: {}".format(e))
    raise

temperature = Gauge('temperature', 'Temperature of zone', ['zone'])
is_active = Gauge('is_active', 'Zone is active', ['zone'])

def fetch_and_transmit():
    while True:
        n = NeoHub(neohub_host)

        # Get the device | active | temp information
        data = n.get_info()
        my_date = datetime.now()
        print(my_date.isoformat())
        for zone_name in data:
            values = data.get(zone_name, None)
            if values is not None:
                print("Zone - {} - {} - {}".format(zone_name, values['active'], float(values['current_temperature'])))
                temperature.labels(zone=zone_name).set(float(values['current_temperature']))
                is_active.labels(zone=zone_name).set(values['active'])
        n.close()
        time.sleep(poll_delay)


if __name__ == '__main__':
    print("Startup")
    # Start up the server to expose the metrics.
    start_http_server(7080)
    # Read the values
    fetch_and_transmit()

