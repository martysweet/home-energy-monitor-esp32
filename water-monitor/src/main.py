from pyModbusTCP.client import ModbusClient
from prometheus_client import start_http_server, Gauge
import time
import os

try:
    host = os.getenv('MODBUS_HOST', "")
    poll_delay = int(os.getenv('POLL_DELAY', 10))
    k = int(os.getenv('WATER_METER_K', 10))     # Unit of the pulse meter, i.e. K=10L (Litres)
except Exception as e:
    print("Failed parsing environmental variables: {}".format(e))


water_litres = Gauge('litres', 'Litres of water consumed', ['source'])


def read_values():
    c = ModbusClient(host=host, auto_open=True, auto_close=True)

    while True:
        new_value = int(c.read_holding_registers(43, 1)[0])
        print("New Value: {}".format(new_value))

        # Use rate() to handle rollovers
        litres = new_value * k
        water_litres.labels(source='mains').set(litres)

        time.sleep(poll_delay)


if __name__ == '__main__':
    # Start up the server to expose the metrics.
    start_http_server(7080)
    # Read the values
    read_values()
