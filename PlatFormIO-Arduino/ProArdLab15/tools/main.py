# main.py
import flet as ft
import importlib.metadata

# Importar componentes de nuestro MVC
from model import ECGModel
from view import ECGView
from controller import ECGController

def main(page: ft.Page):
    """Función principal que inicializa la aplicación."""
    
    page.title = "ECG Monitor Clínico"
    page.window.min_width = 800
    page.window.min_height = 600

    try:
        version_info = importlib.metadata.version("flet")
    except importlib.metadata.PackageNotFoundError:
        version_info = "desconocida"

    page.snack_bar = ft.SnackBar(
        ft.Text(f"Monitor ECG Clínico v1.1 | Flet {version_info}", color=ft.Colors.WHITE),
        bgcolor=ft.Colors.BLUE_700,
        open=True
    )
    
    # Inicializar componentes MVC
    model = ECGModel()
    view = ECGView(page)
    controller = ECGController(model, view)
    
    page.update()

if __name__ == "__main__":
    ft.app(target=main)