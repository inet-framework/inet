#!/usr/bin/env python3
"""Capture local Mosquitto 3.1.1 exchanges through an unmodified TCP proxy.

Pass --prefix pointing at extracted packages (usr/bin and usr/sbin). Set
LD_LIBRARY_PATH if those binaries need the extracted libmosquitto. No root,
packet-capture capability, or production INET dependency is required.
"""
import argparse
import json
import pathlib
import socket
import subprocess
import threading
import time


def frames(data):
    result = []
    offset = 0
    while offset < len(data):
        end = offset + 1
        length = 0
        multiplier = 1
        while True:
            if end >= len(data):
                return result
            digit = data[end]
            end += 1
            length += (digit & 127) * multiplier
            if not digit & 128:
                break
            multiplier *= 128
            if multiplier > 128**4:
                raise ValueError("Invalid reference Remaining Length")
        if end + length > len(data):
            return result
        result.append(data[offset:end + length])
        offset = end + length
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prefix', type=pathlib.Path, required=True)
    parser.add_argument('--output', type=pathlib.Path, required=True)
    parser.add_argument('--vectors', type=pathlib.Path, required=True)
    args = parser.parse_args()
    broker_binary = args.prefix / 'usr/sbin/mosquitto'
    pub_binary = args.prefix / 'usr/bin/mosquitto_pub'
    sub_binary = args.prefix / 'usr/bin/mosquitto_sub'
    probe = socket.socket()
    probe.bind(('127.0.0.1', 0))
    broker_port = probe.getsockname()[1]
    probe.close()
    proxy = socket.socket()
    proxy.bind(('127.0.0.1', 0))
    proxy.listen()
    proxy.settimeout(0.1)
    proxy_port = proxy.getsockname()[1]
    stopping = threading.Event()
    streams = []
    clients = []
    lock = threading.Lock()

    def forward(source, target, stream, direction):
        try:
            while True:
                data = source.recv(65536)
                if not data:
                    break
                with lock:
                    stream[direction].extend(data)
                target.sendall(data)
        except OSError:
            pass
        finally:
            try:
                target.shutdown(socket.SHUT_WR)
            except OSError:
                pass

    def accept():
        while not stopping.is_set():
            try:
                client, _ = proxy.accept()
            except socket.timeout:
                continue
            except OSError:
                return
            server = socket.create_connection(('127.0.0.1', broker_port))
            stream = {'client': bytearray(), 'server': bytearray()}
            with lock:
                streams.append(stream)
            for source, target, direction in ((client, server, 'client'), (server, client, 'server')):
                threading.Thread(target=forward, args=(source, target, stream, direction), daemon=True).start()

    def wait_for(predicate):
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            if predicate():
                return
            stopping.wait(0.01)
        raise RuntimeError('Reference exchange timed out')

    broker = subprocess.Popen([str(broker_binary), '-p', str(broker_port)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    common = ['-V', 'mqttv311', '-h', '127.0.0.1', '-p', str(proxy_port)]
    commands = []
    try:
        def ready():
            try:
                with socket.create_connection(('127.0.0.1', broker_port), timeout=0.1):
                    return True
            except OSError:
                return False
        wait_for(ready)
        threading.Thread(target=accept, daemon=True).start()

        def subscribe(identifier, topic, persistent=False):
            with lock:
                before = len(streams)
            command = [str(sub_binary), *common, '-i', identifier, '-t', topic, '-q', '2', '-C', '1']
            if persistent:
                command.append('-c')
            commands.append(command)
            process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            clients.append(process)
            wait_for(lambda: len(streams) > before)
            with lock:
                stream = streams[before]
            wait_for(lambda: any(frame[0] >> 4 == 9 for frame in frames(bytes(stream['server']))))
            return process

        def publish(identifier, topic, payload, qos, retained=False):
            command = [str(pub_binary), *common, '-i', identifier, '-t', topic, '-m', payload, '-q', str(qos)]
            if retained:
                command.append('-r')
            commands.append(command)
            subprocess.run(command, check=True, capture_output=True, timeout=5)

        for qos in (0, 1, 2):
            subscriber = subscribe('subscriber' + str(qos), 'factory/normal')
            publish('publisher' + str(qos), 'factory/normal', 'A', qos)
            assert subscriber.communicate(timeout=5)[0] == b'A\n'
        publish('retainer', 'factory/retained', 'R', 1, True)
        subscriber = subscribe('lateSubscriber', 'factory/retained')
        assert subscriber.communicate(timeout=5)[0] == b'R\n'
        subscriber = subscribe('persistent', 'factory/offline', True)
        publish('primer', 'factory/offline', 'P', 1)
        assert subscriber.communicate(timeout=5)[0] == b'P\n'
        publish('offlinePublisher', 'factory/offline', 'Q', 1)
        subscriber = subscribe('persistent', 'factory/offline', True)
        assert subscriber.communicate(timeout=5)[0] == b'Q\n'
        version = subprocess.run([str(broker_binary), '-h'], capture_output=True, text=True, check=True).stdout.splitlines()[0]
        with lock:
            captured = [{direction: [frame.hex() for frame in frames(bytes(stream[direction]))] for direction in ('client', 'server')} for stream in streams]
        args.output.write_text(json.dumps({'version': version, 'mqttVersion': '3.1.1', 'commands': commands, 'connections': captured}, indent=2) + '\n')
        vectors = sorted({frame for stream in captured for direction in ('client', 'server') for frame in stream[direction]})
        args.vectors.write_text('// Generated by capture-reference.py from Mosquitto traffic.\n'
                                '// SPDX-License-Identifier: LGPL-3.0-or-later\n'
                                'static const char *mqttReferenceVectors[] = {\n' +
                                ''.join('    "' + frame + '",\n' for frame in vectors) + '};\n')
        print(f'{version}: {len(captured)} connections, {len(vectors)} distinct wire packets')
    finally:
        for process in clients:
            if process.poll() is None:
                process.terminate()
                process.wait(timeout=5)
        stopping.set()
        proxy.close()
        broker.terminate()
        broker.wait(timeout=5)


if __name__ == '__main__':
    main()
