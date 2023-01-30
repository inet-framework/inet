#!/usr/bin/python3

import re
import os
import sys
import random
import argparse
import itertools

import importlib

sys.path.insert(1, os.environ["INET_ROOT"] + "/3rdparty/frugally-deep")

from keras_export.convert_model import convert as convert_model

os.environ['CUDA_VISIBLE_DEVICES'] = '' # make sure it runs in CPU

import numpy as np
import keras as keras
from keras.models import Sequential
from keras.layers import Dense, LSTM

from sklearn.metrics import mean_squared_error

BYTE_NR_INDEX = 8
TIME_NR_INDEX = 13
FREQ_NR_INDEX = 14
FREQUENCY_SIZE = 52
BATCH_SIZE = 8
NUM_EPOCHS = 1000

np.set_printoptions(threshold=sys.maxsize)

parser = argparse.ArgumentParser()
parser.add_argument("-o", "--output", default=None)
parser.add_argument("trainingDataset", nargs="+")
args = parser.parse_args()

"""
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
            snirarray = np.array([float(snir) for snir in symbolSnirMeans])
            snirarray = np.log(snirarray)
            inputs.append(np.array(snirarray))
            outputs.append(np.array([packetErrorRate]))
"""

# param is a list of csv filenames produced by the generator script
def loadTrainingDataset(trainingDataset):
    validation_data = ([], [])
    training_data_by_time_division = {}

    for file_path in trainingDataset:
        print(f"Loading training dataset from {file_path}")
        with open(file_path, "r") as f:
            for row in f.readlines()[1:]: # [1:] to skip header
                row = row.strip().split(",")
                error = float(row[1])

                # there are 20 columns of metadata
                snir_start = 20 + int(row[BYTE_NR_INDEX].strip().split()[0])
                snirs = np.array([float(p) for p in row[snir_start:-1]])

                freq_division = int(row[FREQ_NR_INDEX].strip().split()[0])
                time_division = int(row[TIME_NR_INDEX].strip().split()[0])

                assert(freq_division == FREQUENCY_SIZE)
                assert(len(snirs) == time_division * freq_division)

                snirs = snirs.reshape((time_division, freq_division))

                # put aside 10% of the data for validation
                if random.uniform(0, 1) < 0.1:
                    validation_data[0].append(snirs)
                    validation_data[1].append(error)
                else:
                    if time_division not in training_data_by_time_division:
                        training_data_by_time_division[time_division] = []

                    training_data_by_time_division[time_division].append((snirs, error))

    return validation_data, training_data_by_time_division

def buildModel():
    print(f"Building model")
    model = Sequential()

    model.add(LSTM(64, activation = 'tanh', return_sequences = True, input_shape = (None, FREQUENCY_SIZE)))
    model.add(LSTM(64, dropout = 0.2, recurrent_dropout = 0.2, activation = 'tanh'))
    model.add(Dense(10, activation = 'tanh'))
    model.add(Dense(5, activation = 'tanh'))
    model.add(Dense(1, activation = 'sigmoid'))

    model.compile(loss='mean_squared_error', optimizer="adam")
    model.summary()

    return model


validation_data, training_data_by_time_division = loadTrainingDataset(args.trainingDataset)
model = buildModel()

def trainModel():
    total_num_batches = 0
    for _td, xys in training_data_by_time_division.items():
        num_batches_with_same_td = len(xys) // BATCH_SIZE
        total_num_batches += num_batches_with_same_td

    print(total_num_batches, "batches")
    def train_generator():
        for _e in range(NUM_EPOCHS):
            for _td, xys in training_data_by_time_division.items():
                num_batches_with_same_td = len(xys) // BATCH_SIZE
                for i in range(num_batches_with_same_td):
                    batch = xys[i * BATCH_SIZE:(i + 1) * BATCH_SIZE]
                    # transposing a (BATCH_SIZE long) list of pairs
                    # into a pair of (BATCH_SIZE long) lists
                    xs, ys = zip(*batch)
                    yield np.array(xs), np.array(ys)

    print("Training model")
    model.fit(train_generator(), steps_per_epoch=total_num_batches, epochs=NUM_EPOCHS, verbose=1)

    pred_y = []
    actual_y = []

    print("Evaluating model")
    for x, y in zip(validation_data[0], validation_data[1]):
        predicted = model.predict(np.array([x]), verbose=0)[0][0]
        pred_y.append(predicted)
        actual_y.append(y)
        print(f"Predicted: {predicted}, Actual: {y}")
    rmse = np.sqrt(mean_squared_error(pred_y, actual_y))
    print("RMSE: ", rmse)

    try:
        import termplotlib as tpl
        fig = tpl.figure()
        fig.plot(actual_y, pred_y, width=50, height=24,
            plot_command="plot '-' with points pointtype '*'", # to show points instead of lines
            xlim=(-0.05, 1.05), ylim=(-0.05, 1.05),
            title="Predicted vs. Actual PER")
        fig.show()
    except Exception as e:
        print("Skipping terminal plot:", e)

    # import matplotlib.pyplot as plt
    # plt.scatter(actual_y, pred_y)
    # plt.show()


def saveModel(model=model):
    if args.output is None :
        args.output = re.sub("results/(.*?)(_[0-9]+)?\.log", "models/\\1", args.trainingDataset[0])
    print(f"Saving neural network model as {args.output}.h5")
    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    model.save(args.output + ".h5")
    convert_model(args.output + ".h5", args.output + ".json")



"""
def printSamples(model=model, inputs=inputs, outputs=outputs, numSamples=args.numSamples):
    for i in range(numSamples):
        i = random.randint(0, len(inputs) - 1)
        actual = outputs[i][0]
        predicted = model.predict(np.array([inputs[i]]))[0][0]
        difference = actual-predicted
        percent_diff = difference/actual*100
        print("actual:", actual, "\tpredicted:", round(predicted, 2), "  \tdifference:", round(difference, 2), f"\t({round(percent_diff,2)}%)")
"""

trainModel()
saveModel()
