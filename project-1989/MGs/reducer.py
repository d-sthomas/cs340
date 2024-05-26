

from flask import Flask, jsonify, render_template, request, send_file, make_response
import base64
from PIL import Image
from scipy import spatial
import numpy as np
import os
import filetype
import time
import requests
import math


app = Flask(__name__)

TILES_PATH = "tiles/albums"
COLORS = []
TILES = []

FLASK_RUN_HOST="0.0.0.0"

MIDDLEWARE_SERVER = "http://sp23-cs340-adm.cs.illinois.edu:1989/"
MG_URL = "http://sp23-cs340-027.cs.illinois.edu:4013"
r = requests.put(f'{MIDDLEWARE_SERVER}/registerReducer', 
        data={
            "url": MG_URL,
            "author": "Diya Thomas"})

@app.route("/", methods=["POST"])
def reduce():
    baseImage = request.files["baseImage"]
    mosaic1 = request.files["mosaic1"]
    mosaic2 = request.files["mosaic2"]

    base = Image.open(baseImage)
    m1 = Image.open(mosaic1)
    m2 = Image.open(mosaic2)

    renderedTileSize = int(request.args.get("renderedTileSize"))
    tilesAcross = int(request.args.get("tilesAcross"))
    fileFormat = request.args.get("fileFormat")

    mosaics_tile_dim = m1.size[0] / tilesAcross
    output_tile_dim = base.size[0] / tilesAcross
    tiles_vert = int(np.floor(base.size[1] / output_tile_dim))

    new_width = tilesAcross*renderedTileSize
    new_height = tiles_vert*renderedTileSize
    output = Image.new('RGB', (new_width, new_height)) # updated size

    x_idx = 0 # where to paste tiles
    y_idx = 0
    base = base.resize((new_width, new_height))
    for i in range(tilesAcross): 
      for j in range(tiles_vert):
        mx0, my0 = i*mosaics_tile_dim, j * mosaics_tile_dim # size of tiles
        mx1, my1 = mx0+mosaics_tile_dim, my0+mosaics_tile_dim
        tile1 = m1.crop((mx0,my0,mx1,my1)) # crops tile from base
        tile1_color = np.array(tile1).mean(axis=0).mean(axis=0) # finds average color

        tile2 = m2.crop((mx0,my0,mx1,my1)) # crops tile from base
        tile2_color = np.array(tile2).mean(axis=0).mean(axis=0) # finds average color
        
        bx0, by0 = i*mosaics_tile_dim, j * mosaics_tile_dim # size of tiles
        bx1, by1 = bx0+mosaics_tile_dim, by0+mosaics_tile_dim
        tileb = base.crop((bx0,by0,bx1,by1)) # crops tile from base
        tileb_color = np.array(tileb).mean(axis=0).mean(axis=0) # finds average color

        d1 = math.sqrt((tile1_color[0]-tileb_color[0])**2 + (tile1_color[1]-tileb_color[1])**2 + (tile1_color[2]-tileb_color[2])**2)
        d2 = math.sqrt((tile2_color[0]-tileb_color[0])**2 + (tile2_color[1]-tileb_color[1])**2 + (tile2_color[2]-tileb_color[2])**2)
        
        if (d1 > d2):
            output.paste(tile2.resize((renderedTileSize, renderedTileSize)), (x_idx, y_idx))
        else:
           output.paste(tile1.resize((renderedTileSize, renderedTileSize)), (x_idx, y_idx))
        y_idx+=renderedTileSize
      y_idx = 0
      x_idx += renderedTileSize

    # Return a reduced mosaic that combines the best of mosaic1 and mosaic2:
    base.close()
    m1.close()
    m2.close()
    # Example - Prepare a single mosaic response:
    if fileFormat == 'PNG':
      output_path = "outputs/combined.png"
      output.save(output_path)
      return send_file(output_path, mimetype='image/png')
    elif fileFormat == 'JPEG':
      output_path = "outputs/combined.png"
      output.save(output_path)
      return send_file(output_path, mimetype='image/jpeg')
    