import inet.scave.plot
from matplotlib.lines import Line2D

alternate_handles = [Line2D([0], [0], color='black', lw=1, linestyle=':', label='Line'),
                     Line2D([0], [0], color='black', lw=1, linestyle='--', label='Line'),
                     Line2D([0], [0], color='black', lw=1, linestyle='-', label='Line'),
                     Line2D([0], [0], color='r', lw=4, linestyle='', marker='s', markersize=1, label='Line'),
                     Line2D([0], [0], color='b', lw=4, linestyle='', marker='s', markersize=1, label='Line')]

alternate_labels = ['FifoQueueing', 'PriorityQueueing', 'FramePreemption', 'Background', 'High-Priority']

def add_custom_style(df):
    style_tuple_list = [('module', 'FramePreemptionShowcase.host2.app\[0\].sink', {'color': 'red'}),
                    ('module', 'FramePreemptionShowcase.host2.app\[1\].sink', {'color': 'blue'}),
                    ('configname', '.*FramePreemption', {'linestyle': 'solid'}),
                    ('configname', '.*FifoQueueing', {'linestyle': 'dotted'}),
                    ('configname', '.*PriorityQueueing', {'linestyle': 'dashed'})]
    df = inet.scave.plot.add_to_dataframe(df, style_tuple_list)
    return df

# rectangle location
xmin = 0.41
xmax = 0.82
ymin = 0.24
ymax = 0.27

# distance of text from rectangle
text_offset = 0.002