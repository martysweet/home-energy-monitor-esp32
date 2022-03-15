import os
import time
import json
from neohub import NeoHub

gg_client = greengrasssdk.client('iot-data')

# Get Environmental Variables
try:
    poll_delay = int(os.environ['POLL_DELAY'])
    neohub_host = os.environ['NEOHUB_HOST']
except Exception as e:
    print("Failed parsing environmental variables: {}".format(e))
    raise


# CloudWatch Setup
cw_namespace = 'House/Monitoring/Heating'
cw_topic = 'cloudwatch/metric/put'


def fetch_and_transmit():
    n = NeoHub(neohub_host)

    # Get the device | active | temp information
    data = n.get_info()
    #print(data)

    for zone_name in data:
        values = data.get(zone_name, None)
        if values is not None:

            payload_active = {
                "request": {
                    "namespace": cw_namespace,
                    "metricData": {
                        "metricName": "HeatingActive",
                        "dimensions": [
                            {
                                'name': 'Device',
                                'value': 'NeoHub'
                            },
                            {
                                'name': 'Zone',
                                'value': zone_name
                            }
                        ],
                        "value": int(values['active']),
                        "timestamp": time.time()
                    }
                }
            }

            payload_current_temp = {
                "request": {
                    "namespace": cw_namespace,
                    "metricData": {
                        "metricName": "CurrentTemperature",
                        "dimensions": [
                            {
                                'name': 'Device',
                                'value': 'NeoHub'
                            },
                            {
                                'name': 'Zone',
                                'value': zone_name
                            }
                        ],
                        "value": float(values['current_temperature']),
                        "timestamp": time.time()
                    }
                }
            }

            # Publish to connector
            # print(payload_active)
            # print(payload_current_temp)
            gg_client.publish(topic=cw_topic, payload=json.dumps(payload_active))
            gg_client.publish(topic=cw_topic, payload=json.dumps(payload_current_temp))

    n.close()

# Loop, let Greengrass handle any errors
while True:
    fetch_and_transmit()
    time.sleep(poll_delay)
