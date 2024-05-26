from flask import Flask, render_template, send_file, request
import os

app = Flask(__name__)

filenames = []
# Route for "/" for a web-based interface to this micro-service:
@app.route('/')
def index():
  return render_template("index.html")


# Extract a hidden "uiuc" GIF from a PNG image:
# extract returns 0 for success, 3 for invalid, 1 for no uiuc chunk
@app.route('/extract', methods=["POST"])
def extract_hidden_gif():
  # ...
  os.makedirs("temp", exist_ok=True)
  
  img_name = request.files['png'].filename
  gif_name = img_name[:-3] + "gif"
  cmd = "./png-extractGIF " + "sample/" + img_name + " temp/" + gif_name

  hidden = os.system(cmd) >> 8
  if hidden == 2: 
    return "Not a valid PNG", 422
  if hidden == 1:
    return "No hidden GIF", 415
  if hidden == 0:
    gif_path = "temp/" + gif_name
    global filenames
    filenames.append(gif_path)
    return send_file(gif_path), 200
  return "Error: " + str(hidden), 500
  
  

# Get the nth saved "uiuc" GIF:
@app.route('/extract/<int:image_num>', methods=['GET'])
def extract_image(image_num):
  # ...
  global filenames
  if image_num >= len(filenames):
    return "Not found", 404
  return send_file(filenames[image_num]), 200