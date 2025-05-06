from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO
import json
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Initialize Flask app
app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key'
socketio = SocketIO(app, cors_allowed_origins="*")

# Store the latest data
latest_data = {}

@app.route('/')
def index():
    """Render the main page"""
    return render_template('index.html')

@app.route('/api/data', methods=['POST'])
def receive_data():
    """API endpoint to receive data from Arduino"""
    try:
        data = request.get_json()
        logger.info(f"Received data: {data}")
        
        # Update latest data
        global latest_data
        latest_data = data
        
        # Emit the data to all connected clients
        socketio.emit('new_data', data)
        
        return jsonify({"status": "success"}), 200
    except Exception as e:
        logger.error(f"Error processing data: {e}")
        return jsonify({"status": "error", "message": str(e)}), 400

@app.route('/api/data', methods=['GET'])
def get_latest_data():
    """API endpoint to get the latest data"""
    return jsonify(latest_data)

@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    logger.info('Client connected')
    socketio.emit('new_data', latest_data)

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    logger.info('Client disconnected')

if __name__ == '__main__':
    logger.info("Starting IoT Data Server...")
    # Le port est bien défini sur 3000
    socketio.run(app, host='0.0.0.0', port=3000, debug=True, allow_unsafe_werkzeug=True)

