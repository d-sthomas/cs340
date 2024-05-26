import os
from os import environ
from flask import Flask
import requests
import random
app = Flask(__name__)
FLASK_RUN_HOST="0.0.0.0"

maze_server_url = "http://sp23-cs340-adm.cs.illinois.edu:34000/"
mg_url = "http://sp23-cs340-027.cs.illinois.edu:3400"
r = requests.put(f'{maze_server_url}/addMG', 
        json={
            "name": "diyast2_dynamic_maze",
            "url": mg_url,
            "author": "Diya Thomas"})

@app.route('/generate', methods=['GET'])
def generate_simple():
    mazes = [
        ['988088c','1400404','1000004','0000000','5000044','5222264','3220226'], # smiling =)
        ['98a0a8c','1000004','1000004','0000000','5000044','5222264','3220226'], # closed eyes smile :)
        ['9880a8c','1400004','1000004','0000000','5000044','5222264','3220226'], # winky face ;)
        ['98a0a8c','1000004','1000004','0000000','1222224','1000004','3220226'], # closed eyes blank stare :|
        ['988088c','1400404','1000004','0000000','1222224','1000004','3220226'], # open eyes blank stare =|
        ['988088c','1400404','1000004','0000000','1222224','5000044','3220226'], # sad face =(
        ['98a0a8c','1000004','1000004','0000000','1222224','5000044','3220226'], # sad face :(
        ['98a0a8c','1000004','1000004','0000000','1222264','1000004','3220226'], # smirking :\
        ['988088c','1400404','1000004','0000000','1022204','1422604','3220226'], # =o
        ['988088c','1400404','1000004','0022200','1400404','1422604','3220226'], # =0
        ['988088c','1400404','1000004','0002000','1064204','1400404','3220226'], # ={
        ['988088c','1620004','1400624','0000400','1062004','1040004','3220226'], # stars
    ]
    selected = random.choice(mazes)
    return { "geom": selected }
    