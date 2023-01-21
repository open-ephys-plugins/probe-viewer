import requests

msg = "ProbeA-AP "

region_ids = [947, 322, 895, 631, 178, 128, 125, 947]

for i in range(len(region_ids)):
    for j in range(48):
        msg += f'{i*48+j},{region_ids[i]};'

print(msg)
        
r = requests.put(
    "http://localhost:37497/api/processors/101/config",
    json={"text" : msg})