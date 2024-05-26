Open two terminals. In one, run flask run --host=0.0.0.0 --port=4000 In the other, cd into MGs and run flask run --host=0.0.0.0 --port=8000.

The MMG sends a PUT request to the middleware to let it know that it has been added. From this request, the middleware
has the MMG server url

The middleware sends a POST request to the MMG url with the renderedTileSize and tilesAcross.