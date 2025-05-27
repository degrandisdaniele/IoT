import numpy as np
import matplotlib.pyplot as plt

# Parameters
hours = 48
samples = 500
time = np.linspace(0, hours, samples)

# Voltage function approximating a LiPo discharge
# Initial slight drop, main plateau, steep drop, cutoff
def lipo_voltage(t):
    if t < 5:
        # Slight initial drop
        return 8.29 - (8.29 - 8.0) * t / 5
    elif 5 <= t < 30:
        # Main plateau
        return 8.0
    elif 30 <= t < 36:
        # Steep drop towards cutoff
        return 8.0 - (8.0 - 6.0) * (t - 30) / 6
    else:
        # Cutoff voltage
        return 6.0

voltage = np.array([lipo_voltage(t) for t in time])

# Current function (depends on time, but drops to zero at voltage cutoff)
current_base = 42 + 8 * np.sin(2 * np.pi * time / (10 / 60)) * (1 - (time / 48) * 0.1)
# Current drops to zero when voltage reaches cutoff (6.0V)
current = np.where(voltage <= 6.0, 0, current_base)

# Potenza (tensione * corrente)
power = voltage * current

# Calcola le medie (solo per il periodo in cui la tensione è sopra il cutoff)
valid_indices = voltage > 6.0
average_voltage = np.mean(voltage[valid_indices]) if np.any(valid_indices) else 0
average_current = np.mean(current[valid_indices]) if np.any(valid_indices) else 0
average_power = np.mean(power[valid_indices]) if np.any(valid_indices) else 0

# Configurazione dei colori (RGB normalizzato tra 0 e 1)
blue = (0, 0, 1)
red = (1, 0, 0)
yellow = (1, 215/255, 0)

# Crea una singola figura con 3 subplots disposti verticalmente
fig, axes = plt.subplots(3, 1, figsize=(10, 12))

# Grafico Tensione
axes[0].plot(time, voltage, color=blue, linewidth=1, label='Voltage (V)')
axes[0].set_xlabel('Time (hours)')
axes[0].set_ylabel('Voltage (V)')
axes[0].set_title('Voltage vs. Time ')
axes[0].set_xlim(0, hours)
axes[0].set_ylim(5.5, 8.5) # Adjusted y-limit
axes[0].grid(True)
axes[0].text(0.5, -0.2, f'Average : {average_voltage:.2f} V', ha='center', transform=axes[0].transAxes, fontsize=10)

# Current Plot
axes[1].plot(time, current, color=red, linewidth=1, label='Current (mA)')
axes[1].set_xlabel('Time (hours)')
axes[1].set_ylabel('Current (mA)')
axes[1].set_title('Current vs. Time')
axes[1].set_xlim(0, hours)
axes[1].set_ylim(0, 55) # Adjusted y-limit
axes[1].grid(True)
axes[1].text(0.5, -0.2, f'Average: {average_current:.2f} mA', ha='center', transform=axes[1].transAxes, fontsize=10)

# Power Plot
axes[2].plot(time, power, color=yellow, linewidth=1, label='Power (mW)')
axes[2].set_xlabel('Time (hours)')
axes[2].set_ylabel('Power (mW)')
axes[2].set_title('Power vs. Time')
axes[2].set_xlim(0, hours)
axes[2].set_ylim(0, 450) # Adjusted y-limit
axes[2].grid(True)
axes[2].text(0.5, -0.2, f'Averagex: {average_power:.2f} mW', ha='center', transform=axes[2].transAxes, fontsize=10)

# Adjust spacing between subplots
plt.tight_layout()

# Salva la figura combinata
plt.savefig('combined_graphs_lipo.png')

# Mostra la figura
plt.show()