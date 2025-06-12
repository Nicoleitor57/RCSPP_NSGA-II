# Makefile para ejecutar el script de Python NSGA-II

# Intérprete de Python a usar
PYTHON = python3

# El script principal que contiene todo el código Python
SCRIPT_PRINCIPAL = ejecutable_nsga2.py

# El nombre del ejecutable que crearemos para cumplir con ./nsga-II
EJECUTABLE = nsga-II

# Regla por defecto: se ejecuta con 'make' o 'make all'
all: $(EJECUTABLE)

# Crea un enlace simbólico al script y le da permisos de ejecución
$(EJECUTABLE): $(SCRIPT_PRINCIPAL)
	@echo "#!/usr/bin/env $(PYTHON)" > $(EJECUTABLE)
	@cat $(SCRIPT_PRINCIPAL) >> $(EJECUTABLE)
	@chmod +x $(EJECUTABLE)
	@echo "✅ 'Ejecutable' ./$(EJECUTABLE) creado/actualizado."

# Regla para limpiar los archivos generados
clean:
	@rm -f $(EJECUTABLE)
	@rm -rf __pycache__
	@echo "🧹 Archivos generados eliminados."