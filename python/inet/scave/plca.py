import matplotlib.pyplot as plt
from matplotlib.widgets import Button
from numpy import argmax

def add_go_to_max_packet_pending_delay_button(df, props):
    packetPendingDelayVector = df[df.name == "packetPendingDelay:vector"].iloc[0]
    pos = argmax(packetPendingDelayVector.vecvalue)
    maxPacketPendingDelaySendTime = packetPendingDelayVector.vectime[pos]
    df.drop(df[df['name'] == 'packetPendingDelay:vector'].index, inplace=True)
    def go_to_max_pending_packet_delay(event):
        ax = plt.gcf().axes[1]
        ax.set_xlim(maxPacketPendingDelaySendTime - packetPendingDelayVector.vecvalue[pos] - 10E-6, maxPacketPendingDelaySendTime + 10E-6)
        plt.draw()
    button_ax = plt.axes([0.3, 0.95, 0.4, 0.04])
    button_ax.tick_params(axis='both', which='both', bottom=False, top=False, left=False, right=False)
    button_ax.get_xaxis().set_visible(False)
    button_ax.get_yaxis().set_visible(False)
    button = Button(button_ax, 'Go to max packet pending delay')
    button.on_clicked(go_to_max_pending_packet_delay)
    return button
