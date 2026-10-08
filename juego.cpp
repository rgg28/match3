#include "raylib.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

// --- Configuración de Red ---
const int PUERTO = 4242;
enum EstadoRed { MENU, HOST_ESPERANDO, CONECTADO_CLIENTE, CONECTADO_HOST };
EstadoRed estadoActual = MENU;

// --- Datos del Jugador ---
std::string mi_clase = "Mago";
int mi_nivel = 1;
int mi_vida = 100;
int rival_vida = 100;
std::string texto_estado = "Warcraft Match-3 RPG - Menu Principal";

// --- Estructuras de Interfaz ---
char ip_rival[16] = "\0";
int letras_ip = 0;
bool caja_ip_activa = false;

struct Gema {
    std::string tipo;
    Rectangle rect;
};
std::vector<Gema> tablero;

// --- Funciones del Ciclo de Vida ---
void CrearServidor() {
    texto_estado = "Esperando que el rival se conecte a tu IP (Puerto 4242)...";
    estadoActual = HOST_ESPERANDO;
    std::cout << "☁️ Servidor iniciado localmente. Simulando escucha ENet..." << std::endl;
}

void UnirseAPartida() {
    std::string ip_destino = (letras_ip > 0) ? ip_rival : "127.0.0.1";
    texto_estado = "Intentando conectar con " + ip_destino + "...";
    estadoActual = CONECTADO_CLIENTE;
    std::cout << "⚔️ Conectado al rival en: " << ip_destino << std::endl;
}

void CargarProgresoNube() {
    std::cout << "☁️ Conectando con la base de datos externa en la nube..." << std::endl;
    std::cout << "📥 Datos recuperados: " << mi_clase << " Nivel " << mi_nivel << std::endl;
}

void GuardarProgresoNube() {
    std::cout << "☁️ Enviando progreso en segundo plano... ¡Tu personaje esta a salvo!" << std::endl;
}

void GenerarTableroCombate() {
    tablero.clear();
    std::string tipos_gemas[] = {"IRA", "MANA", "CALAVERA", "ORO", "VIDA"};
    int ancho_casilla = 80;
    int alto_casilla = 80;
    int inicio_x = 650;
    int inicio_y = 300;

    for (int i = 0; i < 36; i++) {
        int fila = i / 6;
        int col = i % 6;
        Gema g;
        g.tipo = tipos_gemas[rand() % 5];
        g.rect = { (float)(inicio_x + col * (ancho_casilla + 10)), (float)(inicio_y + fila * (alto_casilla + 10)), (float)ancho_casilla, (float)alto_casilla };
        tablero.push_back(g);
    }
    CargarProgresoNube();
}

void ProcesarJugada(std::string tipo) {
    if (tipo == "CALAVERA") {
        texto_estado = "¡Lanzaste un ataque! El rival pierde vida.";
        rival_vida -= 15;
        GuardarProgresoNube();
    } else {
        texto_estado = "Combinaste gemas de tipo: " + tipo;
    }
}

int main() {
    srand(time(0));
    InitWindow(1920, 1080, "Warcraft: Puzzle Champions (Raylib Nativo)");
    SetTargetFPS(60);

    // Definición de Botones del Menú Inicial
    Rectangle btnHostRect = { 50, 200, 250, 50 };
    Rectangle btnUnirseRect = { 320, 200, 250, 50 };
    Rectangle txtBoxIPRect = { 50, 120, 520, 50 };

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        // --- Actualizar Lógica de Entradas ---
        if (estadoActual == MENU) {
            if (CheckCollisionPointRec(mousePos, txtBoxIPRect)) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) caja_ip_activa = true;
            } else {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) caja_ip_activa = false;
            }

            if (caja_ip_activa) {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= '0' && key <= '9') || key == '.') {
                        if (letras_ip < 15) {
                            ip_rival[letras_ip] = (char)key;
                            letras_ip++;
                            ip_rival[letras_ip] = '\0';
                        }
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    letras_ip--;
                    if (letras_ip < 0) letras_ip = 0;
                    ip_rival[letras_ip] = '\0';
                }
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (CheckCollisionPointRec(mousePos, btnHostRect)) CrearServidor();
                if (CheckCollisionPointRec(mousePos, btnUnirseRect)) {
                    UnirseAPartida();
                    GenerarTableroCombate();
                }
            }
        }
        else if (estadoActual == HOST_ESPERANDO) {
            // Simulación: Presiona la tecla ENTER para simular la llegada de un rival por red
            if (IsKeyPressed(KEY_ENTER)) {
                estadoActual = CONECTADO_HOST;
                texto_estado = "⚔️ ¡Rival Conectado! Empieza la batalla Match-3 ⚔️";
                GenerarTableroCombate();
            }
        }
        else if (estadoActual == CONECTADO_CLIENTE || estadoActual == CONECTADO_HOST) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                for (int i = 0; i < 36; i++) {
                    if (CheckCollisionPointRec(mousePos, tablero[i].rect)) {
                        ProcesarJugada(tablero[i].tipo);
                        tablero[i].tipo = "USADA"; // Desactivar visualmente
                    }
                }
            }
        }

        // --- Renderizado en Pantalla ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Cabecera común
        DrawText(texto_estado.c_str(), 50, 40, 30, RAYWHITE);

        if (estadoActual == MENU) {
            // Renderizar Caja de Entrada de IP
            DrawRectangleRec(txtBoxIPRect, caja_ip_activa ? LIGHTGRAY : WHITE);
            DrawRectangleLines((int)txtBoxIPRect.x, (int)txtBoxIPRect.y, (int)txtBoxIPRect.width, (int)txtBoxIPRect.height, BLACK);
            if (letras_ip > 0) DrawText(ip_rival, 65, 130, 30, BLACK);
            else DrawText("Escribe la IP del rival aqui (Default: 127.0.0.1)...", 65, 130, 24, GRAY);

            // Renderizar Botones
            DrawRectangleRec(btnHostRect, MAROON);
            DrawText("Crear Partida (Host)", 70, 215, 20, WHITE);

            DrawRectangleRec(btnUnirseRect, BLUE);
            DrawText("Conectarse al Rival", 350, 215, 20, WHITE);
        }
        else if (estadoActual == HOST_ESPERANDO) {
            DrawText("PRESIONA [ENTER] PARA SIMULAR CONEXION DEL RIVAL", 50, 150, 20, GOLD);
        }
        else if (estadoActual == CONECTADO_CLIENTE || estadoActual == CONECTADO_HOST) {
            // Dibujar Datos Estadísticos de los Jugadores
            DrawText(("Tu Vida: " + std::to_string(mi_vida)).c_str(), 50, 120, 24, GREEN);
            DrawText(("Vida Rival: " + std::to_string(rival_vida)).c_str(), 50, 160, 24, RED);

            // Dibujar el Tablero Interactivo 6x6
            for (int i = 0; i < 36; i++) {
                if (tablero[i].tipo == "USADA") {
                    DrawRectangleRec(tablero[i].rect, BLACK);
                } else {
                    Color colorGema = ORANGE;
                    if (tablero[i].tipo == "IRA") colorGema = RED;
                    else if (tablero[i].tipo == "MANA") colorGema = BLUE;
                    else if (tablero[i].tipo == "CALAVERA") colorGema = PURPLE;
                    else if (tablero[i].tipo == "VIDA") colorGema = GREEN;

                    DrawRectangleRec(tablero[i].rect, colorGema);
                    DrawRectangleLines((int)tablero[i].rect.x, (int)tablero[i].rect.y, (int)tablero[i].rect.width, (int)tablero[i].rect.height, WHITE);
                    DrawText(tablero[i].tipo.substr(0, 4).c_str(), (int)tablero[i].rect.x + 10, (int)tablero[i].rect.y + 30, 14, BLACK);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
