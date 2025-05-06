import requests
import json
import time
import random
import logging

# Configurazione Logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# --- Configurazione ---
SERVER_IP = "127.0.0.1"  # Indirizzo IP del tuo server Flask (usa 127.0.0.1 se è sulla stessa macchina)
SERVER_PORT = 3000       # Porta su cui è in ascolto il tuo server Flask
API_ENDPOINT = "/api/data"
DEVICE_ID = "python_simulator_1"
SEND_INTERVAL_SECONDS = 5 # Intervallo di invio dati in secondi
# --------------------

server_url = f"http://{SERVER_IP}:{SERVER_PORT}{API_ENDPOINT}"

def generate_simulated_data():
    """Genera un valore di temperatura simulato."""
    # Simula una temperatura che varia leggermente
    temperature = round(random.uniform(18.0, 28.0), 2)
    timestamp = int(time.time() * 1000) # Timestamp approssimativo in millisecondi
    data = {
        "temperature": temperature,
        "device_id": DEVICE_ID,
        "timestamp": timestamp
    }
    return data

def send_data_to_server(data):
    """Invia i dati al server Flask tramite HTTP POST."""
    headers = {'Content-Type': 'application/json'}
    try:
        json_payload = json.dumps(data)
        logging.info(f"Invio dati: {json_payload} a {server_url}")
        response = requests.post(server_url, headers=headers, data=json_payload, timeout=10)
        response.raise_for_status() # Solleva un'eccezione per risposte HTTP errate (4xx o 5xx)
        logging.info(f"Dati inviati con successo. Risposta server: {response.status_code} - {response.text}")
        return True
    except requests.exceptions.ConnectionError as e:
        logging.error(f"Errore di connessione: Impossibile connettersi al server a {server_url}. Verifica che il server sia in esecuzione e l'IP/porta siano corretti.")
        logging.error(f"Dettagli errore: {e}")
    except requests.exceptions.Timeout:
        logging.error(f"Errore di Timeout: La richiesta a {server_url} ha impiegato troppo tempo.")
    except requests.exceptions.RequestException as e:
        logging.error(f"Errore durante l'invio dei dati a {server_url}: {e}")
        if e.response is not None:
            logging.error(f"Risposta del server (se disponibile): {e.response.status_code} - {e.response.text}")
    except Exception as e:
        logging.error(f"Errore imprevisto durante l'invio dei dati: {e}")
    return False

if __name__ == "__main__":
    logging.info(f"Avvio simulatore Arduino ({DEVICE_ID}). Invio dati a {server_url} ogni {SEND_INTERVAL_SECONDS} secondi.")
    logging.info("Premi CTRL+C per fermare.")

    while True:
        try:
            simulated_data = generate_simulated_data()
            send_data_to_server(simulated_data)
            time.sleep(SEND_INTERVAL_SECONDS)
        except KeyboardInterrupt:
            logging.info("Simulatore fermato dall'utente.")
            break
        except Exception as e:
            logging.error(f"Errore nel ciclo principale: {e}")
            # Attendi un po' prima di riprovare in caso di errori ripetuti
            time.sleep(SEND_INTERVAL_SECONDS)