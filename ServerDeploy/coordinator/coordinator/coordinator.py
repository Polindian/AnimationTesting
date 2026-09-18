import re

from flask import Flask, request, jsonify
import subprocess
import time

from consts import SESSION_NAME_KEY, SESSION_SEARCH_ID_KEY, PORT_KEY, STATUS_KEY, SESSIONS_KEY

app = Flask(__name__)


# search_id -> {name, port, status, last_seen}
# status: "open" (joinable) or "started" (past team selection)
activeSessions = {}

# Two missed heartbeats before removal, so one dropped packet doesn't kill a live lobby
HEARTBEAT_TIMEOUT_SECONDS = 90


def PruneStaleSessions():
    now = time.time()
    stale = [sid for sid, s in activeSessions.items()
             if now - s["last_seen"] > HEARTBEAT_TIMEOUT_SECONDS]
    for sid in stale:
        print(f"[Coordinator] Pruning stale session {sid}")
        del activeSessions[sid]

def GetUsedPorts():
    result = subprocess.run(['docker', 'ps', '--format', '{{.Ports}}'], capture_output=True, text=True)
    output = result.stdout

    usedPorts = set()

    for line in output.strip().split("\n"):
        matches = re.findall(r'0\.0\.0\.0:(\d+)->', line)
       
        usedPorts.update(map(int, matches))

    return usedPorts
    


def FindNextAvailablePort(start=7777, end=8000):
    usedPorts = GetUsedPorts()
    for port in range(start, end+1):
        if port not in usedPorts:
            return port
        
    return 0

def CreateServerImplementation(sessionName, sessionSearchId):
    port = FindNextAvailablePort()
    print(f"Launching server: {sessionName}, with id: ({sessionSearchId}), on port: {port}")

    proc = subprocess.Popen([
        "docker",
        "run",
        "--rm",
        "-p", f"{port}:{port}/tcp",
        "-p", f"{port}:{port}/udp",
        "server",
        "-server",
        "-log",
        "-epicapp=ServerClient",
        f"-SESSION_NAME={sessionName}",
        f"-SESSION_SEARCH_ID={sessionSearchId}",
        f"-PORT={port}"
    ])

    return port, proc


# The UE server calls this on its heartbeat timer and when its status changes.
# Any message counts as a heartbeat, so status updates refresh last_seen too.
@app.route('/SessionStatus', methods=['POST'])
def UpdateSessionStatus():
    body = request.get_json()
    sessionSearchId = body.get(SESSION_SEARCH_ID_KEY)
    status = body.get(STATUS_KEY)

    if sessionSearchId not in activeSessions:
        return jsonify({"status": "unknown session"}), 404

    if status == "ended":
        del activeSessions[sessionSearchId]
        print(f"[Coordinator] Removed {sessionSearchId}")
        return jsonify({"status": "success"}), 200

    activeSessions[sessionSearchId]["last_seen"] = time.time()

    # "alive" is a pure heartbeat and must not overwrite a "started" session
    if status in ("open", "started"):
        activeSessions[sessionSearchId]["status"] = status
        print(f"[Coordinator] {sessionSearchId} -> {status}")

    return jsonify({"status": "success"}), 200


# The client polls this; anything not returned here is hidden from the browser
@app.route('/Sessions', methods=['GET'])
def ListJoinableSessions():
    PruneStaleSessions()

    joinable = [sid for sid, s in activeSessions.items() if s["status"] == "open"]
    return jsonify({SESSIONS_KEY: joinable}), 200

@app.route('/Session', methods=['POST'])
def CreateServer():
    body = request.get_json()
    sessionName = body.get(SESSION_NAME_KEY)
    sessionSearchId = body.get(SESSION_SEARCH_ID_KEY)

    port, proc = CreateServerImplementation(sessionName, sessionSearchId)

    activeSessions[sessionSearchId] = {
        "name": sessionName,
        "port": port,
        "status": "open",
        "last_seen": time.time(),
        "proc": proc
    }

    print(f"[Coordinator] Created {sessionName} ({sessionSearchId}) on port {port}")
    return jsonify({"status": "success", PORT_KEY: port}), 200


@app.route('/SessionCancel', methods=['POST'])
def CancelServer():
    body = request.get_json()
    sessionSearchId = body.get(SESSION_SEARCH_ID_KEY)
    print(f"[Coordinator] Cancel for '{sessionSearchId}', known: {list(activeSessions.keys())}")

    entry = activeSessions.pop(sessionSearchId, None)
    if not entry:
        return jsonify({"status": "unknown session"}), 404

    if entry.get("proc"):
        entry["proc"].terminate()

    print(f"[Coordinator] Cancelled and killed {sessionSearchId}")
    return jsonify({"status": "success"}), 200

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=80)