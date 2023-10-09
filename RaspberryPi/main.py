#!/usr/bin/env python3
import serial
import time
import re
import requests
import json
import os
from dotenv import load_dotenv

if __name__ == '__main__':
    load_dotenv()
    ser = serial.Serial('/dev/ttyUSB0', 57600, timeout=1)
    ser.reset_input_buffer()

    pattern = r"output val: ([-+]?[\d]*\.?[\d]+)"
    api_url = os.getenv('API_URL')
    is_sit = False
    while True:
        data = ""
        try :
            data = ser.readline()
        except :
            continue

        decoded_data = data.decode('utf-8', errors='replace')
        stripped_data = decoded_data.strip()
        #print(stripped_data)
        values = re.findall(pattern, stripped_data)
        if len(values) == 2:
            output_val1 = float(values[0])
            output_val2 = float(values[1])
            print(output_val1, output_val2)
            #skip idling data
            if(output_val1 <= 1000 and output_val2 <= 1000):
                #print("test idling , is_sit : ",is_sit)
                if is_sit :
                    is_sit = False
                    #print("Test is_sit = False")
                    try :
                        print("requesting new tare via serial")
                        ser.write("t".encode())
                        #time.sleep(1)
                    except :
                        pass
                continue;
            #negative error
            if(output_val1 <= -1000 or output_val2 <= -1000):
                continue;
            #below here is mean validate success
            is_sit = True
            #print("Test is_sit = True")
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
            print(" Status code", response.status_code)
            #print("Test body", body)
        else:
            print("Unexpected number of values found!")
