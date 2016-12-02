#!/usr/bin/python3

# vim: set et nosi ai ts=2 sts=2 sw=2:
# coding: utf-8

from __future__ import absolute_import, print_function, unicode_literals

import curses
import json
import os
import select
import subprocess
import sys


def debug(*args):
  pass


class MicrobitSimulator(object):
  def __init__(self):
    # Create a pipe to send client events and receive updates.
    self._client_events_pipe = os.pipe()
    self._device_updates_pipe = os.pipe()
    # Make the pipe inheritable (to the simulator process).
    os.set_inheritable(self._client_events_pipe[0], True)
    os.set_inheritable(self._device_updates_pipe[1], True)
    # The child subprocess (running the simulator binary).
    self._microbit_child_process = None
    # Queue of pending updates.
    self._updates_queue = []

  def start(self):
    try:
      env = {
          'GROK_CLIENT_PIPE': str(self._client_events_pipe[0]),
          'GROK_UPDATES_PIPE': str(self._device_updates_pipe[1])
      }
      # -t enables the heartbeat
      cmds = ['/home/ubuntu/repos/bbcmicrobit-micropython/build/x86-linux-native-32bit/source/microbit-micropython', 'program.py']
      self._microbit_child_process = subprocess.Popen(args=cmds, env=env, close_fds=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    except Exception as e:
      print('Unable to run simulator: ' + str(e) + '.', file=sys.stderr)

  def running(self):
    """
    Returns True if the simulator is still running.
    """
    return self._microbit_child_process.poll() is None

  def terminate(self):
    """
    Terminate the simulator (if it hasn't already exited).
    """
    if not self._microbit_child_process or self._microbit_child_process.poll() is not None:
      return

    self._microbit_child_process.kill()

  def event(self, event):
    """
    Send an event to the simulator.
    """
    event = '[' + json.dumps(event) + ']'
    debug('      > sending event: {}'.format(event))
    os.write(self._client_events_pipe[1], (event + '\n').encode(encoding='utf-8'))

  def next_update(self):
    if self._updates_queue:
      return self._updates_queue[0]
    r, w, x = select.select([self._device_updates_pipe[0]], [], [], 0.25)
    if not r:
      return
    lines = os.read(self._device_updates_pipe[0], 20000).decode().split('\n')
    for line in lines:
      if line:
        try:
          self._updates_queue.extend(json.loads(line))
        except Exception as e:
          print('Invalid json: ' + str(e), file=sys.stderr)
    if self._updates_queue:
      return self._updates_queue[0]
    else:
      return []

  def pop_update(self):
    return self._updates_queue.pop(0)

  def get_stdout(self):
    return self._microbit_child_process.stdout.read()


def main(stdscr):
  s = MicrobitSimulator()
  s.start()
  leds = [0]*25

  curses.init_pair(1, curses.COLOR_WHITE, curses.COLOR_WHITE)
  curses.init_pair(2, curses.COLOR_RED, curses.COLOR_RED)

  while True:
    stdscr.clear()

    for x in range(0, 5):
      for y in range(0, 5):
        if leds[y*5+x]:
          stdscr.addstr(3 + y*2, 10 + x*3, '  ', curses.color_pair(2))
        else:
          stdscr.addstr(3 + y*2, 10 + x*3, '  ', curses.color_pair(1))

    stdscr.addstr(7, 5, 'A')
    stdscr.addstr(7, 28, 'B')
    stdscr.addstr(14, 2, ' 0     1      2      3V    GND')
    stdscr.addstr(15, 2, '_[]____[]_____[]_____[]____[]_')

    stdscr.refresh()
    x = s.next_update()
    if x:
      if x['type'] == 'microbit_leds':
        leds = x['data']['b']
      s.pop_update()


if __name__ == '__main__':
  curses.wrapper(main)
