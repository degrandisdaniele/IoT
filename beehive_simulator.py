import requests
import json
import time
import random
import logging

# Configurazione Logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# --- Configurazione ---
SERVER_IP = "127.0.0.1"  # Indirizzo IP del server Flask
SERVER_PORT = 3000       # Porta del server Flask
API_ENDPOINT = "/api/data"
DEVICE_ID = "beehive_simulator_1"
SEND_INTERVAL_SECONDS = 5  # Intervallo di invio dati in secondi
# --------------------

server_url = f"http://{SERVER_IP}:{SERVER_PORT}{API_ENDPOINT}"

def generate_simulated_data():
    """Genera dati simulati per un'arnia."""
    # Simula temperatura, umidità e volume sonoro
    temperature = round(random.uniform(20.0, 30.0), 2)  # Temperatura tra 20°C e 30°C
    humidity = round(random.uniform(40.0, 70.0), 2)     # Umidità tra 40% e 70%
    sound = round(random.uniform(30.0, 60.0), 2)        # Volume sonoro tra 30dB e 60dB
    battery = round(random.uniform(50.0, 100.0), 0)     # Livello batteria tra 50% e 100%
    
    timestamp = int(time.time() * 1000)  # Timestamp in millisecondi
    
    data = {
        "temperature": temperature,
        "humidity": humidity,
        "sound": sound,
        "battery": battery,
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
        response.raise_for_status()  # Solleva un'eccezione per risposte HTTP errate
        logging.info(f"Dati inviati con successo. Risposta server: {response.status_code} - {response.text}")
        return True
    except requests.exceptions.ConnectionError as e:
        logging.error(f"Errore di connessione: Impossibile connettersi al server a {server_url}.")
        logging.error(f"Dettagli errore: {e}")
    except requests.exceptions.Timeout:
        logging.error(f"Errore di Timeout: La richiesta a {server_url} ha impiegato troppo tempo.")
    except requests.exceptions.RequestException as e:
        logging.error(f"Errore durante l'invio dei dati a {server_url}: {e}")
        if e.response is not None:
            logging.error(f"Risposta del server: {e.response.status_code} - {e.response.text}")
    except Exception as e:
        logging.error(f"Errore imprevisto durante l'invio dei dati: {e}")
    return False

if __name__ == "__main__":
    logging.info(f"Avvio simulatore arnia ({DEVICE_ID}). Invio dati a {server_url} ogni {SEND_INTERVAL_SECONDS} secondi.")
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
            time.sleep(SEND_INTERVAL_SECONDS)  # Attendi prima di riprovare