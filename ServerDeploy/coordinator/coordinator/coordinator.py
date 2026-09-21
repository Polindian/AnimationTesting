import hmac
import os
import re
 
from flask import Flask, request, jsonify
import subprocess
import time
 
from consts import (SESSION_NAME_KEY, SESSION_SEARCH_ID_KEY, PORT_KEY, STATUS_KEY, SESSIONS_KEY,
                    PLAYERS_KEY, PUID_KEY, NAME_KEY, KILLS_KEY, DEATHS_KEY, WON_KEY,
                    WINS_KEY, LOSSES_KEY, ENTRIES_KEY, SERVER_SECRET_HEADER, ADMIN_SECRET_HEADER)
from leaderboard_db import InitDatabase, RecordMatch, GetLeaderboard, ResetLeaderboard, MAX_NAME_LENGTH
 
app = Flask(__name__)
 
# Come from the .env file via docker-compose; never hard-coded or committed
SERVER_SECRET = os.environ.get("SERVER_SECRET", "")
ADMIN_SECRET = os.environ.get("ADMIN_SECRET", "")
 
# Sanity ceiling per match — anything above this is a bug or a forged request
MAX_STAT_PER_MATCH = 999
 
 
# search_id -> {name, port, status, last_seen}
# status: "open" (joinable) or "started" (past team selection)
activeSessions = {}
 
# Two missed heartbeats before removal, so one dropped packet doesn't kill a live lobby
HEARTBEAT_TIMEOUT_SECONDS = 90
 
 
def HasSecret(headerName, expectedSecret):
    # An unset secret must reject everything, not accept an empty header
    if expectedSecret == "":
        return False
    provided = request.headers.get(headerName, "")
    # compare_digest takes the same time whether the guess is close or not
    return hmac.compare_digest(provided, expectedSecret)
 
 
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
        f"-PORT={port}",
        # Only servers launched here know the secret, so only they can post results
        f"-SERVER_SECRET={SERVER_SECRET}"
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
 
 
# ---------------- Leaderboard ----------------
 
def ToWholeNumber(value):
    """Unreal's JSON writer can send 5 as 5.0, so whole floats count as ints."""
    # bool is a subclass of int in Python, so rule it out explicitly
    if isinstance(value, bool):
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, float) and value.is_integer():
        return int(value)
    return None
 
 
def ParseMatchPlayer(raw):
    """Returns a clean player dict, or None if anything is missing or out of range."""
    if not isinstance(raw, dict):
        return None
 
    puid = raw.get(PUID_KEY)
    name = raw.get(NAME_KEY)
    kills = ToWholeNumber(raw.get(KILLS_KEY))
    deaths = ToWholeNumber(raw.get(DEATHS_KEY))
    won = raw.get(WON_KEY)
 
    if not isinstance(puid, str) or puid == "":
        return None
    if not isinstance(name, str) or name.strip() == "":
        return None
    for stat in (kills, deaths):
        if stat is None or not 0 <= stat <= MAX_STAT_PER_MATCH:
            return None
    if not isinstance(won, bool):
        return None
 
    return {
        "puid": puid,
        # The game server already caps names; this is a second line of defence
        "name": name.strip()[:MAX_NAME_LENGTH],
        "kills": kills,
        "deaths": deaths,
        "won": won,
    }
 
 
# Only the dedicated server calls this, once per player at match end
@app.route('/MatchResults', methods=['POST'])
def PostMatchResults():
    if not HasSecret(SERVER_SECRET_HEADER, SERVER_SECRET):
        return jsonify({"status": "forbidden"}), 403
 
    body = request.get_json(silent=True) or {}
    rawPlayers = body.get(PLAYERS_KEY)
    if not isinstance(rawPlayers, list) or len(rawPlayers) == 0:
        return jsonify({"status": "no players"}), 400
 
    players = []
    for raw in rawPlayers:
        player = ParseMatchPlayer(raw)
        if player is None:
            # Skip only this row so everyone else still gets their stats
            print(f"[Leaderboard] Skipped invalid player row: {raw}")
            continue
        players.append(player)
 
    if len(players) == 0:
        return jsonify({"status": "invalid player data"}), 400
 
    RecordMatch(players)
    skipped = len(rawPlayers) - len(players)
    print(f"[Leaderboard] Recorded match with {len(players)} players ({skipped} skipped)")
    return jsonify({"status": "success", "recorded": len(players), "skipped": skipped}), 200
 
 
# The client's leaderboard widget calls this; rows arrive already sorted
@app.route('/Leaderboard', methods=['GET'])
def GetLeaderboardEntries():
    entries = [
        {NAME_KEY: name, WINS_KEY: wins, LOSSES_KEY: losses, KILLS_KEY: kills, DEATHS_KEY: deaths}
        for name, wins, losses, kills, deaths in GetLeaderboard()
    ]
    return jsonify({ENTRIES_KEY: entries}), 200
 
 
# Launch-day wipe of test stats; you run this by hand, the game never calls it
@app.route('/AdminReset', methods=['POST'])
def AdminResetLeaderboard():
    if not HasSecret(ADMIN_SECRET_HEADER, ADMIN_SECRET):
        return jsonify({"status": "forbidden"}), 403
 
    removed = ResetLeaderboard()
    return jsonify({"status": "success", "removed": removed}), 200
 
 
if __name__ == '__main__':
    if SERVER_SECRET == "" or ADMIN_SECRET == "":
        print("[Coordinator] WARNING: SERVER_SECRET or ADMIN_SECRET not set — leaderboard writes will be refused")
 
    InitDatabase()
    app.run(host="0.0.0.0", port=80)