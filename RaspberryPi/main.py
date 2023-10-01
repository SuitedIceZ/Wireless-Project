#!/usr/bin/env python3
import serial
import re
import requests
import json

if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyUSB0', 57600, timeout=1)
    ser.reset_input_buffer()

    pattern = r"output val: ([-+]?[\d]*\.?[\d]+)"
    api_url = "https://intelliseat-api.pkhing.dev/sensor/log"
    while True:
        data = ser.readline()
        decoded_data = data.decode('utf-8', errors='replace')
        stripped_data = decoded_data.strip()
        print(stripped_data)
        values = re.findall(pattern, stripped_data)
        if len(values) == 2:
            output_val1 = float(values[0])
            output_val2 = float(values[1])
            print(output_val1, output_val2)
            headers =  {"Content-Type":"application/json"}
            body = {
                  "nodeGroup": "chair-1",
                  "sensor": [
                    {
                      "nodeSide": "LEFT",
                      "weight": output_val1
                    },
                    {
                      "nodeSide": "RIGHT",
                      "weight": output_val2
                    }
                  ]
                }
            response = requests.post(api_url, data=json.dumps(body), headers=headers, verify=False)
            print("Status code", response.status_code)
            print("Test body", body)
        else:
            print("Unexpected number of values found!")
