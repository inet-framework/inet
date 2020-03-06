import struct
from functools import singledispatch

import numpy as np

from keras.engine.input_layer import InputLayer
from keras.layers.core import (Dense, Flatten, Activation)
from keras.layers.convolutional import (Conv1D, Conv2D)
from keras.layers.local import (LocallyConnected1D, LocallyConnected2D)
from keras.layers.recurrent import LSTM
from keras.layers.advanced_activations import ELU
from keras.layers.pooling import MaxPooling2D
from keras.layers.embeddings import Embedding
from keras.layers.normalization import BatchNormalization

LAYERS = (
    InputLayer,
    Dense,
    Conv1D, Conv2D,
    LocallyConnected1D, LocallyConnected2D,
    Flatten,
    ELU,
    Activation,
    MaxPooling2D,
    LSTM,
    Embedding,
    BatchNormalization,
)

ACTIVATIONS = (
    'linear',
    'relu',
    'elu',
    'softplus',
    'softsign',
    'sigmoid',
    'tanh',
    'hard_sigmoid',
    'softmax',
)


def write_tensor(f, data, dims=1):
    """
    Writes tensor as flat array of floats to file in 1024 chunks,
    prevents memory explosion writing very large arrays to disk
    when calling struct.pack().
    """
    for stride in data.shape[:dims]:
        f.write(struct.pack('I', stride))

    data = data.flatten()
    step = 1024
    written = 0

    for i in np.arange(0, len(data), step):
        remaining = min(len(data) - i, step)
        written += remaining
        f.write(struct.pack(f'={remaining}f', *data[i: i + remaining]))

    assert written == len(data)


def export_activation(activation, f):
    try:
        f.write(struct.pack('I', ACTIVATIONS.index(activation) + 1))
    except ValueError as exc:
        raise NotImplementedError(activation) from exc


@singledispatch
def export(layer, _):
    raise NotImplementedError(layer)


@export.register(Flatten)
def _(_0, _1):
    pass


@export.register(Activation)
def _(layer, f):
    activation = layer.get_config()['activation']
    export_activation(activation, f)


@export.register(ELU)
def _(layer, f):
    f.write(struct.pack('f', layer.alpha))


@export.register(BatchNormalization)
def _(layer, f):
    epsilon = layer.epsilon
    gamma = layer.get_weights()[0]
    beta = layer.get_weights()[1]
    pop_mean = layer.get_weights()[2]
    pop_variance = layer.get_weights()[3]

    weights = gamma / np.sqrt(pop_variance + epsilon)
    biases = beta - pop_mean * weights

    write_tensor(f, weights)
    write_tensor(f, biases)


@export.register(InputLayer)
def _(layer, f):
    pass

@export.register(Dense)
def _(layer, f):
    # shape: (outputs, dims)
    weights = layer.get_weights()[0].transpose()
    biases = layer.get_weights()[1]
    activation = layer.get_config()['activation']

    write_tensor(f, weights, 2)
    write_tensor(f, biases)
    export_activation(activation, f)


@export.register(Conv1D)
def _(layer, f):
    # shape: (outputs, steps, dims)
    weights = layer.get_weights()[0].transpose(2, 0, 1)
    biases = layer.get_weights()[1]
    activation = layer.get_config()['activation']

    write_tensor(f, weights, 3)
    write_tensor(f, biases)
    export_activation(activation, f)


@export.register(Conv2D)
def _(layer, f):
    # shape: (outputs, rows, cols, depth)
    weights = layer.get_weights()[0].transpose(3, 0, 1, 2)
    biases = layer.get_weights()[1]
    activation = layer.get_config()['activation']

    write_tensor(f, weights, 4)
    write_tensor(f, biases)
    export_activation(activation, f)


@export.register(LocallyConnected1D)
def _(layer, f):
    # shape: (new_steps, outputs, ksize*dims)
    weights = layer.get_weights()[0].transpose(0, 2, 1)
    biases = layer.get_weights()[1]
    activation = layer.get_config()['activation']

    write_tensor(f, weights, 3)
    write_tensor(f, biases, 2)
    export_activation(activation, f)


@export.register(LocallyConnected2D)
def _(layer, f):
    # shape: (rows*cols, outputs, ksize*depth)
    weights = layer.get_weights()[0]
    # weights = weights.transpose(0, 2, 1)
    biases = layer.get_weights()[1]
    activation = layer.get_config()['activation']

    write_tensor(f, weights, 3)
    write_tensor(f, biases, 2)
    export_activation(activation, f)


@export.register(MaxPooling2D)
def _(layer, f):
    pool_size = layer.get_config()['pool_size']

    f.write(struct.pack('I', pool_size[0]))
    f.write(struct.pack('I', pool_size[1]))


@export.register(LSTM)
def _(layer, f):
    inner_activation = layer.get_config()['recurrent_activation']
    activation = layer.get_config()['activation']
    return_sequences = int(layer.get_config()['return_sequences'])

    weights = layer.get_weights()
    units = layer.units

    kernel, rkernel, bias = ([x[i: i+units] for i in range(0, 4*units, units)]
                             for x in (weights[0].transpose(),
                                       weights[1].transpose(),
                                       weights[2]))
    bias = [x.reshape(1, -1) for x in bias]
    for tensors in zip(kernel, rkernel, bias):
        for tensor in tensors:
            write_tensor(f, tensor, 2)

    export_activation(inner_activation, f)
    export_activation(activation, f)
    f.write(struct.pack('I', return_sequences))


@export.register(Embedding)
def _(layer, f):
    weights = layer.get_weights()[0]
    write_tensor(f, weights, 2)


def export_model(model, filename):
    with open(filename, 'wb') as f:
        layers = [layer for layer in model.layers
                  if type(layer).__name__ not in ['Dropout']]
        f.write(struct.pack('I', len(layers)))

        for layer in layers:
            f.write(struct.pack('I', LAYERS.index(type(layer)) + 1))
            export(layer, f)
