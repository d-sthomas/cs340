from flask import Flask, jsonify, render_template, request
import base64
import glob
from PIL import Image
from scipy import spatial
import numpy as np
import subprocess
import os


servers = {
    "albums_gen": '4000',
    'barbie_gen': '4001',
    "bats_gen" : '4002',
    "books_gen": '4003',
    'flags_gen': '4004',
    'flowers_gen': '4005',
    'food_gen': '4006',
    'landscapes_gen': '4007',
    'movies_gen': '4008',
    'murica_gen': '4009',
    'sky_gen': '4010',
    "tay_gen" : '4011',
    "tv_gen": '4012',
    'reducer': '4013'
}

for s in servers:
    cmd = 'flask -A ' + s + " run --host=0.0.0.0 --port=" + servers[s]
    subprocess.Popen(cmd, shell=True)