# view.py
import flet as ft
from flet.canvas import Canvas, Path

class ECGView:
    """Clase vista que maneja la interfaz de usuario"""
    
    # Constantes para el gráfico de ECG (estándar médico)
    CHART_WIDTH = 800
    CHART_HEIGHT = 400
    SMALL_GRID_SIZE = 40
    LARGE_GRID_SIZE = 200
    
    # Altura del punto para visibilidad
    POINT_HEIGHT = 1.5
    
    def __init__(self, page: ft.Page):
        self.page = page
        self.page.title = "ECG Monitor Clínico - AD8232 + ESP32"
        self.page.theme_mode = ft.ThemeMode.DARK
        self.page.padding = 20
        self.page.window.width = 1000
        self.page.window.height = 650
        self.page.window.resizable = True
        self.page.window.min_width = 800
        self.page.window.min_height = 600
        
        # Elementos de la UI
        self.chart = None
        self.start_button = None
        self.status_text = None
        self.bpm_text = None
        self.lead_status = None
        self.port_dropdown = None
        
        # Elementos para el gráfico con Canvas
        self.ecg_canvas = None
        self.ecg_path = None
        
        # Inicializar la interfaz
        self._create_ui()
    
    def _create_ui(self):
        """Crea todos los los elementos de la interfaz usando Canvas para el ECG."""
        # Path para la línea de ECG que se dibujará en el Canvas
        self.ecg_path = Path(
            [Path.MoveTo(0, self.CHART_HEIGHT / 2)], # Empezar en el centro
            paint=ft.Paint(
                stroke_width=2,
                style=ft.PaintingStyle.STROKE,
                stroke_cap=ft.StrokeCap.ROUND,
                stroke_join=ft.StrokeJoin.ROUND,
                color=ft.Colors.CYAN_300  # Usar color sólido compatible
            )
        )

        # Canvas donde se dibujará el ECG
        self.ecg_canvas = ft.canvas.Canvas([self.ecg_path])

        # Configuración del gráfico con rejilla médica
        self.chart = ft.Stack(
            height=self.CHART_HEIGHT,
            width=self.CHART_WIDTH,
            controls=[
                self._create_medical_grid(), # Rejilla de fondo
                self.ecg_canvas,             # Canvas para la línea de ECG
            ],
            clip_behavior=ft.ClipBehavior.HARD_EDGE,
        )
        
        # Controles de interfaz (sin cambios)
        self.port_dropdown = ft.Dropdown(
            width=180,
            label="Puerto Serial",
            options=[],
        )
        
        self.start_button = ft.ElevatedButton(
            "Iniciar",
            icon=ft.Icons.PLAY_ARROW,
            bgcolor=ft.Colors.GREEN_700,
            style=ft.ButtonStyle(shape=ft.RoundedRectangleBorder(radius=5)),
            disabled=True
        )
        
        self.status_text = ft.Text(
            "Listo para conexión",
            color=ft.Colors.GREY_400,
            size=12
        )
        
        self.bpm_text = ft.Text(
            "BPM: --", 
            size=22, 
            color=ft.Colors.CYAN_200,
            weight=ft.FontWeight.BOLD
        )
        
        self.lead_status = ft.Text(
            "Estado electrodos: --", 
            color=ft.Colors.GREY_400,
            size=14
        )
        
        # Layout de la página (sin cambios)
        self.page.add(
            ft.Column([
                ft.Row([
                    self.port_dropdown,
                    self.start_button,
                    ft.Container(expand=True),
                    self.status_text,
                ]),
                ft.Container(
                    content=ft.Column([
                        ft.Row([
                            ft.Container(
                                width=40,
                                height=self.CHART_HEIGHT,
                                content=ft.Column([
                                    ft.Text("+1.0 mV", size=10, color=ft.Colors.GREY_300),
                                    ft.Container(expand=True),
                                    ft.Text("0.0 mV", size=10, color=ft.Colors.GREY_300),
                                    ft.Container(expand=True),
                                    ft.Text("-1.0 mV", size=10, color=ft.Colors.GREY_300),
                                ],
                                alignment=ft.MainAxisAlignment.SPACE_BETWEEN)
                            ),
                            self.chart,
                        ]),
                        ft.Row([
                            ft.Container(width=40),
                            ft.Container(
                                width=self.CHART_WIDTH,
                                content=ft.Row(
                                    controls=[
                                        ft.Text("0s", size=10, color=ft.Colors.GREY_300),
                                        ft.Container(expand=True),
                                        ft.Text("2s", size=10, color=ft.Colors.GREY_300),
                                        ft.Container(expand=True),
                                        ft.Text("4s", size=10, color=ft.Colors.GREY_300),
                                        ft.Container(expand=True),
                                        ft.Text("6s", size=10, color=ft.Colors.GREY_300),
                                        ft.Container(expand=True),
                                        ft.Text("8s", size=10, color=ft.Colors.GREY_300),
                                    ],
                                    alignment=ft.MainAxisAlignment.SPACE_BETWEEN
                                )
                            )
                        ])
                    ])
                ),
                ft.Row([
                    ft.Container(
                        content=self.bpm_text,
                        padding=10,
                        border_radius=8,
                        border=ft.border.all(1, ft.Colors.CYAN_700),
                        width=200,
                        alignment=ft.alignment.center
                    ),
                    ft.Container(expand=True),
                    ft.Container(
                        content=self.lead_status,
                        padding=10,
                        width=250,
                    )
                ])
            ])
        )
        
        self.page.add(
            ft.Container(
                content=ft.Text(
                    "Nota: Este dispositivo es para fines educativos. "
                    "La interpretación clínica debe ser realizada por personal médico calificado.",
                    size=12,
                    color=ft.Colors.GREY_500,
                    text_align=ft.TextAlign.CENTER
                ),
                padding=ft.padding.only(top=10),
                border=ft.border.only(top=ft.BorderSide(1, ft.Colors.GREY_800)),
            )
        )
    
    def _create_medical_grid(self):
        """Crea la rejilla médica estándar para ECG"""
        grid_controls = ft.Stack()
        lines = []
        
        # Líneas verticales (tiempo)
        for x in range(0, self.CHART_WIDTH + 1, self.SMALL_GRID_SIZE):
            is_large = (x % self.LARGE_GRID_SIZE == 0)
            line_color = ft.Colors.GREY_700 if is_large else ft.Colors.GREY_800
            
            # Líneas verticales
            lines.append(
                ft.Container(
                    left=x,
                    width=0.5 if is_large else 0.2,
                    height=self.CHART_HEIGHT,
                    bgcolor=line_color,
                )
            )
        
        # Líneas horizontales (amplitud)
        for y in range(0, self.CHART_HEIGHT + 1, self.SMALL_GRID_SIZE):
            is_large = (y % self.LARGE_GRID_SIZE == 0)
            line_color = ft.Colors.GREY_700 if is_large else ft.Colors.GREY_800
            
            # Líneas horizontales
            lines.append(
                ft.Container(
                    top=y,
                    width=self.CHART_WIDTH,
                    height=0.5 if is_large else 0.2,
                    bgcolor=line_color,
                )
            )
        
        # Añadir fondo con opacidad para mejor visibilidad
        grid_controls.controls.append(
            ft.Container(
                bgcolor=ft.Colors.with_opacity(0.03, ft.Colors.WHITE),
                width=self.CHART_WIDTH,
                height=self.CHART_HEIGHT,
            )
        )
        
        # Añadir todas las líneas
        grid_controls.controls.extend(lines)
        return grid_controls
    
    def update_ports(self, ports):
        """Actualiza la lista de puertos disponibles"""
        self.port_dropdown.options = [
            ft.dropdown.Option(key=port, text=f"{port} - Puerto Serial") 
            for port in ports
        ]
        self.port_dropdown.disabled = len(ports) == 0
        self.start_button.disabled = len(ports) == 0
        self.page.update()
    
    def update_start_button(self, is_running):
        """Actualiza el estado del botón de inicio/detener"""
        if is_running:
            self.start_button.text = "Detener"
            self.start_button.icon = ft.Icons.STOP
            self.start_button.bgcolor = ft.Colors.RED_700
        else:
            self.start_button.text = "Iniciar"
            self.start_button.icon = ft.Icons.PLAY_ARROW
            self.start_button.bgcolor = ft.Colors.GREEN_700
        self.page.update()
    
    def update_status(self, message):
        """Actualiza el texto de estado y el color según el tipo de mensaje"""
        self.status_text.value = message
        
        # Determinar color según el mensaje
        if "✅" in message or "Conectado" in message:
            self.status_text.color = ft.Colors.GREEN_400
        elif "⚠️" in message or "Seleccione" in message:
            self.status_text.color = ft.Colors.AMBER
        elif "❌" in message or "Error" in message:
            self.status_text.color = ft.Colors.RED_400
        elif "⏹️" in message or "detenido" in message:
            self.status_text.color = ft.Colors.GREY_400
        else:
            self.status_text.color = ft.Colors.GREY_400
        self.page.update()
    
    def update_bpm(self, bpm, status="normal"):
        """Actualiza el valor de BPM y su color según estado clínico"""
        self.bpm_text.value = f"BPM: {bpm}"
        
        # Colorear según valores clínicos (la vista decide los colores)
        if status == "bradycardia" or bpm < 60:
            self.bpm_text.color = ft.Colors.YELLOW_200  # Bradicardia
        elif status == "tachycardia" or bpm > 100:
            self.bpm_text.color = ft.Colors.RED_200  # Taquicardia
        else:
            self.bpm_text.color = ft.Colors.CYAN_200  # Normal
        self.page.update()
    
    def update_lead_status(self, is_connected):
        """Actualiza el estado de los electrodos"""
        if is_connected:
            self.lead_status.value = "✅ Electrodos conectados correctamente"
            self.lead_status.color = ft.Colors.CYAN_200
        else:
            self.lead_status.value = "⚠️ Electrodos desconectados o mal colocados"
            self.lead_status.color = ft.Colors.YELLOW_200
        self.page.update()
    
    def update_ecg_chart(self, data_points):
        """Actualiza el gráfico de ECG dibujando una línea continua en el Canvas."""
        if not data_points or len(data_points) < 2:
            self._clear_chart()
            return

        num_points = len(data_points)
        chart_height = self.CHART_HEIGHT
        chart_width = self.CHART_WIDTH
        point_spacing = chart_width / (num_points - 1)

        # Normalizar valores para el gráfico
        min_val = min(data_points)
        max_val = max(data_points)
        range_val = max_val - min_val or 1

        # Crear los elementos del path (línea)
        path_elements = []
        for i, val in enumerate(data_points):
            x = i * point_spacing
            
            # Normalizar y mapear Y al centro del gráfico
            normalized_y = (val - min_val) / range_val
            y = chart_height - (normalized_y * chart_height * 0.8 + chart_height * 0.1)

            if i == 0:
                path_elements.append(ft.canvas.Path.MoveTo(x, y))
            else:
                path_elements.append(ft.canvas.Path.LineTo(x, y))
        
        # Actualizar el path y el canvas
        self.ecg_path.elements = path_elements
        self.ecg_canvas.update()

    def _clear_chart(self):
        """Limpia el Canvas de ECG."""
        self.ecg_path.elements = [ft.canvas.Path.MoveTo(0, self.CHART_HEIGHT / 2)]
        if self.ecg_canvas:
            self.ecg_canvas.update()
    
    def set_start_handler(self, handler):
        """Establece el manejador para el evento de inicio/detener"""
        self.start_button.on_click = handler
    
    def update_ui(self):
        """Actualiza la interfaz de usuario"""
        self.page.update()