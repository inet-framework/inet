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
import tensorflow as tf
import keras.utils

from keras.models import Model, Sequential
from keras.layers import Activation, Dense, InputLayer, Concatenate, BatchNormalization, Conv1D, LSTM, GlobalAveragePooling1D, MaxPool1D, AveragePooling1D
from keras.optimizers import SGD, Adam, Adagrad
from PySide2.QtWidgets import QWidget, QSlider, QHBoxLayout, QSplitter, QScrollArea, QLabel, QApplication, QDialog, QLineEdit, QPushButton

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
inputs = None
outputs = None

def loadTrainingDatasetFile(filename, inputs, outputs):
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
            inputs.append(numpy.array(snirarray))
            outputs.append(numpy.array([packetErrorRate]))
            
def loadTrainingDataset(trainingDataset):
    inputs = list()
    outputs = list()
    for filename in trainingDataset:
        print(f"Loading training dataset from {filename}")
        loadTrainingDatasetFile(filename, inputs, outputs)
    return keras.utils.pad_sequences(inputs, dtype="float"), numpy.array(outputs)

inputs, outputs = loadTrainingDataset(args.trainingDataset)

def buildModel(inputs=inputs, outputs=outputs):
    print(f"Building model for {len(inputs[0])} inputs")
    model = Sequential()

    #model.add(Conv1D(filters=32, kernel_size=7, padding="same", input_shape=(None, 1)))
    #model.add(MaxPool1D(4))
    #model.add(Conv1D(filters=16, kernel_size=5, padding="same"))
    #model.add(AveragePooling1D(4))
    #model.add(Conv1D(filters=8, kernel_size=3, padding="same"))
    #model.add(LSTM(4, return_sequences=True))
    #model.add(GlobalAveragePooling1D())

    model.add(InputLayer(input_shape=(len(inputs[0]),), name="snirs"))
    model.add(Dense(32, activation="tanh"))
    model.add(Dense(16, activation="tanh"))
    model.add(Dense(1, activation="sigmoid"))
    model.build()
    return model

model = buildModel(inputs, outputs)

def trainModel(model=model, inputs=inputs, outputs=outputs):
    optimizer = SGD() if args.optimizer == "sgd" else Adam() if args.optimizer == "adam" else exit(0)
    model.compile(loss="mean_squared_logarithmic_error", optimizer=optimizer)
    model.summary()
    try:
        inse = numpy.expand_dims(inputs, 2)
        model.fit(inse, outputs, epochs=int(args.epochs), validation_split=0.1, verbose=1, shuffle=True, batch_size=args.batch)
    except KeyboardInterrupt:
        print("interrupted fitting")


def saveModel(model=model):
    if args.output is None :
        args.output = re.sub("results/(.*?)(_[0-9]+)?\.log", "models/\\1", args.trainingDataset[0])
    print(f"Saving neural network model as {args.output}.h5")
    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    model.save(args.output + ".h5")

    tflite_model = tf.lite.TFLiteConverter.from_keras_model(model).convert()
    with open(args.output + ".tflite", 'wb') as f:
        f.write(tflite_model)

def printSamples(model=model, inputs=inputs, outputs=outputs, numSamples=args.numSamples):
    for i in range(numSamples):
        i = random.randint(0, len(inputs) - 1)
        actual = outputs[i][0]
        predicted = model.predict(numpy.array([inputs[i]]))[0][0]
        difference = actual-predicted
        percent_diff = difference/actual*100
        print("actual:", actual, "\tpredicted:", round(predicted, 2), "  \tdifference:", round(difference, 2), f"\t({round(percent_diff,2)}%)")
      
def showGui(inputs=inputs, outputs=outputs):
    sliders = list()
    label = None

    def slide_handler(widget):
        print("puts, outputschanged")
        global label
        if not label:
            return
        inp = numpy.array([ [(s.value() / 1000.0 * 5) for s in sliders] ] )
        print("predicting for " + str(inp))
        outp = model.predict(inp)
        label.setText(str(outp[0][0]))

    class Form(QDialog):

        def __init__(self, parent=None):
            super(Form, self).__init__(parent)
            self.setWindowTitle("My Form")
    
            self.scrollArea = QScrollArea()
    
            # Create layout and add widgets
            layout = QHBoxLayout()
            self.sliders = list()
            for i in range(len(inputs[0])):
                slider = QSlider()
                slider.setMinimum(0)
                slider.setMaximum(1000)
                slider.setValue(500)
                slider.valueChanged.connect(slide_handler)
                layout.addWidget(slider)
    
                self.sliders.append(slider)
    
            global sliders
            sliders = self.sliders
    
            w = QWidget()
            w.setLayout(layout)
            # Set dialog layout
            self.scrollArea.setWidget(w)
            self.splitter = QSplitter()
            self.splitter.addWidget(self.scrollArea)
    
            # Create widgets
            self.label = QLabel("?")
            self.splitter.addWidget(self.label)
    
            global label
            label = self.label
    
            l = QHBoxLayout()
            l.addWidget(self.splitter)
    
            self.setLayout(l)
    
    # Create the Qt Application
    app = QApplication(sys.argv)
    # Create and show the form
    form = Form()
    form.show()
    # Run the main Qt loop
    sys.exit(app.exec_())

if args.interactive :
    print("You can use trainModel(), saveModel(), printSamples(), showGui(), etc.")
    code.interact(banner="", local=locals())
else :
    trainModel()
    saveModel()

if args.numSamples > 0 :
    printSamples()

if args.showGui :
    showGui()
