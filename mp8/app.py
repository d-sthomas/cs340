from itertools import count
import json
from flask import Flask, jsonify, send_file, render_template, request
import requests
import os
import io
import boto3
import botocore
import base64
import dotenv
import logging
import os
import PIL.Image as Image
import redis
import emoji

# http://127.0.0.1:34000/mandelbrot/viridis/0.36:-0.09:0.01:512:64
app = Flask(__name__)
colormap = "viridis"
real = 0.36
imag = -0.09
height = 0.01
dim = 512
iter = 64

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/all')
def all():
    return render_template('all.html')

# moves center of image down by 25% of current height
@app.route('/moveUp', methods=['POST'])
def moveUp(): 
    global imag
    imag = imag + height/4
    return "Center moved up", 200

# moves center of image down by 25% of current height
@app.route('/moveDown', methods=['POST'])
def moveDown(): 
    global imag
    imag = imag - height/4
    return "Center moved down", 200

# moves the center of the image left by 25% of current height
@app.route('/moveLeft', methods=['POST'])
def moveLeft(): 
    global real
    real = real - height/4
    return "Center moved left", 200

# moves the center of the image right by 25% of current height
@app.route('/moveRight', methods=['POST'])
def moveRight(): 
    global real
    real = real + height/4
    return "Center moved right", 200

# modifies the height by a factor of 1 / 1.4
@app.route('/zoomIn', methods=['POST'])
def zoomIn(): 
    global height
    height = height * (1/1.4)
    return "Zoomed in", 200

# modifies the height by a factor of 1.4
@app.route('/zoomOut', methods=['POST'])
def zoomOut(): 
    global height
    height = height * 1.4
    return "Zoomed out", 200

# modifies the dim of the image by a factor of 1 / 1.25
@app.route('/smallerImage', methods=['POST'])
def smallerImage(): 
    global dim
    dim = dim * (1/1.25)
    return "Dim decreased", 200

# modifies the dim of the image by a factor of 1.25
@app.route('/largerImage', methods=['POST'])
def largerImage(): 
    global dim
    dim = dim * 1.25
    return "Dim increased", 200

# modifies the iter of the image by a factor of 2.
@app.route('/moreIterations', methods=['POST'])
def moreIters(): 
    global iter
    iter = iter * 2
    return "Increased iter", 200

# modifies the iter of the image by a factor of 1 / 2.
@app.route('/lessIterations', methods=['POST'])
def lessIters(): 
    global iter
    iter = iter / 2
    return "Decreased iter", 200

# changes the colormap to be equal to the colormap value in the JSON in the request’s body
@app.route('/changeColorMap', methods=['POST'])
def changeColorMap():
    global colormap
    colormap = request.json["colormap"]
    return "Changed colormap", 200

# renders image based on current state
# /mandelbrot/<colormap>/<real>:<imag>:<height>:<dim>:<iter>
# http://127.0.0.1:34000/mandelbrot/viridis/0.36:-0.09:0.01:512:64
@app.route('/mandelbrot', methods=['GET'])
def mandelbrot():
    s3_client = boto3.client('s3',
        endpoint_url="http://127.0.0.1:9000",
        aws_access_key_id="ROOTNAME",
        aws_secret_access_key="CHANGEME123")
    file_name = colormap + '-' + str(real) + ':' + str(imag) + ':' + str(height) + ':' + str(dim) + ':' + str(iter) + ".png"
    response = s3_client.list_buckets()
    buckets =[]
    for bucket in response['Buckets']:
      buckets += {bucket["Name"]}
    if len(buckets) != 0:
        pass
    else:
        s3_client.create_bucket(Bucket='bucket')
        #s3_client.upload_file(file_name, "bucket", file_name)
    try:
        s3_client.head_object(Bucket='bucket_name', Key=file_name)
        s3_client.download_file('bucket', file_name, file_name)
        return send_file(file_name), 200
    except botocore.exceptions.ClientError as e:
        server_url = os.getenv('MANDELBROT_MICROSERVICE_URL')
        r = requests.get(f'{server_url}/mandelbrot/{colormap}/{str(real)}:{str(imag)}:{str(height)}:{str(dim)}:{str(iter)}')
        
        with open(file_name, "wb") as f:
            f.write(r.content)
        f.close()
        s3_client.upload_file(file_name, "bucket", file_name)
        
    return send_file(file_name), 200

# return JSON of every image stored as entries
@app.route('/storage', methods=['GET'])
def getCache():
    s3_client = boto3.client('s3',
        endpoint_url="http://127.0.0.1:9000",
        aws_access_key_id="ROOTNAME",
        aws_secret_access_key="CHANGEME123")
    
    response = s3_client.list_buckets()
    buckets =[]
    for bucket in response['Buckets']:
      buckets += {bucket["Name"]}
    if len(buckets) == 0:
        s3_client.create_bucket(Bucket='bucket')
    objects = s3_client.list_objects_v2(Bucket='bucket')
   
    dict = []
    for obj in objects['Contents']:
        pair = {'key': None, 'value': None}
        file = obj['Key']
        path = file.replace("-", "/", 1)
        s3_client.download_file('bucket', file, file)
        binary_fc = open(file, 'rb').read()  # fc aka file_content
        base64_utf8_str = base64.b64encode(binary_fc).decode('utf-8')
        ext = file.split('.')[-1]
        dataurl = f'data:image/{ext};base64,{base64_utf8_str}'
        pair['key'] = path
        pair['value'] = dataurl
        dict.append(pair)
        print("appended")
    print(dict)
    return dict, 200

# clears cache
@app.route('/clearCache', methods=['GET'])
def clearCache():
    s3_client = boto3.client('s3',
        endpoint_url="http://127.0.0.1:9000",
        aws_access_key_id="ROOTNAME",
        aws_secret_access_key="CHANGEME123")
    
    response = s3_client.list_buckets()
    buckets =[]
    for bucket in response['Buckets']:
      buckets += {bucket["Name"]}
    if len(buckets) == 0:
        s3_client.create_bucket(Bucket='bucket')
    objects = s3_client.list_objects_v2(Bucket='bucket')
    for obj in objects['Contents']:
        file = obj['Key']
        s3_client.delete_object(Bucket='bucket', Key=file)
    return "OK", 200

# resets parameters to values in JSON
@app.route('/resetTo', methods=['POST'])
def reset():
    global colormap
    colormap = request.json['colormap']
    global real
    real = request.json['real']
    global imag
    imag = request.json['imag']
    global height
    height = request.json['height']
    global dim
    dim = request.json['dim']
    global iter
    iter = request.json['iter']
    return "Reset values", 200
    
# return current state as JSON
@app.route('/getState', methods=['GET'])
def getState():
    return {
        "colormap": colormap,
        "dim": dim,
        "height": height,
        "imag": imag,
        "iter": iter,
        "real": real,
    }, 200
    
@app.route('/goldrush', methods=['GET'])
def goldrush():
    s3_client = boto3.client('s3',
        aws_access_key_id="AKIAQUUXEXCMMQ2EYQGD",
        aws_secret_access_key="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")
    file = "2604a880fdc7a2659e67b703585de1cf87e92d81eb6d40d2292d9d8bfe2fbc20.txt"
    with open('./ec.txt', "wb"):
        s3_client.download_file('waf-uiuc-cs340', file, "ec.txt")
   
    
    with open('./ec2.txt', "wb"):
        s3_client.download_file('waf-uiuc-cs340', "b82754edf4ae29f1f3b3d98bed396722a62038664956f8b6e828d5eef422e09e.txt", "ec2.txt")
    
    return "OK", 200

@app.route('/wonderland', methods=['GET'])
def wonderland():
    clue1 = ""
    clue2 = ""
    r1 = redis.Redis(
    host='sp23-cs340-adm.cs.illinois.edu',
    port=6379,
    username='diyast2', 
    password='5bb1327926392c047f9376db05b20407')
    
    prefix  = 'diyast2:page'
    print(r1.get('diyast2:start'))
    for i in range(5, 1000):
        v = str(r1.get(prefix+str(i)))
        if 'clue' in v:
            clue1 = str(i) + " " + v
            print(clue1)
            break
    r2 = redis.Redis(
    host='sp23-cs340-adm.cs.illinois.edu',
    port=6379,
    username='5bb1327926392c047f9376db05b20407', 
    password='84015174bbdb348fe6cbdc20c8a77767')
    
    prefix = '5bb1327926392c047f9376db05b20407:page'
    
    for i in range(5, 1000):
        v = str(r2.get(prefix+str(i)))
        utf8 = r2.get(prefix+str(i)).decode('utf8')
        if "🎉" in utf8:
            clue2 = str(i) + " " + v
            print(clue2)
            break
    clues = {
        "1": clue1,
        "2": clue2
    }
    
    return clues, 200