def processDataFrame(df):
    df['module'] = df['module'].map({
        'EthernetInterfaceConfigurationExample.host2.app[0]': 'background',
        'EthernetInterfaceConfigurationExample.host2.app[1]': 'video',
        'EthernetInterfaceConfigurationExample.host2.app[2]': 'voice',
        'EthernetInterfaceConfigurationExample.host2.app[3]': 'low-ts',
        'EthernetInterfaceConfigurationExample.host2.app[4]': 'high-ts'
    })
    df['m'] = df['module'].map({
        'background': 0,
        'video': 1,
        'voice': 2,
        'low-ts': 3,
        'high-ts': 4,
    })
    df.sort_values(by='m', kind='mergesort', inplace=True)
    if 'configname' in df:
        df['c'] = df['configname'].map({
            'Default': 0,
            'Basic': 1,
            'PriorityQueueing': 2,
            'SelectiveQueueing': 3,
            'Preemption': 4,
            'PreemptionWithPriorityQueueing': 5,
            'PreemptionWithSelectiveQueueing': 6,
        })
        df.sort_values(by='c', kind='mergesort', inplace=True)
    return df

def processPivotDataFrame(df):
    order = ['Default', 'Basic', 'PriorityQueueing', 'SelectiveQueueing', 'Preemption', 'PreemptionWithPriorityQueueing', 'PreemptionWithSelectiveQueueing']
    df = df.reindex(order, axis='columns')
    order = ['background', 'video', 'voice', 'low-ts', 'high-ts']
    df = df.reindex(order, axis='index')
    return df
