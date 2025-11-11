# controller.py
import asyncio
import time

class ECGController:
    """Clase controlador que maneja la lógica de la aplicación
    
    NOTA: El controlador NO debe conocer detalles de la vista como colores específicos.
    Solo maneja lógica de aplicación y eventos.
    """
    
    def __init__(self, model, view):
        self.model = model
        self.view = view
        
        # Configurar manejadores de eventos
        self.view.set_start_handler(self._on_start_toggle)
        
        # Actualizar lista de puertos
        self._update_ports()
        
        # Iniciar actualizaciones periódicas
        self.update_task = None
        self.bpm_update_interval = 0.2  # Actualizar BPM cada 200ms (más frecuente)
        self.chart_update_interval = 0.02  # Actualizar gráfico cada 20ms (suavizado)
        self.last_bpm_update = 0
        
        # Iniciar bucle de actualización
        self.view.page.run_task(self._update_loop)
    
    def _update_ports(self):
        """Actualiza la lista de puertos disponibles en la vista"""
        ports = self.model.get_ports()
        self.view.update_ports(ports)
        
        if ports:
            self.view.update_status(f"{len(ports)} puerto(s) disponible(s)")
        else:
            self.view.update_status("No se encontraron puertos seriales. Verifica la conexión del ESP32")
    
    def _on_start_toggle(self, e):
        """Manejador para el botón de inicio/detener"""
        if not self.model.is_running():
            # Intentar iniciar con el puerto seleccionado
            port = self.view.port_dropdown.value
            if not port:
                self.view.update_status("⚠️ Seleccione un puerto válido")
                self.view.update_ui()
                return
            
            success, message = self.model.start_monitoring(port)
            if success:
                self.view.update_status(f"✅ Conectado a {port}")
            else:
                self.view.update_status(f"❌ {message}")
            
            # Actualizar estado del botón
            self.view.update_start_button(self.model.is_running())
            self.view.update_ui()
        
        else:
            # Detener monitorización
            self.model.stop_monitoring()
            self._clear_ecg_chart()
            self.view.update_status("⏹️ Monitoreo detenido")
            self.view.update_start_button(self.model.is_running())
            self.view.update_ui()
    
    def _clear_ecg_chart(self):
        """Limpia el gráfico ECG cuando se detiene la monitorización"""
        self.model.data_points.queue.clear()
        self.view.ecg_stack.controls.clear()
        self.view.chart_cache = []
        self.view.last_num_points = 0
        self.view.update_ui()
    
    async def _update_loop(self):
        """Bucle principal de actualización con frecuencias optimizadas"""
        while True:
            current_time = time.time()
            
            try:
                # Actualizar estado de los electrodos
                self.view.update_lead_status(self.model.is_lead_connected())
                
                # Actualizar BPM con mayor frecuencia
                if current_time - self.last_bpm_update >= self.bpm_update_interval:
                    bpm = self.model.get_bpm()
                    if self.model.is_running() and bpm > 0:
                        # Determinar estado clínico
                        if bpm < 60:
                            self.view.update_bpm(bpm, "bradycardia")
                        elif bpm > 100:
                            self.view.update_bpm(bpm, "tachycardia")
                        else:
                            self.view.update_bpm(bpm, "normal")
                    self.last_bpm_update = current_time
                
                # Actualizar gráfico con mayor frecuencia para suavidad
                if current_time - self.last_bpm_update <= self.bpm_update_interval or current_time % 0.1 < 0.05:
                    data_points = self.model.get_data_points()
                    if data_points:
                        self.view.update_ecg_chart(data_points)
                
                # Actualizar UI periódicamente para evitar colas muy largas
                if current_time % 0.05 < 0.02:  # Cada 50ms aproximadamente
                    self.view.update_ui()
            
            except Exception as e:
                # En una aplicación real, querrías registrar este error
                print(f"Error en el bucle de actualización: {e}")
                await asyncio.sleep(1)  # Evitar bucle de errores infinito
            
            # Esperar un breve momento antes de la próxima iteración
            await asyncio.sleep(0.01)  # 10ms para no consumir demasiados recursos