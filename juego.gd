extends Node2D

# Configuración de Red
const PUERTO = 4242
var peer = ENetMultiplayerPeer.new()

# Datos del Jugador (RPG)
var mi_clase = "Mago"
var mi_nivel = 1
var mi_vida = 100
var rival_vida = 100

# Elementos de Interfaz Visual que crearemos por código
var label_estado: Label
var input_ip: LineEdit
var btn_host: Button
var btn_unirse: Button
var contenedor_tablero: GridContainer

func _ready():
	# 1. Diseñar la interfaz básica de red en pantalla
	label_estado = Label.new()
	label_estado.text = "Warcraft Match-3 RPG - Menú Principal"
	label_estado.position = Vector2(50, 30)
	add_child(label_estado)
	
	input_ip = LineEdit.new()
	input_ip.placeholder_text = "Escribe la IP del rival aquí..."
	input_ip.position = Vector2(50, 80)
	input_ip.size = Vector2(300, 40)
	add_child(input_ip)
	
	btn_host = Button.new()
	btn_host.text = "Crear Partida (Host)"
	btn_host.position = Vector2(50, 140)
	btn_host.pressed.connect(_crear_servidor)
	add_child(btn_host)
	
	btn_unirse = Button.new()
	btn_unirse.text = "Conectarse al Rival"
	btn_unirse.position = Vector2(200, 140)
	btn_unirse.pressed.connect(_unirse_a_partida)
	add_child(btn_unirse)

	# Configurar eventos de red nativos de Godot
	multiplayer.peer_connected.connect(_rival_conectado)

# --- LÓGICA MULTI_JUGADOR P2P ---
func _crear_servidor():
	peer.create_server(PUERTO, 2)
	multiplayer.multiplayer_peer = peer
	label_estado.text = "Esperando que el rival se conecte a tu IP..."
	btn_host.disabled = true
	btn_unirse.disabled = true

func _unirse_a_partida():
	var ip_destino = input_ip.text if input_ip.text != "" else "127.0.0.1"
	peer.create_client(ip_destino, PUERTO)
	multiplayer.multiplayer_peer = peer
	label_estado.text = "Intentando conectar con " + ip_destino + "..."

func _rival_conectado(_id):
	label_estado.text = "⚔️ ¡Rival Conectado! Empieza la batalla Match-3 ⚔️"
	_ocultar_menu_inicial()
	_generar_tablero_combate()
	_cargar_progreso_nube()

# --- LÓGICA DEL TABLERO MATCH-3 RPG ---
func _ocultar_menu_inicial():
	input_ip.hide()
	btn_host.hide()
	btn_unirse.hide()

func _generar_tablero_combate():
	contenedor_tablero = GridContainer.new()
	contenedor_tablero.columns = 6
	contenedor_tablero.position = Vector2(400, 150)
	add_child(contenedor_tablero)
	
	# Tipos de gemas: 🟥 Ira, 🟦 Maná, 💀 Calavera (Daño)
	var gemas = ["🟥", "🟦", "💀", "🟨", "🟩"]
	
	# Generamos un tablero básico de 6x6 botones interactivos
	for i in range(36):
		var btn_gema = Button.new()
		btn_gema.text = gemas[randi() % gemas.size()]
		btn_gema.custom_minimum_size = Vector2(60, 60)
		# Al hacer clic simulamos que el jugador destruye esa gema
		btn_gema.pressed.connect(func(): _procesar_jugada(btn_gema.text))
		contenedor_tablero.add_child(btn_gema)

func _procesar_jugada(tipo_gema):
	# Ejecutamos la acción sincronizada en ambas pantallas usando RPC (Remote Procedure Call)
	rpc("_sincronizar_accion_rpg", tipo_gema, multiplayer.get_unique_id())

@rpc("any_peer", "call_local")
func _sincronizar_accion_rpg(tipo_gema, id_autor):
	if tipo_gema == "💀":
		if id_autor == multiplayer.get_unique_id():
			label_estado.text = "¡Lanzaste un ataque! El rival pierde vida."
			_guardar_progreso_nube() # Guardamos la victoria o avance
		else:
			label_estado.text = "¡El rival te atacó con calaveras!"

# --- MÓDULO SIMULADO DE GUARDADO EN LA NUBE ---
func _cargar_progreso_nube():
	print("☁️ Conectando con la base de datos externa en la nube...")
	print("📥 Datos recuperados: " + mi_clase + " Nivel " + str(mi_nivel))

func _guardar_progreso_nube():
	print("☁️ Enviando progreso en segundo plano... ¡Tu personaje está a salvo!")
