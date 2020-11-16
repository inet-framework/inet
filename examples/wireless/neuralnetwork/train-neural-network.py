#!/usr/bin/python3

import re
import os
import sys
import csv
import math
import code
import numpy
import random
import argparse
import tensorflow
import keras.utils

from keras.models import Model, Sequential
from keras.layers import Input, Activation, Dense, InputLayer, Concatenate, BatchNormalization, Conv1D, LSTM, GlobalAveragePooling1D, MaxPool1D, AveragePooling1D, concatenate
from keras.optimizers import SGD, Adam, Adagrad
from keras2cpp import keras2cpp
from PySide2.QtWidgets import QWidget, QSlider, QHBoxLayout, QSplitter, QScrollArea, QLabel, QApplication, QDialog, QLineEdit, QPushButton
from tensorflow.keras import callbacks

numpy.set_printoptions(threshold=sys.maxsize)

parser = argparse.ArgumentParser()
parser.add_argument("-o", "--output", default=None)
parser.add_argument("-c", "--constantInputs", default=None)
parser.add_argument("-p", "--optimizer", default="adam")
parser.add_argument("-e", "--epochs", default=1000, type=int)
parser.add_argument("-b", "--batch", default=256, type=int)
parser.add_argument("-n", "--numSamples", default=0, type=int)
parser.add_argument("-g", "--showGui", default=False, action='store_true')
parser.add_argument("-i", "--interactive", default=False, action='store_true')
parser.add_argument("trainingDataset", nargs="+")
args = parser.parse_args()

model = None

def loadTrainingDatasetFile(filename, inputs_fixed, inputs_vary, outputs):
    with open(filename, "r") as f:
        reader = csv.reader(f, delimiter=",")
        for line in reader:
            if not line or line[0].startswith("#") or not line[0].isdigit():
                continue
            line = [w for w in line if w.strip()]
            #print(line)
            index, packetErrorRate, noisePowerMean, noisePowerStddev, numInterferingSignals, meanInterferingSignalNoisePowerMean, meanInterferingSignalNoisePowerStddev, bitrate, packetLength, modulation, subcarrierModulation, centerFrequency, bandwidth, timeDivision, frequencyDivision, numSymbols, preambleDuration, headerDuration, dataDuration, duration = line[0:20]
            if packetLength.endswith("b"):
                continue
            line = line[20:]
            packetErrorRate = float(packetErrorRate)
            noisePowerMean = float(noisePowerMean.strip()[:-2])
            noisePowerStddev = float(noisePowerStddev.strip()[:-2])
            bitrate = float(bitrate.strip()[:-4])
            packetLength = int(packetLength.strip()[:-2])
            centerFrequency = float(centerFrequency.strip()[:-3])
            bandwidth = float(bandwidth.strip()[:-3])
            numSymbols = int(numSymbols.strip())
            packetBytes = line[:packetLength]
            symbolSnirMeans = line[-numSymbols:]
            if args.constantInputs is not None :
                symbolSnirMeans = args.constantInputs.split(",") + symbolSnirMeans
            #print(index, packetErrorRate, noisePowerMean, noisePowerStddev, bitrate, packetLength, modulation, centerFrequency, bandwidth, numSymbols, len(symbolSnirMeans))
            snirarray = numpy.array([float(snir) for snir in symbolSnirMeans])
            snirarray = numpy.log(snirarray)
            inputs_fixed.append(numpy.array([bitrate, packetLength, centerFrequency, bandwidth, numSymbols]))
            inputs_vary.append(numpy.array(snirarray))
            outputs.append(numpy.array([packetErrorRate]))

def loadTrainingDataset(trainingDataset):
    inputs_fixed = list()
    inputs_vary = list()
    outputs = list()
    for filename in trainingDataset:
        print(f"Loading training dataset from {filename}")
        loadTrainingDatasetFile(filename, inputs_fixed, inputs_vary, outputs)
    return inputs_fixed, inputs_vary, numpy.array(outputs)

inputs_fixed, inputs_vary, outputs = loadTrainingDataset(args.trainingDataset)

"""
def buildModel(inputs_fixed=inputs_fixed, inputs_vary=inputs_vary, outputs=outputs):
    print(f"Building model for {len(inputs_fixed[0])} fixed and {len(inputs_vary[0])} vary inputs")

    model = Sequential()

    model.add(Conv1D(filters=32, kernel_size=7, padding="same", input_shape=(None, 1)))
    #model.add(MaxPool1D(4))
    #model.add(Conv1D(filters=16, kernel_size=5, padding="same"))
    model.add(AveragePooling1D(4))
    model.add(Conv1D(filters=8, kernel_size=3, padding="same"))
    model.add(LSTM(4, return_sequences=True))
    model.add(GlobalAveragePooling1D())
    model.add(Dense(32, activation="tanh"))
    model.add(Dense(16, activation="tanh"))
    model.add(Dense(1, activation="sigmoid"))
    model.build()

    return model
"""

def buildModel(inputs_fixed=inputs_fixed, inputs_vary=inputs_vary, outputs=outputs):
    print(f"Building model for {len(inputs_fixed[0])} fixed and {len(inputs_vary[0])} vary inputs")


    inputB = Input(shape=(5,))
    y = Dense(5, activation="relu")(inputB)
    y = Model(inputs=inputB, outputs=y)


    # define two sets of inputs
    inputA = Input(shape=(None, 1))
    x = Conv1D(filters=32, kernel_size=7, padding="same")(inputA)
    x = AveragePooling1D(4)(x)
    x = Conv1D(filters=8, kernel_size=3, padding="same")(x)
    x = LSTM(4, return_sequences=True)(x)
    x = GlobalAveragePooling1D()(x)
    x = Dense(32, activation="tanh")(x)
    x = Model(inputs=inputA, outputs=x)

    # combine the output of the two branches
    combined = concatenate([y.output, x.output])

    # apply a FC layer and then a regression prediction on the
    # combined outputs
    #z = Dense(4, activation="relu")(combined)

    z = Dense(16, activation="tanh")(combined)
    z = Dense(1, activation="sigmoid")(z)
    # our model will accept the inputs of the two branches and
    # then output a single value
    model = Model(inputs=[y.input, x.input], outputs=z)

    return model




model = buildModel(inputs_fixed, inputs_vary, outputs)

import tensorflow as tf

model.save("sm")

# Convert the model.
converter = tf.lite.TFLiteConverter.from_saved_model("sm")
tflite_model = converter.convert()

# Save the model.
with open('model.tflite', 'wb') as f:
  f.write(tflite_model)


def trainModel(model=model, inputs_fixed=inputs_fixed, inputs_vary=inputs_vary, outputs=outputs):
    optimizer = SGD() if args.optimizer == "sgd" else Adam() if args.optimizer == "adam" else exit(0)
    model.compile(loss="mean_squared_logarithmic_error", optimizer=optimizer)
    model.summary()
    try:
        inse = numpy.expand_dims(inputs_vary, 2)

        tensorboard_callback = tf.keras.callbacks.TensorBoard(log_dir="./logs")

        model.fit([numpy.expand_dims(inputs_fixed, 2), inse], outputs, epochs=int(args.epochs), validation_split=0.1, verbose=1, shuffle=True, batch_size=args.batch, callbacks=[tensorboard_callback])
    except KeyboardInterrupt:
        print("interrupted fitting")


def saveModel(model=model):
    if args.output is None :
        args.output = re.sub("results/(.*?)(_[0-9]+)?\.log", "models/\\1", args.trainingDataset[0])
    print(f"Saving neural network model as {args.output}.h5")
    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    model.save(args.output + ".h5")
    keras2cpp.export_model(model, args.output + ".model")

def printSamples(model=model, inputs_fixed=inputs_fixed, inputs_vary=inputs_vary, outputs=outputs, numSamples=args.numSamples):
    for i in range(numSamples):
        i = random.randint(0, len(inputs_vary) - 1)
        actual = outputs[i][0]
        predicted = model.predict([inputs_fixed[i], inputs_vary[i]])[0][0]
        difference = actual-predicted
        percent_diff = difference/actual*100
        print("actual:", actual, "\tpredicted:", round(predicted, 2), "  \tdifference:", round(difference, 2), f"\t({round(percent_diff,2)}%)")


#saveModel()

trainModel()
saveModel()


if args.numSamples > 0 :
    printSamples()
