import socket
import json


class NeoHub:

    def __init__(self, ip):
        self.ip = ip
        self.port = 4242

        # Connect to hub
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.ip, self.port))

    def __socket_send(self, msg):
        totalsent = 0
        while totalsent < len(msg):
            sent = self.sock.send(msg[totalsent:])
            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent

    def __call(self, j, expecting=None):
        payload = bytearray(json.dumps(j) + "\0\r", "utf-8")
        self.__socket_send(payload)

        response = ""
        while True:
            buf = (self.sock.recv(4096)).decode()
            response += buf
            if "\0" in response:
                response = response.rstrip("\0")
                break
            if len(response) == 0:
                break

        # If this fails, an exception will be raised, and Greengrass will retry
        return json.loads(response)

    def get_info(self):
        data = self.__call({"INFO": 0})

        resp = {}
        for d in data['devices']:
            #print("{} {} {}".format(self.__fmt_name(d['device']), d['HEATING'], d['CURRENT_TEMPERATURE']))
            resp[self.__fmt_name(d['device'])] = {'current_temperature': d['CURRENT_TEMPERATURE'], 'active': d['HEATING']}

        return resp

    def close(self):
        self.sock.close()

    def __fmt_name(self, name):
        return name.replace(" ", "")