#include the flask, request, ad jsonify moduels from flask library
from flask import Flask, request, jsonify
import subprocess

from consts import SESSION_NAME_KEY, SESSION_SEARCH_ID_KEY, PORT_KEY
import re

app=Flask(__name__)

# TODO: Remove when using docker in the future
nextAvailablePort = 7777

@app.route('/Session', methods=['POST'])
def CreateServer():
    global nextAvailablePort
    print(dict(request.headers))

    port = nextAvailablePort
    return jsonify({"status": "success", PORT_KEY: port}), 200

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=80)

