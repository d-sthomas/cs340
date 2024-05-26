from flask import Flask, jsonify, render_template, request, send_file
import base64
from PIL import Image
from scipy import spatial
import numpy as np
import os
import filetype
import requests

app = Flask(__name__)


TILES_PATH = "tiles/tv"
COLORS = []
TILES = []

def set_tiles_and_colors():
  for p in os.listdir(TILES_PATH):
    path = os.path.join(TILES_PATH, p) # proper path name
    if filetype.is_image(path):
      tile = Image.open(path)
      TILES.append(tile)
      mean_color = np.array(tile).mean(axis=0).mean(axis=0) # average color of tile
      COLORS.append(mean_color)

FLASK_RUN_HOST="0.0.0.0"

MIDDLEWARE_SERVER = "http://sp23-cs340-adm.cs.illinois.edu:1989/"
MG_URL = "http://sp23-cs340-027.cs.illinois.edu:4012"
set_tiles_and_colors()
r = requests.put(f'{MIDDLEWARE_SERVER}/addMMG', 
        data={
          'name': 'tv',
          "url": MG_URL,
          'author': 'Diya Thomas',
          'tileImageCount': 60})

@app.route('/', methods=['POST'])
def makeMosaic():
  f = request.files["image"]
  renderedTileSize = int(request.args.get('renderedTileSize'))
  tilesAcross = int(request.args.get('tilesAcross'))
  file_format = request.args.get('fileFormat')

  try:  
    base = Image.open(f)
    resized = []
    for tile in TILES:
      resized.append(tile.resize((renderedTileSize, renderedTileSize)))
    tree = spatial.KDTree(COLORS)
    tile_dim = base.size[0] / tilesAcross # determines tile size based on tilesAcross
    tiles_vert = int(np.floor(base.size[1] / tile_dim)) # number of tiles vertically
    
    # adjusts height and width based on tileSize
    new_width = tilesAcross*renderedTileSize
    new_height = tiles_vert*renderedTileSize
    output = Image.new('RGB', (new_width, new_height)) # updated size


    x_idx = 0 # where to paste tiles
    y_idx = 0
    for i in range(tilesAcross): 
      for j in range(tiles_vert):
        x0, y0 = i*tile_dim, j * tile_dim # size of tiles
        x1, y1 = x0+tile_dim, y0+tile_dim
        tile = base.crop((x0,y0,x1,y1)) # crops tile from base
        tile_color = np.array(tile).mean(axis=0).mean(axis=0) # finds average color
        closest = tree.query(tile_color) # returns (distance, index)
        output.paste(resized[closest[1]], (x_idx, y_idx))
        y_idx+=renderedTileSize
      y_idx = 0
      x_idx += renderedTileSize
    
    ...
    base.close()
    # Example - Prepare a single mosaic response:
    if file_format == 'PNG':
      output_path = "outputs/" + f.filename + TILES_PATH[6:] + ".png"
      output.save(output_path)
      return send_file(output_path, mimetype='image/png')
    elif file_format == 'JPEG':
      output_path = "outputs/" + f.filename + TILES_PATH[6:] + ".jpeg"
      output.save(output_path)
      return send_file(output_path, mimetype='image/jpeg')
  except:
     return 'No image', 500