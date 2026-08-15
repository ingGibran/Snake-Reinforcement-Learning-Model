import torch
import random
import numpy as np
from collections import deque
import snake_env
from model import Linear_QNet, QTrainer

MAX_MEMORY = 100_000
BATCH_SIZE = 1000
LR = 0.001

class Agent:
    def __init__(self):
        self.n_games = 0
        self.epsilon = 0 # Controla la aleatoriedad (Exploración)
        self.gamma = 0.9 # Tasa de descuento
        self.memory = deque(maxlen=MAX_MEMORY) # Replay Buffer (si se llena, borra lo más viejo)
        
        # El cerebro: 11 entradas (estado), capa oculta de 256, 3 salidas (acciones)
        self.model = Linear_QNet(11, 256, 3) 
        self.trainer = QTrainer(self.model, lr=LR, gamma=self.gamma)

    def get_state(self, env):
        # El entorno ya nos da la lista de 11 booleanos
        return np.array(env.GetState(), dtype=int)

    def remember(self, state, action, reward, next_state, done):
        # Guardar en memoria la tupla de la experiencia
        self.memory.append((state, action, reward, next_state, done))

    def train_long_memory(self):
        # Entrenar con un lote de experiencias pasadas (Minibatch)
        if len(self.memory) > BATCH_SIZE:
            mini_sample = random.sample(self.memory, BATCH_SIZE)
        else:
            mini_sample = self.memory

        states, actions, rewards, next_states, dones = zip(*mini_sample)
        self.trainer.train_step(states, actions, rewards, next_states, dones)

    def train_short_memory(self, state, action, reward, next_state, done):
        # Entrenar solo con el movimiento que acaba de ocurrir
        self.trainer.train_step(state, action, reward, next_state, done)

    def get_action(self, state):
        # EXPLORACIÓN VS EXPLOTACIÓN
        # Al inicio, hace muchos movimientos al azar para descubrir el juego
        self.epsilon = 80 - self.n_games
        final_move = [0, 0, 0]
        
        if random.randint(0, 200) < self.epsilon:
            move = random.randint(0, 2)
            final_move[move] = 1
        else:
            # Pide la predicción al modelo de PyTorch
            state0 = torch.tensor(state, dtype=torch.float)
            prediction = self.model(state0)
            move = torch.argmax(prediction).item()
            final_move[move] = 1

        return final_move


def train():
    record = 0
    agent = Agent()
    env = snake_env.SnakeEnv()
    
    print("Iniciando entrenamiento...")

    while True:
        # Obtener estado anterior
        state_old = agent.get_state(env)

        # Decidir movimiento
        move = agent.get_action(state_old)
        
        # Mapear el array [1,0,0] a la acción de C++
        action_cpp = snake_env.Action.STRAIGHT
        if move[1] == 1:
            action_cpp = snake_env.Action.TURN_RIGHT
        elif move[2] == 1:
            action_cpp = snake_env.Action.TURN_LEFT

        # Ejecutar movimiento en el entorno
        result = env.Step(action_cpp)
        state_new = np.array(result.state, dtype=int)
        
        # Entrenar memoria a corto plazo
        agent.train_short_memory(state_old, move, result.reward, state_new, result.done)

        # Recordar
        agent.remember(state_old, move, result.reward, state_new, result.done)

        if result.done:
            # La serpiente murió. Reiniciar partida y entrenar memoria a largo plazo.
            env.Reset()
            agent.n_games += 1
            agent.train_long_memory()

            if result.score > record:
                record = result.score
                agent.model.save() # Guarda el modelo si rompe récord

            print(f'Partida: {agent.n_games} | Puntuación: {result.score} | Récord: {record}')

if __name__ == '__main__':
    train()