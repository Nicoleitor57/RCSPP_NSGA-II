
import re # Usaremos expresiones regulares para facilitar el parseo
import re
import heapq

class ProblemaRCSPP:
    def __init__(self, archivo_instancia):
        self.nodos = set()
        self.arcos_set = set()
        self.source = ''
        self.sink = ''
        self.R_max = 0.0
        self.nodos_prohibidos = set()
        self.nodos_requeridos = set()
        self.alpha = 0.0

        # Diccionarios para los datos de los arcos y nodos
        self.dist = {}
        self.time = {}
        self.resource = {}
        self.epsilon = {}
        self.risk_node = {}
        self.earliest = {}
        self.latest = {}

        self.cargar_instancia(archivo_instancia)
        print("✅ Instancia cargada y parseada correctamente.")

    def cargar_instancia(self, archivo):
        """Parser de estado simple para leer el formato de datos tipo AMPL."""
        with open(archivo, 'r') as f:
            contenido = f.read()

        # Usar expresiones regulares para encontrar cada bloque param/set
        self.nodos = set(re.search(r'set NODES :=(.*?);', contenido, re.DOTALL).group(1).split())
        self.source = re.search(r'param source := (.*?);', contenido).group(1)
        self.sink = re.search(r'param sink := (.*?);', contenido).group(1)
        self.R_max = float(re.search(r'param R_max := (.*?);', contenido).group(1))
        
        # Sets opcionales
        forbidden_match = re.search(r'set FORBIDDEN_NODES :=(.*?);', contenido, re.DOTALL)
        if forbidden_match:
            self.nodos_prohibidos = set(forbidden_match.group(1).split())
            
        required_match = re.search(r'set REQUIRED_NODES :=(.*?);', contenido, re.DOTALL)
        if required_match:
            self.nodos_requeridos = set(required_match.group(1).split())

        # Parseo de datos tabulares (arcos y sus parámetros)
        def parse_tabular_data(param_name, target_dict):
            # Busca un bloque como 'param dist := [*,*] n1 n2 5 ... ;'
            pattern = re.compile(rf'param {param_name} :=.*?\[\*,\*\](.*?;)', re.DOTALL)
            data_block = pattern.search(contenido).group(1)
            lines = data_block.strip().split('\n')
            for line in lines:
                parts = line.strip().split()
                if len(parts) == 3:
                    u, v, val = parts
                    target_dict[(u, v)] = float(val)

        parse_tabular_data('dist', self.dist)
        parse_tabular_data('time_', self.time)
        parse_tabular_data('resource', self.resource)
        parse_tabular_data('epsilon', self.epsilon)

        # Parseo de datos de nodos
        def parse_node_data(param_name, target_dict):
            pattern = re.compile(rf'param {param_name} :=(.*?);', re.DOTALL)
            data_block = pattern.search(contenido).group(1)
            lines = data_block.strip().split('\n')
            for line in lines:
                parts = line.strip().split()
                if len(parts) == 2:
                    node, val = parts
                    target_dict[node] = float(val)

        parse_node_data('risk_node', self.risk_node)
        parse_node_data('earliest', self.earliest)
        parse_node_data('latest', self.latest)
        
        # El set de arcos válidos lo derivamos de los datos de distancia
        self.arcos_set = set(self.dist.keys())

        self.adj = {node: [] for node in self.nodos}
        for u, v in self.arcos_set:
            self.adj[u].append(v)

def nsga2(problema, tam_poblacion, num_generaciones, paciencia=50):
    """Función principal que ejecuta el algoritmo NSGA-II."""

    # --- Funciones anidadas (ayudantes) del algoritmo ---

    def inicializar_poblacion():
        """Genera la población inicial creando caminos que pasan por los nodos requeridos."""
        poblacion = []
        intentos_max_por_individuo = 20

        print(f"Generando población inicial de tamaño {tam_poblacion}...")

        while len(poblacion) < tam_poblacion:
            # Barajamos los nodos requeridos para generar diversidad
            nodos_requeridos_barajados = random.sample(list(problema.nodos_requeridos), len(problema.nodos_requeridos))
            puntos_de_paso = [problema.source] + nodos_requeridos_barajados + [problema.sink]

            camino_completo = [problema.source]
            nodos_usados = {problema.source}
            construccion_exitosa = True

            for i in range(len(puntos_de_paso) - 1):
                origen_sub = puntos_de_paso[i]
                destino_sub = puntos_de_paso[i+1]
                
                # Nodos a evitar: los prohibidos globalmente + los ya usados en este camino (sin incluir el origen_sub)
                nodos_a_evitar = problema.nodos_prohibidos.union(nodos_usados - {origen_sub})
                
                sub_camino = encontrar_subcamino_bfs(origen_sub, destino_sub, problema.adj, nodos_a_evitar)

                if sub_camino is None:
                    construccion_exitosa = False
                    break

                # Añadir el sub-camino (sin el primer nodo que ya está)
                for nodo in sub_camino[1:]:
                    camino_completo.append(nodo)
                    nodos_usados.add(nodo)
            
            if construccion_exitosa:
                individuo = Individuo(camino=camino_completo)
                poblacion.append(individuo)
                if len(poblacion) % 10 == 0:
                     print(f"  ... {len(poblacion)} individuos generados.")

        print("✅ Población inicial generada.")
        return poblacion

    # Reemplaza la función completa DENTRO de nsga2

    # def inicializar_poblacion():
    #     """
    #     Genera la población inicial creando "caminos aleatorios" desde el origen al destino.
    #     ADVERTENCIA: Este método es ineficiente y es muy improbable que cumpla
    #     con restricciones como los nodos requeridos.
    #     """
    #     poblacion = []
    #     print("Generando población inicial con CAMINOS ALEATORIOS (Modo Experimental)...")
        
    #     # Límite para evitar que un solo camino se genere infinitamente
    #     max_intentos_por_individuo = 1000 
    #     intentos_totales = 0

    #     while len(poblacion) < tam_poblacion:
    #         intentos_totales += 1
    #         if intentos_totales > max_intentos_por_individuo * tam_poblacion:
    #             print("❌ Límite máximo de intentos alcanzado. No se pudo generar la población completa.")
    #             break

    #         # Iniciar un nuevo camino aleatorio
    #         camino_actual = [problema.source]
    #         nodos_vistos = {problema.source}
    #         construccion_exitosa = False

    #         for _ in range(len(problema.nodos) * 2): # Límite de longitud para evitar caminos infinitos
    #             ultimo_nodo = camino_actual[-1]

    #             # Condición de éxito
    #             if ultimo_nodo == problema.sink:
    #                 construccion_exitosa = True
    #                 break
                
    #             # Encontrar vecinos válidos
    #             vecinos_posibles = problema.adj.get(ultimo_nodo, [])
    #             vecinos_validos = [
    #                 v for v in vecinos_posibles 
    #                 if v not in nodos_vistos and v not in problema.nodos_prohibidos
    #             ]

    #             if not vecinos_validos:
    #                 # Callejón sin salida, este camino falla
    #                 break
                
    #             # Elegir el siguiente paso al azar
    #             siguiente_nodo = random.choice(vecinos_validos)
    #             camino_actual.append(siguiente_nodo)
    #             nodos_vistos.add(siguiente_nodo)

    #         if construccion_exitosa:
    #             individuo = Individuo(camino=camino_actual)
    #             poblacion.append(individuo)
    #             if len(poblacion) % 10 == 0:
    #                 print(f"  ... {len(poblacion)} individuos generados.")
        
    #     if not poblacion:
    #         print("❌ ADVERTENCIA: La generación aleatoria no produjo ni un solo camino válido del origen al destino.")

    #     print(f"✅ Población inicial de {len(poblacion)} individuos generada.")
    #     return poblacion

    def encontrar_subcamino_bfs(origen, destino, adj, nodos_a_evitar):
        """Encuentra un camino entre origen y destino usando BFS, evitando ciertos nodos."""
        if origen == destino:
            return [origen]
        
        cola = [(origen, [origen])] # (nodo_actual, camino_hasta_aqui)
        visitados = {origen}

        while cola:
            nodo_actual, camino_actual = cola.pop(0)

            if nodo_actual not in adj: continue

            for vecino in adj[nodo_actual]:
                if vecino == destino:
                    return camino_actual + [destino]
                
                if vecino not in visitados and vecino not in nodos_a_evitar:
                    visitados.add(vecino)
                    nuevo_camino = camino_actual + [vecino]
                    cola.append((vecino, nuevo_camino))
        
        return None # No se encontró camino

    # Estas funciones reemplazan a los placeholders DENTRO de la función nsga2

    def seleccion(poblacion):
        """Selección por torneo binario basado en rango y distancia de apiñamiento."""
        p1 = random.choice(poblacion)
        p2 = random.choice(poblacion)
        
        # Gana el de mejor rango (menor)
        if p1.rango_dominancia < p2.rango_dominancia:
            return p1
        elif p2.rango_dominancia < p1.rango_dominancia:
            return p2
        
        # Si el rango es igual, gana el de mayor distancia de apiñamiento
        if p1.distancia_apiniamiento > p2.distancia_apiniamiento:
            return p1
        else:
            return p2

    # def cruce(padre1, padre2):
    #     """
    #     Cruce de un punto basado en un nodo común.
    #     Devuelve un solo hijo. Puede necesitar reparación.
    #     """
    #     nodos_comunes = list(set(padre1.camino[1:-1]).intersection(set(padre2.camino[1:-1])))
    #     if not nodos_comunes:
    #         return Individuo(camino=padre1.camino[:]) # Si no hay cruce, clona al padre 1

    #     punto_cruce = random.choice(nodos_comunes)
    #     idx1 = padre1.camino.index(punto_cruce)
    #     idx2 = padre2.camino.index(punto_cruce)

    #     camino_hijo = padre1.camino[:idx1] + padre2.camino[idx2:]
        
    #     # Simple reparación de ciclos
    #     camino_hijo_sin_ciclos = []
    #     nodos_vistos = set()
    #     for nodo in camino_hijo:
    #         if nodo not in nodos_vistos:
    #             camino_hijo_sin_ciclos.append(nodo)
    #             nodos_vistos.add(nodo)
        
    #     # TODO: Implementar una reparación más robusta que re-inserte nodos requeridos si se pierden.
        
    #     return Individuo(camino=camino_hijo_sin_ciclos)

    # Reemplaza tu función cruce actual DENTRO de nsga2 con esta

    # def cruce(padre1, padre2):
    #     """
    #     Implementación del Edge Recombination Crossover (ERX).
    #     Crea un hijo que hereda la mayor cantidad de aristas posibles de los padres.
    #     """
    #     # --- Paso 1: Construir el Mapa de Aristas ---
    #     mapa_aristas = {}
    #     # Usamos un set para obtener todos los nodos únicos de ambos padres
    #     nodos_totales = set(padre1.camino) | set(padre2.camino)

    #     for nodo in nodos_totales:
    #         mapa_aristas[nodo] = set()

    #     # Función auxiliar para poblar el mapa
    #     def agregar_aristas_al_mapa(camino):
    #         for i, nodo in enumerate(camino):
    #             # Agregar el vecino anterior si no es el primer nodo
    #             if i > 0:
    #                 mapa_aristas[nodo].add(camino[i-1])
    #             # Agregar el vecino siguiente si no es el último nodo
    #             if i < len(camino) - 1:
    #                 mapa_aristas[nodo].add(camino[i+1])

    #     agregar_aristas_al_mapa(padre1.camino)
    #     agregar_aristas_al_mapa(padre2.camino)

    #     # --- Paso 2: Construir el Camino del Hijo ---
    #     hijo_camino = []
    #     # Empezamos con el nodo inicial del primer padre
    #     nodo_actual = padre1.camino[0]
    #     nodos_no_visitados = nodos_totales - {nodo_actual}
    #     hijo_camino.append(nodo_actual)

    #     while len(hijo_camino) < len(nodos_totales):
    #         # Remover el nodo actual de todas las listas de vecinos del mapa
    #         for nodo in mapa_aristas:
    #             mapa_aristas[nodo].discard(nodo_actual)

    #         vecinos_de_actual = mapa_aristas.get(nodo_actual)
    #         siguiente_nodo = None

    #         if vecinos_de_actual and len(vecinos_de_actual) > 0:
    #             # --- Regla de Selección de ERX ---
    #             # Elegir el vecino que tenga la lista de vecinos más corta
    #             min_len = float('inf')
    #             for vecino in sorted(list(vecinos_de_actual)): # sorted para consistencia
    #                 longitud_vecinos = len(mapa_aristas.get(vecino, []))
    #                 if longitud_vecinos < min_len:
    #                     min_len = longitud_vecinos
    #                     siguiente_nodo = vecino
    #                 # Desempate: si tienen la misma longitud, se elige uno al azar
    #                 elif longitud_vecinos == min_len:
    #                     if random.choice([True, False]):
    #                         siguiente_nodo = vecino
    #         else:
    #             # --- Manejo de Callejón sin Salida ---
    #             # Si el nodo actual no tiene más vecinos, elegimos uno al azar de los no visitados
    #             if nodos_no_visitados:
    #                 siguiente_nodo = random.choice(list(nodos_no_visitados))

    #         if siguiente_nodo is None:
    #             # No hay más nodos que agregar, se termina la construcción
    #             break
            
    #         nodo_actual = siguiente_nodo
    #         hijo_camino.append(nodo_actual)
    #         nodos_no_visitados.discard(nodo_actual)
        
    #     # El hijo generado puede no ser una solución completa (origen->destino) o
    #     # puede haber perdido nodos requeridos. La función de evaluación lo penalizará.
    #     return Individuo(camino=hijo_camino)

    # Añade esta nueva función DENTRO de nsga2, junto a la otra función 'cruce'

    def cruce_heuristico(padre1, problema, cost_func):
        """
        Crea un hijo preservando la "cabeza" de un padre y completando
        el resto del camino de forma voraz (greedy) hasta el sink.
        """
        # Si el camino del padre es muy corto, no se puede hacer mucho
        if len(padre1.camino) <= 2:
            return Individuo(camino=padre1.camino[:]) # Devuelve un clon

        # 1. Copiar una "cabeza" de longitud aleatoria del padre
        # El punto de corte será entre el primer nodo y el penúltimo
        punto_corte = random.randint(1, len(padre1.camino) - 1)
        cabeza = padre1.camino[:punto_corte]
        
        hijo_camino = list(cabeza)
        nodos_usados = set(hijo_camino)
        nodo_actual = hijo_camino[-1]

        # 2. Completar el resto del camino de forma voraz
        # Límite de seguridad para evitar bucles infinitos en casos extraños
        for _ in range(len(problema.nodos)):
            if nodo_actual == problema.sink:
                break # Hemos llegado al destino

            vecinos_posibles = problema.adj.get(nodo_actual, [])
            vecinos_validos = [
                v for v in vecinos_posibles
                if v not in nodos_usados and v not in problema.nodos_prohibidos
            ]

            if not vecinos_validos:
                break # Callejón sin salida, el camino termina aquí

            # --- Decisión Voraz (Greedy) ---
            # Evaluar cada vecino y elegir el de menor costo
            mejor_siguiente_nodo = None
            menor_costo = float('inf')

            for vecino in vecinos_validos:
                costo_arco = cost_func(nodo_actual, vecino)
                if costo_arco < menor_costo:
                    menor_costo = costo_arco
                    mejor_siguiente_nodo = vecino
            
            if mejor_siguiente_nodo is None:
                break # No se pudo elegir un siguiente nodo

            # Moverse al mejor nodo encontrado
            nodo_actual = mejor_siguiente_nodo
            hijo_camino.append(nodo_actual)
            nodos_usados.add(nodo_actual)
        
        return Individuo(camino=hijo_camino)


    def mutacion(individuo):
        """
        Mutación por re-enrutamiento de un sub-camino.
        """
        camino = individuo.camino[:]
        if len(camino) < 4:
            return Individuo(camino=camino) # No se puede mutar un camino muy corto

        # Elige dos puntos de corte (excluyendo origen y destino)
        idx1 = random.randint(1, len(camino) - 3)
        idx2 = random.randint(idx1 + 1, len(camino) - 2)

        origen_sub = camino[idx1-1]
        destino_sub = camino[idx2+1]
        
        nodos_a_evitar = problema.nodos_prohibidos.union(set(camino[:idx1] + camino[idx2+2:]))
        
        sub_camino = encontrar_subcamino_bfs(origen_sub, destino_sub, problema.adj, nodos_a_evitar)

        if sub_camino and len(sub_camino) > 1:
            # Reconstruye el camino
            camino_mutado = camino[:idx1-1] + sub_camino + camino[idx2+2:]
            return Individuo(camino=camino_mutado)
        else:
            return Individuo(camino=camino) # Si la mutación falla, devuelve el original

    def non_dominated_sort(poblacion):
        frentes = []
        dominados_por = {id(ind): set() for ind in poblacion}
        conteo_dominacion = {id(ind): 0 for ind in poblacion}
        
        for i, p in enumerate(poblacion):
            for j, q in enumerate(poblacion):
                if i == j: continue
                
                # Chequear si p domina a q
                if all(p.objetivos[k] <= q.objetivos[k] for k in range(2)) and any(p.objetivos[k] < q.objetivos[k] for k in range(2)):
                    dominados_por[id(p)].add(id(q))
                # Chequear si q domina a p
                elif all(q.objetivos[k] <= p.objetivos[k] for k in range(2)) and any(q.objetivos[k] < p.objetivos[k] for k in range(2)):
                    conteo_dominacion[id(p)] += 1
        
        frente_actual = []
        for ind in poblacion:
            if conteo_dominacion[id(ind)] == 0:
                ind.rango_dominancia = 1
                frente_actual.append(ind)
        
        rango_frente = 1
        while frente_actual:
            frentes.append(frente_actual)
            siguiente_frente = []
            for p in frente_actual:
                for id_q in dominados_por[id(p)]:
                    # Encontrar el individuo q correspondiente al id
                    q = next(ind for ind in poblacion if id(ind) == id_q)
                    conteo_dominacion[id(q)] -= 1
                    if conteo_dominacion[id(q)] == 0:
                        q.rango_dominancia = rango_frente + 1
                        siguiente_frente.append(q)
            frente_actual = siguiente_frente
            rango_frente += 1
            
        return frentes


    def crowding_distance_assignment(frente):
        if not frente: return
        
        num_objetivos = 2
        tam_frente = len(frente)
        
        for ind in frente:
            ind.distancia_apiniamiento = 0.0

        for m in range(num_objetivos):
            frente.sort(key=lambda x: x.objetivos[m])
            
            # Asignar distancia infinita a las soluciones en los bordes
            frente[0].distancia_apiniamiento = float('inf')
            frente[-1].distancia_apiniamiento = float('inf')
            
            obj_min = frente[0].objetivos[m]
            obj_max = frente[-1].objetivos[m]
            
            if obj_max == obj_min: continue

            for i in range(1, tam_frente - 1):
                frente[i].distancia_apiniamiento += (frente[i+1].objetivos[m] - frente[i-1].objetivos[m]) / (obj_max - obj_min)

    # --- Ciclo principal del algoritmo ---
    
    # El cuerpo de la función nsga2 comienza aquí
    
    poblacion_actual = inicializar_poblacion()
    if not poblacion_actual:
        print("Error: La población inicial está vacía. Terminando ejecución.")
        return [] 

    for ind in poblacion_actual:
        ind.evaluar(problema)

   
    # Inicializar variables para el criterio de parada por convergencia
    stall_counter = 0
    mejor_frente_global = []
    mejor_huella = (-1, float('inf'), float('inf')) 
    # Huella = (num_soluciones_factibles, suma_obj1, suma_obj2)
   

    # Búsca el inicio de tu bucle 'for' y reemplázalo con todo este bloque

    for gen in range(num_generaciones):
        # --- 1. Creación de la siguiente generación (esta parte no cambia) ---
        poblacion_hijos = []
        while len(poblacion_hijos) < tam_poblacion:
            padre1 = seleccion(poblacion_actual)
            padre2 = seleccion(poblacion_actual)
            if not padre1 or not padre2: continue
            hijo = cruce_heuristico(padre1, padre2)
            mutacion(hijo)
            hijo.evaluar(problema)
            poblacion_hijos.append(hijo)
        
        poblacion_combinada = poblacion_actual + poblacion_hijos
        frentes = non_dominated_sort(poblacion_combinada)
        
        poblacion_siguiente = []
        if frentes:
            for frente in frentes:
                if len(poblacion_siguiente) + len(frente) <= tam_poblacion:
                    poblacion_siguiente.extend(frente)
                else:
                    crowding_distance_assignment(frente)
                    frente.sort(key=lambda x: x.distancia_apiniamiento, reverse=True)
                    necesarios = tam_poblacion - len(poblacion_siguiente)
                    poblacion_siguiente.extend(frente[:necesarios])
                    break
        poblacion_actual = poblacion_siguiente

        # --- 2. Lógica de chequeo de convergencia (ESTA ES LA PARTE CORREGIDA Y REORDENADA) ---
        
        frentes_actuales = non_dominated_sort(poblacion_actual)
        mejor_frente_actual = frentes_actuales[0] if frentes_actuales else []
        frente_factible_actual = [ind for ind in mejor_frente_actual if ind.violacion_total == 0]

        # Primero, actualizamos el contador
        mejoro_en_esta_generacion = False
        if frente_factible_actual:
            num_sol = len(frente_factible_actual)
            suma_obj1 = sum(ind.objetivos[0] for ind in frente_factible_actual)
            suma_obj2 = sum(ind.objetivos[1] for ind in frente_factible_actual)
            huella_actual = (num_sol, suma_obj1, suma_obj2)

            if huella_actual[0] > mejor_huella[0] or \
               (huella_actual[0] == mejor_huella[0] and (huella_actual[1] + huella_actual[2] < mejor_huella[1] + mejor_huella[2])):
                mejor_huella = huella_actual
                mejor_frente_global = frente_factible_actual
                stall_counter = 0
                mejoro_en_esta_generacion = True
            else:
                stall_counter += 1
        else:
            stall_counter += 1
            
        # --- 3. Impresión y decisión de parada (Lógica más clara) ---
        
        # AHORA, comprobamos si debemos parar ANTES de imprimir el estado
        if stall_counter >= paciencia:
            # Si se cumple el criterio, imprimir el mensaje final y SALIR
            print(f"\n🛑 DETENCIÓN ANTICIPADA en la generación {gen+1}: Algoritmo estancado por {paciencia} generaciones.")
            break  # <-- ESTA ES LA INSTRUCCIÓN QUE DETIENE EL BUCLE
        else:
            # Si no nos detenemos, imprimimos el progreso normal de la generación
            print(f"Generación {gen+1}/{num_generaciones} | ", end="")
            if mejoro_en_esta_generacion:
                 print(f"✨ ¡Nuevo mejor frente! ({mejor_huella[0]} sols). Reiniciando contador.")
            else:
                 print(f"No hubo mejora. Estancado: {stall_counter}/{paciencia}")
       
    # Modificamos el return para que devuelva el mejor frente global encontrado,
    # en lugar del de la última generación, que podría ser peor.
    print("\nEjecución finalizada. Devolviendo el mejor frente de Pareto encontrado.")
    if not mejor_frente_global:
        # Fallback: si nunca se encontró un frente factible, devuelve el mejor de la última población
        frentes_finales = non_dominated_sort(poblacion_actual)
        return frentes_finales[0] if frentes_finales else []
    else:
        return mejor_frente_global
    # >>>>>>>>>>>>>>> FIN DE CÓDIGO MODIFICADO (3/3) >>>>>>>>>>>>>>>
    import heapq # Asegúrate de tener 'import heapq' al principio de tu archivo.

# Pon esta función DENTRO de la función nsga2

def dijkstra(origen, destino, problema, nodos_a_evitar, cost_function):
    """
    Encuentra el camino de menor costo desde un origen a un destino usando Dijkstra.
    El costo es definido por la 'cost_function'.
    """
    cola_prioridad = [(0, origen, [origen])] # (costo_acumulado, nodo_actual, camino_hasta_aqui)
    costos_minimos = {origen: 0}

    while cola_prioridad:
        costo_actual, nodo_actual, camino = heapq.heappop(cola_prioridad)

        if nodo_actual == destino:
            return camino # Hemos encontrado el camino de menor costo

        if costo_actual > costos_minimos.get(nodo_actual, float('inf')):
            continue

        for vecino in problema.adj.get(nodo_actual, []):
            if vecino in nodos_a_evitar:
                continue
            
            # Calcula el costo de este arco específico usando la función proporcionada
            costo_arco = cost_function(nodo_actual, vecino)
            nuevo_costo = costo_actual + costo_arco

            if nuevo_costo < costos_minimos.get(vecino, float('inf')):
                costos_minimos[vecino] = nuevo_costo
                nuevo_camino = camino + [vecino]
                heapq.heappush(cola_prioridad, (nuevo_costo, vecino, nuevo_camino))
    
    return None # No se encontró camino

# ==============================================================================
# ▼▼▼▼▼ REEMPLAZA TU CLASE INDIVIDUO ACTUAL CON ESTO ▼▼▼▼▼
# ==============================================================================

# ==============================================================================
# ▼▼▼▼▼ REEMPLAZA TU CLASE INDIVIDUO ACTUAL CON ESTA VERSIÓN MEJORADA ▼▼▼▼▼
# ==============================================================================

class Individuo:
    """Representa una solución (un camino) en la población. Ahora con seguimiento detallado de violaciones."""
    def __init__(self, camino=None):
        self.camino = camino if camino is not None else []
        self.objetivos = [float('inf'), float('inf')]
        self.rango_dominancia = -1
        self.distancia_apiniamiento = 0.0
        
        # Diccionario para seguimiento detallado de violaciones
        self.violaciones = {
            'camino_invalido': 0, # No empieza/termina en source/sink
            'ciclos': 0,
            'recursos': 0.0, # Exceso de recursos consumidos
            'nodos_prohibidos': 0, # Cantidad de nodos prohibidos visitados
            'nodos_requeridos': 0, # Cantidad de nodos requeridos que faltan
            'ventanas_tiempo': 0.0 # Suma de desviaciones de tiempo
        }
        self.violacion_total = 0.0

    def __repr__(self):
        return f"Individuo(Camino: {'->'.join(map(str, self.camino[:2]))}...{'->'.join(map(str, self.camino[-2:]))})"

    def evaluar(self, problema):
        # Reiniciar violaciones para la re-evaluación
        for k in self.violaciones: self.violaciones[k] = 0.0

        # --- Chequeo 1: Validez estructural del camino (origen/destino) ---
        if not self.camino or self.camino[0] != problema.source or self.camino[-1] != problema.sink:
            self.violaciones['camino_invalido'] = 1
        
        # --- Chequeo 2: Ciclos ---
        if len(self.camino) != len(set(self.camino)):
            self.violaciones['ciclos'] = 1
        
        # Si el camino es estructuralmente inválido, no podemos calcular el resto
        if self.violaciones['camino_invalido'] or self.violaciones['ciclos']:
            self.violacion_total = sum(self.violaciones.values())
            self.objetivos = [float('inf'), float('inf')]
            return

        # --- Cálculos acumulativos ---
        dist_total, tiempo_esperado_total, recurso_total, tiempo_llegada_actual = 0.0, 0.0, 0.0, 0.0

        # --- Chequeo 3: Ventanas de tiempo ---
        for i in range(len(self.camino) - 1):
            u, v = self.camino[i], self.camino[i+1]
            
            # Chequeo de conectividad
            if (u, v) not in problema.arcos_set:
                self.violaciones['camino_invalido'] = 2 # Código para "camino roto"
                break

            tiempo_base = problema.time.get((u,v), 0)
            p_incidente = problema.risk_node.get(u, 0) + problema.risk_node.get(v, 0)
            tiempo_esperado_arco = tiempo_base * (1 + 0.5 * p_incidente)
            
            tiempo_llegada_actual += tiempo_esperado_arco
            
            earliest_v = problema.earliest.get(v, 0)
            latest_v = problema.latest.get(v, 9999)
            self.violaciones['ventanas_tiempo'] += max(0, earliest_v - tiempo_llegada_actual)
            self.violaciones['ventanas_tiempo'] += max(0, tiempo_llegada_actual - latest_v)
            
            # Acumular objetivos y recursos
            dist_total += problema.dist.get((u, v), 0)
            tiempo_esperado_total += tiempo_esperado_arco
            recurso_total += problema.resource.get((u, v), 0)

        # --- Chequeos a nivel de camino completo ---
        nodos_camino_set = set(self.camino)
        # --- Chequeo 4: Recursos ---
        self.violaciones['recursos'] = max(0, recurso_total - problema.R_max)
        # --- Chequeo 5: Nodos Prohibidos ---
        self.violaciones['nodos_prohibidos'] = len(problema.nodos_prohibidos.intersection(nodos_camino_set))
        # --- Chequeo 6: Nodos Requeridos ---
        self.violaciones['nodos_requeridos'] = len(problema.nodos_requeridos - nodos_camino_set)
        
        # Calcular la violación total y aplicar penalización
        self.violacion_total = sum(self.violaciones.values())
        if self.violacion_total > 0:
            penalizacion = 1e11
            self.objetivos = [dist_total + penalizacion, tiempo_esperado_total + penalizacion]
        else:
            self.objetivos = [dist_total, tiempo_esperado_total]

import csv # Asegúrate de que 'import csv' esté al principio de tu archivo

# ... (después de la clase Individuo) ...

# Reemplaza esta función en tu archivo

def escribir_resultados_a_csv(frente_pareto, nombre_archivo):
    """Guarda las soluciones del frente de Pareto en un archivo CSV, incluyendo un desglose de las violaciones de restricciones."""
    print(f"\nEscribiendo resultados detallados en: {nombre_archivo} ...")
    if frente_pareto is None:
        print("❌ No hay frente de Pareto para guardar (es None).")
        return

    try:
        with open(nombre_archivo, 'w', newline='') as f:
            writer = csv.writer(f)
            # Nueva cabecera con detalles de violaciones
            writer.writerow([
                "ID_Solucion", 
                "Objetivo_Distancia", "Objetivo_Tiempo_Esperado", "Es_Factible",
                "V_Camino_Inv", "V_Ciclos", "V_Recursos_Exceso", "V_Nodos_Prohib",
                "V_Nodos_Req_Falt", "V_Ventanas_T",
                "Camino"
            ])
            
            # Escribir cada solución con su desglose
            for i, sol in enumerate(frente_pareto):
                es_factible = sol.violacion_total == 0
                camino_str = ' -> '.join(map(str, sol.camino))
                writer.writerow([
                    i + 1,
                    f"{sol.objetivos[0]:.4f}",
                    f"{sol.objetivos[1]:.4f}",
                    es_factible,
                    int(sol.violaciones['camino_invalido']),
                    int(sol.violaciones['ciclos']),
                    f"{sol.violaciones['recursos']:.4f}",
                    int(sol.violaciones['nodos_prohibidos']),
                    int(sol.violaciones['nodos_requeridos']),
                    f"{sol.violaciones['ventanas_tiempo']:.4f}",
                    camino_str
                ])
        print("✅ Resultados guardados exitosamente.")
    except Exception as e:
        print(f"❌ Error al escribir el archivo de resultados: {e}")


def main():
    """Función principal que orquesta todo el proceso."""
    # --- 1. Parseo de Argumentos de Línea de Comandos ---
    if len(sys.argv) not in [5, 6]:
        print("\nUso: ./nsga-II <instancia> <poblacion> <alpha> <generaciones> [paciencia_opcional]")
        print("Ejemplo: ./nsga-II ./instancias/5000n.txt 160 0.5 800 100\n")
        sys.exit(1)

    try:
        instancia_file = sys.argv[1]
        tam_poblacion = int(sys.argv[2])
        alpha = float(sys.argv[3])  # Capturado pero no usado directamente por NSGA-II
        num_generaciones = int(sys.argv[4])
        # El parámetro 'paciencia' es opcional, con un valor por defecto de 50
        paciencia = int(sys.argv[5]) if len(sys.argv) == 6 else 50
    except (ValueError, IndexError) as e:
        print(f"\nError en los argumentos: {e}. Asegúrate de que los valores numéricos son correctos.")
        sys.exit(1)
        
    print("-" * 50)
    print("Iniciando optimización con NSGA-II")
    print(f"  Instancia: {instancia_file}")
    print(f"  Configuración: Población={tam_poblacion}, Generaciones={num_generaciones}, Paciencia={paciencia}")
    print("-" * 50)

    # --- 2. Carga del Problema ---
    problema = ProblemaRCSPP(instancia_file)
    problema.alpha = alpha  # Guardamos alpha por si se necesita en el futuro

    # --- 3. Ejecución del Algoritmo NSGA-II ---
    print("\nIniciando algoritmo evolutivo...")
    frente_pareto = nsga2(problema, tam_poblacion, num_generaciones, paciencia)

    # --- 4. Impresión de Resultados en Consola ---
    print("\n" + "=" * 50)
    print("--- 🏆 RESUMEN DEL FRENTE DE PARETO FINAL 🏆 ---")
    if frente_pareto:
        print(f"Se encontraron {len(frente_pareto)} soluciones en el mejor frente de Pareto.")
        # Imprimir hasta 5 soluciones de ejemplo en la consola
        for i, sol in enumerate(frente_pareto[:5]):
            es_factible = sol.violacion_total == 0
            print(f"  Ej. Sol {i+1}: Dist={sol.objetivos[0]:.2f}, Tiempo={sol.objetivos[1]:.2f}, Factible={es_factible}")
        if len(frente_pareto) > 5:
            print("  ...")
    else:
        print("  No se encontraron soluciones en el frente de Pareto final.")
    print("=" * 50)
            
    # --- 5. Escritura de Resultados Detallados en Archivo CSV ---
    nombre_base_instancia = instancia_file.split('/')[-1].replace('.txt', '')
    nombre_archivo_salida = f"resultados_{nombre_base_instancia}_pob{tam_poblacion}_gen{num_generaciones}.csv"
    
    escribir_resultados_a_csv(frente_pareto, nombre_archivo_salida)


if __name__ == '__main__':
    main()

