import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as F
import os

class Linear_QNet(nn.Module):
    def __init__(self, input_size, hidden_size, output_size):
        super().__init__()
        # input_size = 11 (nuestro GetState de C++)
        # output_size = 3 (nuestras acciones: recto, derecha, izquierda)
        self.linear1 = nn.Linear(input_size, hidden_size)
        self.linear2 = nn.Linear(hidden_size, output_size)

    def forward(self, x):
        # Usamos ReLU como función de activación en la capa oculta
        x = F.relu(self.linear1(x))
        x = self.linear2(x)
        return x

    def save(self, file_name='model.pth'):
        model_folder_path = './model'
        if not os.path.exists(model_folder_path):
            os.makedirs(model_folder_path)
        file_name = os.path.join(model_folder_path, file_name)
        torch.save(self.state_dict(), file_name)



class QTrainer:
    def __init__(self, model, lr, gamma):
        self.lr = lr
        self.gamma = gamma # Tasa de descuento (qué tanto le importan las recompensas futuras)
        self.model = model
        self.optimizer = optim.Adam(model.parameters(), lr=self.lr)
        self.criterion = nn.MSELoss() # Error Cuadrático Medio

    def train_step(self, state, action, reward, next_state, done):
        # Convertimos los datos a tensores de PyTorch
        state = torch.tensor(state, dtype=torch.float)
        next_state = torch.tensor(next_state, dtype=torch.float)
        action = torch.tensor(action, dtype=torch.long)
        reward = torch.tensor(reward, dtype=torch.float)
        
        # Si pasamos un solo estado, le agregamos una dimensión (1, x)
        if len(state.shape) == 1:
            state = torch.unsqueeze(state, 0)
            next_state = torch.unsqueeze(next_state, 0)
            action = torch.unsqueeze(action, 0)
            reward = torch.unsqueeze(reward, 0)
            done = (done, )

        # 1. Obtener los Q-values predichos con el estado actual
        pred = self.model(state)

        # 2. Aplicar la Ecuación de Bellman para los Q-values objetivo
        target = pred.clone()
        for idx in range(len(done)):
            Q_new = reward[idx]
            if not done[idx]:
                # Q_new = r + gamma * max(Siguiente Q)
                Q_new = reward[idx] + self.gamma * torch.max(self.model(next_state[idx]))
            
            # Actualizamos el valor solo de la acción que se tomó
            target[idx][torch.argmax(action[idx]).item()] = Q_new

        # 3. Backpropagation
        self.optimizer.zero_grad()
        loss = self.criterion(target, pred)
        loss.backward()
        self.optimizer.step()