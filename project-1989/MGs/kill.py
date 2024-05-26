from flask import Flask, jsonify, render_template, request
import base64
import glob
from PIL import Image
from scipy import spatial
import numpy as np
import subprocess
import os


ports = ['4000', '4001', '4002', '4003', '4004', '4005', '4006', '4007', '4008', '4009', '4010', '4011', '4012', '4013']

for p in ports:
    cmd = "killport " + p
    out = os.popen(cmd).read()