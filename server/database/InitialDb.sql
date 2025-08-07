-- Table des clients
CREATE TABLE IF NOT EXISTS client (
    id              SERIAL PRIMARY KEY,
    username        TEXT NOT NULL,
    ip              INET NOT NULL,
    online_status   BOOLEAN NOT NULL DEFAULT TRUE,
    last_seen       TIMESTAMPTZ NOT NULL DEFAULT now(),
    CONSTRAINT unique_client UNIQUE (username, ip)
);

CREATE INDEX IF NOT EXISTS idx_client_online ON client (online_status);

-- Table de l'historique des commandes
CREATE TABLE IF NOT EXISTS command_result (
    id          SERIAL PRIMARY KEY,
    client_id   INTEGER NOT NULL REFERENCES client(id) ON DELETE CASCADE,
    command     TEXT NOT NULL,
    output      TEXT,
    executed_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_command_result_client_id ON command_result (client_id);


-- (Optionnel) Vérification du résultat
-- SELECT * FROM client;
-- SELECT * FROM command_result;
-- SELECT * FROM keylog_entry;



-- Add pending_commands table to track commands sent to clients
-- This table helps correlate commands with their responses using packet IDs

CREATE TABLE IF NOT EXISTS pending_commands (
    id SERIAL PRIMARY KEY,
    client_id INTEGER NOT NULL,
    packet_id INTEGER NOT NULL,
    command TEXT NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE(client_id, packet_id),
    FOREIGN KEY (client_id) REFERENCES client(id) ON DELETE CASCADE
);

-- Add index for faster lookups
CREATE INDEX IF NOT EXISTS idx_pending_commands_client_packet 
ON pending_commands(client_id, packet_id);

-- Optional: Add cleanup function to remove old pending commands
-- (commands that were sent but never received a response after 1 hour)
CREATE OR REPLACE FUNCTION cleanup_old_pending_commands()
RETURNS INTEGER AS $$
DECLARE
    deleted_count INTEGER;
BEGIN
    DELETE FROM pending_commands 
    WHERE created_at < NOW() - INTERVAL '1 hour';
    
    GET DIAGNOSTICS deleted_count = ROW_COUNT;
    RETURN deleted_count;
END;
$$ LANGUAGE plpgsql;

-- You can run this cleanup function periodically:
-- SELECT cleanup_old_pending_commands();