#include "raylib.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>

// --- Configuración de Red ---
enum EstadoRed { MENU, HOST_ESPERANDO, CONECTADO };
EstadoRed estadoActual = MENU;
std::string texto_estado = "Warcraft Match-3 RPG - Menu Principal";

// --- Datos del Juego RPG ---
int mi_vida = 100;
int rival_vida = 100;

// --- Configuración e Interfaz del Tablero ---
const int FILAS = 6;
const int COLUMNAS = 6;
const int TAM_CASILLA = 70;
const int ESPACIO = 8;
const int ORIGEN_X = 650;
const int ORIGEN_Y = 250;

// Tipos de gemas: 0=Ira(Rojo), 1=Mana(Azul), 2=Calavera(Purpura), 3=Oro(Amarillo), 4=Vida(Verde)
enum TipoGema { IRA, MANA, CALAVERA, ORO, VIDA, VACIO };
int matriz[FILAS][COLUMNAS];

// Control de Selección para Intercambio
int celdaSeleccionadaFila = -1;
int celdaSeleccionadaCol = -1;

// --- Funciones del Flujo del Match-3 ---

void GenerarTableroInicial() {
    for (int f = 0; f < FILAS; f++) {
        for (int c = 0; c < COLUMNAS; c++) {
            do {
                matriz[f][c] = rand() % 5;
            } while ((c >= 2 && matriz[f][c] == matriz[f][c-1] && matriz[f][c] == matriz[f][c-2]) ||
                     (f >= 2 && matriz[f][c] == matriz[f-1][c] && matriz[f][c] == matriz[f-2][c]));
        }
    }
}

bool VerificarYEliminarCombinaciones() {
    bool hubo_combinacion = false;
    bool eliminar[FILAS][COLUMNAS] = { false };

    // Validar líneas horizontales
    for (int f = 0; f < FILAS; f++) {
        for (int c = 0; c < COLUMNAS - 2; c++) {
            if (matriz[f][c] != VACIO && matriz[f][c] == matriz[f][c+1] && matriz[f][c] == matriz[f][c+2]) {
                eliminar[f][c] = eliminar[f][c+1] = eliminar[f][c+2] = true;
                hubo_combinacion = true;
            }
        }
    }

    // Validar líneas verticales
    for (int c = 0; c < COLUMNAS; c++) {
        for (int f = 0; f < FILAS - 2; f++) {
            if (matriz[f][c] != VACIO && matriz[f][c] == matriz[f+1][c] && matriz[f][c] == matriz[f+2][c]) {
                eliminar[f][c] = eliminar[f+1][c] = eliminar[f+2][c] = true;
                hubo_combinacion = true;
            }
        }
    }

    // Procesar efectos RPG si se destruyen calaveras
    for (int f = 0; f < FILAS; f++) {
        for (int c = 0; c < COLUMNAS; c++) {
            if (eliminar[f][c]) {
                if (matriz[f][c] == CALAVERA) {
                    rival_vida -= 5;
                    texto_estado = "¡Lanzaste un ataque! El rival pierde vida.";
                }
                matriz[f][c] = VACIO;
            }
        }
    }

    return hubo_combinacion;
}

void AplicarGravedadYCompletar() {
    // Caída de gemas existentes
    for (int c = 0; c < COLUMNAS; c++) {
        for (int f = FILAS - 1; f >= 0; f--) {
            if (matriz[f][c] == VACIO) {
                for (int k = f - 1; k >= 0; k--) {
                    if (matriz[k][c] != VACIO) {
                        matriz[f][c] = matriz[k][c];
                        matriz[k][c] = VACIO;
                        break;
                    }
                }
            }
        }
    }
    // Rellenar huecos superiores con nuevas gemas
    for (int f = 0; f < FILAS; f++) {
        for (int c = 0; c < COLUMNAS; c++) {
            if (matriz[f][c] == VACIO) {
                matriz[f][c] = rand() % 5;
            }
        }
    }
}

void IntercambiarGemas(int f1, int c1, int f2, int c2) {
    int temp = matriz[f1][c1];
    matriz[f1][c1] = matriz[f2][c2];
    matriz[f2][c2] = temp;

    // Si el intercambio no produce un Match-3, se deshace el movimiento
    if (!VerificarYEliminarCombinaciones()) {
        int temp2 = matriz[f1][c1];
        matriz[f1][c1] = matriz[f2][c2];
        matriz[f2][c2] = temp2;
        texto_estado = "Movimiento invalido. No genera Match-3.";
    } else {
        // Ciclo automático para combos encadenados por gravedad
        do {
            AplicarGravedadYCompletar();
        } while (VerificarYEliminarCombinaciones());
    }
}

int main() {
    srand(time(0));
    InitWindow(1920, 1080, "Warcraft: Puzzle Champions");
    SetTargetFPS(60);

    Rectangle btnHostRect = { 50, 150, 250, 50 };
    Rectangle btnUnirseRect = { 320, 150, 250, 50 };

    GenerarTableroInicial();

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        // --- Manejo del Menú de Red ---
        if (estadoActual == MENU) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (CheckCollisionPointRec(mousePos, btnHostRect)) {
                    estadoActual = HOST_ESPERANDO;
                    texto_estado = "Esperando rival... Presiona [ENTER] para simular conexion.";
                }
                if (CheckCollisionPointRec(mousePos, btnUnirseRect)) {
                    estadoActual = CONECTADO;
                    texto_estado = "⚔️ ¡Conectado! Intercambia gemas adyacentes clicando una y luego otra.";
                }
            }
        }
        else if (estadoActual == HOST_ESPERANDO) {
            if (IsKeyPressed(KEY_ENTER)) {
                estadoActual = CONECTADO;
                texto_estado = "⚔️ ¡Rival Conectado! Encuentra combinaciones de 3.";
            }
        }
        // --- Manejo de la Lógica del Match-3 Real ---
        else if (estadoActual == CONECTADO) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                for (int f = 0; f < FILAS; f++) {
                    for (int c = 0; c < COLUMNAS; c++) {
                        Rectangle celdaRect = { 
                            (float)(ORIGEN_X + c * (TAM_CASILLA + ESPACIO)), 
                            (float)(ORIGEN_Y + f * (TAM_CASILLA + ESPACIO)), 
                            (float)TAM_CASILLA, (float)TAM_CASILLA 
                        };

                        if (CheckCollisionPointRec(mousePos, celdaRect)) {
                            if (celdaSeleccionadaFila == -1) {
                                // Primer clic: Seleccionar gema
                                celdaSeleccionadaFila = f;
                                celdaSeleccionadaCol = c;
                            } else {
                                // Segundo clic: Comprobar si es vecina inmediata
                                int diffFila = abs(f - celdaSeleccionadaFila);
                                int diffCol = abs(c - celdaSeleccionadaCol);

                                if ((diffFila == 1 && diffCol == 0) || (diffFila == 0 && diffCol == 1)) {
                                    IntercambiarGemas(celdaSeleccionadaFila, celdaSeleccionadaCol, f, c);
                                }
                                // Resetear selección
                                celdaSeleccionadaFila = -1;
                                celdaSeleccionadaCol = -1;
                            }
                        }
                    }
                }
            }
        }

        // --- Renderizado Gráfico ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawText(texto_estado.c_str(), 50, 40, 26, RAYWHITE);

        if (estadoActual == MENU) {
            DrawRectangleRec(btnHostRect, MAROON);
            DrawText("Crear Partida (Host)", 70, 165, 20, WHITE);

            DrawRectangleRec(btnUnirseRect, BLUE);
            DrawText("Conectarse al Rival", 350, 165, 20, WHITE);
        }
        else if (estadoActual == HOST_ESPERANDO) {
            DrawText("PRESIONA EL BOTON [ENTER] PARA SIMULAR JUGADOR", 50, 150, 20, GOLD);
        }
        else if (estadoActual == CONECTADO) {
            DrawText(("Tu Vida: " + std::to_string(mi_vida)).c_str(), 50, 120, 24, GREEN);
            DrawText(("Vida Rival: " + std::to_string(rival_vida)).c_str(), 50, 160, 24, RED);

            // Dibujar el Tablero de Match-3
            for (int f = 0; f < FILAS; f++) {
                for (int c = 0; c < COLUMNAS; c++) {
                    int tipo = matriz[f][c];
                    Color colorGema = BLACK;

                    if (tipo == IRA) colorGema = RED;
                    else if (tipo == MANA) colorGema = BLUE;
                    else if (tipo == CALAVERA) colorGema = PURPLE;
                    else if (tipo == ORO) colorGema = YELLOW;
                    else if (tipo == VIDA) colorGema = GREEN;

                    int x = ORIGEN_X + c * (TAM_CASILLA + ESPACIO);
                    int y = ORIGEN_Y + f * (TAM_CASILLA + ESPACIO);

                    // Si está seleccionada temporalmente, dibujarla con un borde brillante
                    if (f == celdaSeleccionadaFila && c == celdaSeleccionadaCol) {
                        DrawRectangle(x - 4, y - 4, TAM_CASILLA + 8, TAM_CASILLA + 8, GOLD);
                    }

                    DrawRectangle(x, y, TAM_CASILLA, TAM_CASILLA, colorGema);
                    DrawRectangleLines(x, y, TAM_CASILLA, TAM_CASILLA, WHITE);

                    // Nombre abreviado de la gema en el centro
                    std::string textoGema = (tipo == IRA) ? "IRA" : (tipo == MANA) ? "MANA" : (tipo == CALAVERA) ? "SKULL" : (tipo == ORO) ? "ORO" : "VIDA";
                    DrawText(textoGema.c_str(), x + 12, y + 26, 14, (tipo == ORO) ? BLACK : WHITE);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
