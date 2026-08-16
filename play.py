import torch
import numpy as np
import time
import snake_env_gui
from model import Linear_QNet

def play():
    # 1. Inicializar el entorno
    env = snake_env_gui.SnakeEnv()
    
    # 2. Recrear la misma arquitectura de red neuronal (11 entradas, 256 ocultas, 3 salidas)
    model = Linear_QNet(11, 256, 3)
    
    # 3. Cargar los pesos del modelo entrenado
    # Asegúrate de que la ruta coincida con el modelo que quieres probar
    model_path = './model/model_2700.pth' 
    
    try:
        model.load_state_dict(torch.load(model_path))
        print("¡Modelo cargado exitosamente!")
    except Exception as e:
        print(f"Error al cargar el modelo: {e}")
        return

    # Poner el modelo en modo "evaluación" (buena práctica en PyTorch para inferencia)
    model.eval()

    n_games = 1
    record = 0
    
    # Opcional: Controlar la velocidad visual. 
    # En Raylib le pusimos 120 FPS. Si se mueve tan rápido que es un borrón, 
    # aumentaremos este delay.
    delay = 0.05 

    while True:
        # Obtener estado actual
        state = np.array(env.GetState(), dtype=int)
        state_tensor = torch.tensor(state, dtype=torch.float)
        
        # Predecir la mejor acción usando la red (sin calcular gradientes)
        with torch.no_grad():
            prediction = model(state_tensor)
            move_index = torch.argmax(prediction).item()

        # Mapear la predicción a la acción del entorno
        action_cpp = snake_env_gui.Action.STRAIGHT
        if move_index == 1:
            action_cpp = snake_env_gui.Action.TURN_RIGHT
        elif move_index == 2:
            action_cpp = snake_env_gui.Action.TURN_LEFT

        # Ejecutar acción
        result = env.Step(action_cpp)
        
        # Actualizar el récord local
        if result.score > record:
            record = result.score

        # Dibujar (Enviamos epsilon=0.0 porque no hay aleatoriedad)
        env.Render(n_games, record, 0.0)

        # Si muere, reiniciar
        if result.done:
            print(f"Partida {n_games} terminada. Puntuación: {result.score}")
            env.Reset()
            n_games += 1
            # Pausa de 1 segundo para poder ver dónde chocó antes de reiniciar
            time.sleep(1)
            
        # Pausa para que el ojo humano pueda ver la acción
        time.sleep(delay)

if __name__ == '__main__':
    play()