#!/usr/bin/env python3
import asyncio
import time

class time_measure(object):
    def __init__(self):
        self._start = time.perf_counter()

    def finish(self, iterations):
        end = time.perf_counter()
        seconds = end - self._start
        ns = seconds * 1000000000.0
        milliseconds = seconds * 1000.0
        iter_per_sec = iterations / seconds
        print('nanoseconds: {}'.format(int(round(ns))))
        print('milliseconds: {}'.format(milliseconds))
        print('seconds: {}'.format(seconds))
        print('iterations: {}'.format(iterations))
        print('per iteration: {} ns'.format(int(round(ns / iterations))))
        print('iterations per second: {}'.format(int(round(iter_per_sec))))

ITERATIONS = 500
CLIENTS = 100

async def handle_client(client_reader, client_writer):
    for i in range(ITERATIONS):
        buf = await client_reader.readexactly(1)
        client_writer.write(buf)
        await client_writer.drain()

async def run_client(reader, writer):
    data = b'a'
    for i in range(ITERATIONS):
        writer.write(data)
        await writer.drain()
        await reader.readexactly(1)

async def main(loop):
    server = await asyncio.start_server(handle_client, '127.0.0.1', 5530)

    readers = []
    writers = []
    for i in range(CLIENTS):
        reader, writer = await asyncio.open_connection('127.0.0.1', 5530)
        readers.append(reader)
        writers.append(writer)

    measure = time_measure()
    clients = []
    for c in range(CLIENTS):
        clients.append(loop.create_task(run_client(readers[c], writers[c])))
    for c in clients:
        await c
    measure.finish(ITERATIONS * CLIENTS)

    server.close()
    await server.wait_closed()

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main(loop))
