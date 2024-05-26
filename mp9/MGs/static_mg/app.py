from dotenv import load_dotenv
load_dotenv()

import os
from os import environ
from flask import Flask
import requests
import string

app = Flask(__name__)
FLASK_RUN_HOST="0.0.0.0"

maze_server_url = "http://sp23-cs340-adm.cs.illinois.edu:34000"
mg_url = "http://sp23-cs340-027.cs.illinois.edu:4000"
r = requests.put(f'{maze_server_url}/addMG', 
        json={
            "name": "diyast2_static_maze",
            "url": mg_url,
            "author": "Diya Thomas"})

@app.route('/generate', methods=['GET'])
def generate_simple():
    return { "geom": ['988088c','1400404','1000004','0000000','5000044','5222264','3220226'] }

# import requests
# import string
# import time

# url = "http://sp23-cs340-adm.cs.illinois.edu:34003"
# index_code = requests.get(f'{url}/').status_code
# print(str(index_code))
# start_time = time.time()
# first_two = ""
# for c1 in string.ascii_uppercase + string.ascii_lowercase + string.digits + "-_":
#     for c2 in string.ascii_uppercase + string.ascii_lowercase + string.digits + "-_":
#         route = f"/{c1}{c2}"
#         response_code = requests.get(url + route).status_code
#         if response_code == index_code:
#             print(f"Found secret route: {route}")
#             first_two = route
#             break
#     else:
#         continue
#     break
# for c3 in string.ascii_uppercase + string.ascii_lowercase + string.digits + "-_":
#     route = f"{first_two}{c3}"
#     response_code = requests.get(url + route).status_code
#     if response_code == index_code:
#         print(f"Found secret route: {route}")
#         first_two = route
#         break

# end_time = time.time()
# print(f"Time taken: {end_time - start_time} seconds")
# requests_per_second = (end_time - start_time) / 64
# print(f"Requests per second: {requests_per_second}")

