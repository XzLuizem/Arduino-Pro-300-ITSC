# model.py
import serial  # type: ignore
import serial.tools.list_ports  # type: ignore
import threading
import queue
import time

class ECGModel:
    """Clase modelo que maneja los datos y la lógica de negocio"""
    
    def __init__(self):
        self.data_queue = queue.Queue()
        self.port = None
        self.serial_port = None
        self.running = False
        self.bpm = 0
        self.lead_connected = True
        self.data_points = queue.Queue(maxsize=1000)  # Almacenar 4 segundos de datos (1000 pts @ 250Hz)
    
    def start_monitoring(self, port):
        """Inicia la monitorización del puerto serial"""
        try:
            self.port = port
            self.serial_port = serial.Serial(
                port=port,
                baudrate=115200,
                timeout=0.2
            )
            self.running = True
            self.lead_connected = True
            
            # Limpiar buffer inicial
            time.sleep(0.5)
            self.serial_port.reset_input_buffer()
            
            # Iniciar hilo de lectura
            threading.Thread(target=self._serial_reader, daemon=True).start()
            
            return True, f"Conectado a {port}"
        except serial.SerialException as e:
            error_msg = str(e)
            if "Access denied" in error_msg:
                error_msg = "Puerto en uso por otro programa"
            return False, f"Error de conexión: {error_msg}"
        except Exception as e:
            return False, f"Error inesperado: {str(e)}"
    
    def stop_monitoring(self):
        """Detiene la monitorización"""
        self.running = False
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
    
    def _serial_reader(self):
        """Lector de datos serial que procesa el formato unificado CSV."""
        buffer = bytearray()
        
        while self.running:
            try:
                if self.serial_port.in_waiting:
                    buffer.extend(self.serial_port.read(self.serial_port.in_waiting))
                    
                    while b'\n' in buffer:
                        line, buffer = buffer.split(b'\n', 1)
                        line_str = line.decode('utf-8', errors='ignore').strip()
                        
                        if not line_str:
                            continue

                        try:
                            # Procesar el formato unificado: filtered_val,bpm,lead_status
                            parts = line_str.split(',')
                            if len(parts) == 3:
                                filtered_val = float(parts[0])
                                self.bpm = int(parts[1])
                                self.lead_connected = (int(parts[2]) == 1)

                                # Añadir punto de datos para el gráfico
                                if self.data_points.qsize() >= self.data_points.maxsize:
                                    try:
                                        self.data_points.get_nowait()  # Eliminar el más antiguo
                                    except queue.Empty:
                                        pass
                                self.data_points.put_nowait(filtered_val)

                        except (ValueError, IndexError):
                            # Ignorar líneas que no se ajustan al formato
                            continue
                
                time.sleep(0.001) # Dormir muy poco para alta resolución
            except Exception:
                self.running = False
                break
    
    def get_data_points(self):
        """Obtiene los puntos de datos actuales"""
        points = list(self.data_points.queue)
        return points
    
    def get_bpm(self):
        """Obtiene el valor actual de BPM"""
        return self.bpm
    
    def is_lead_connected(self):
        """Verifica si los electrodos están conectados"""
        return self.lead_connected
    
    def is_running(self):
        """Verifica si la monitorización está activa"""
        return self.running
    
    def get_ports(self):
        """Obtiene los puertos seriales disponibles"""
        return [p.device for p in serial.tools.list_ports.comports()]