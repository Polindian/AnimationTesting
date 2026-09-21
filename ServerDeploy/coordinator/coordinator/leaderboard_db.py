import os
import sqlite3
import time
from contextlib import closing
 
# /data is the Docker volume, so the file outlives the container
DB_PATH = os.environ.get("LEADERBOARD_DB_PATH", "/data/leaderboard.db")
 
MAX_NAME_LENGTH = 12
MAX_LEADERBOARD_ROWS = 100
 
 
def _Connect():
    return sqlite3.connect(DB_PATH)
 
 
def InitDatabase():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    # closing() shuts the connection; the second "conn" commits (or rolls back on error)
    with closing(_Connect()) as conn, conn:
        conn.execute("""
            CREATE TABLE IF NOT EXISTS players (
                puid       TEXT PRIMARY KEY,
                name       TEXT NOT NULL,
                wins       INTEGER NOT NULL DEFAULT 0,
                losses     INTEGER NOT NULL DEFAULT 0,
                kills      INTEGER NOT NULL DEFAULT 0,
                deaths     INTEGER NOT NULL DEFAULT 0,
                updated_at REAL NOT NULL
            )
        """)
    print(f"[Leaderboard] Database ready at {DB_PATH}")
 
 
def RecordMatch(players):
    """players: list of dicts with puid, name, kills, deaths, won (already validated)."""
    now = time.time()
    with closing(_Connect()) as conn, conn:
        for p in players:
            won = 1 if p["won"] else 0
            # First match inserts the row; later matches add onto it.
            # Name is overwritten so Steam name changes show up.
            conn.execute("""
                INSERT INTO players (puid, name, wins, losses, kills, deaths, updated_at)
                VALUES (?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT(puid) DO UPDATE SET
                    name       = excluded.name,
                    wins       = wins   + excluded.wins,
                    losses     = losses + excluded.losses,
                    kills      = kills  + excluded.kills,
                    deaths     = deaths + excluded.deaths,
                    updated_at = excluded.updated_at
            """, (p["puid"], p["name"], won, 1 - won, p["kills"], p["deaths"], now))
 
 
def GetLeaderboard():
    with closing(_Connect()) as conn:
        rows = conn.execute(
            "SELECT name, wins, losses, kills, deaths FROM players"
        ).fetchall()
 
    def SortKey(row):
        name, wins, losses, kills, deaths = row
        games = wins + losses
        winRate = wins / games if games else 0.0
        kd = kills / deaths if deaths else float(kills)
        return (-wins, -winRate, -kd)
 
    rows.sort(key=SortKey)
    return rows[:MAX_LEADERBOARD_ROWS]
 
 
def ResetLeaderboard():
    with closing(_Connect()) as conn, conn:
        count = conn.execute("DELETE FROM players").rowcount
    print(f"[Leaderboard] Reset — removed {count} players")
    return count