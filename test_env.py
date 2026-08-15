import snake_env

# Inicializar el entorno escrito en C++
env = snake_env.SnakeEnv()

# Ver el estado inicial (11 variables booleanas)
estado_inicial = env.GetState()
print(f"Estado inicial: {estado_inicial}")

# Hacer que la IA (manualmente) avance recto
resultado = env.Step(snake_env.Action.STRAIGHT)

print(f"Nuevo estado: {resultado.state}")
print(f"Recompensa obtenida: {resultado.reward}")
print(f"¿Terminó el juego?: {resultado.done}")
print(f"Puntuación: {resultado.score}")