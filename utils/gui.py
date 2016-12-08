#!/usr/bin/python3

# vim: set et nosi ai ts=2 sts=2 sw=2:
# coding: utf-8

from __future__ import absolute_import, print_function, unicode_literals

import curses
import fcntl
import json
import os
import select
import signal
import subprocess
import sys
import time


class MicrobitSimulator(object):
  def __init__(self, program_path=None, interactive=False):
    self._program_path = program_path
    self._interactive = interactive

    # Create a pipe to send client events and receive updates (must be inheritable by the simulator).
    self._client_events_pipe = os.pipe()
    self._device_updates_pipe = os.pipe()
    os.set_inheritable(self._client_events_pipe[0], True)
    os.set_inheritable(self._device_updates_pipe[1], True)

    # The child subprocess (running the simulator binary).
    self._microbit_child_process = None

    # Queue of pending updates.
    self._updates_queue = []

    self._ep = None

  def start(self):
    try:
      env = {
          'GROK_CLIENT_PIPE': str(self._client_events_pipe[0]),
          'GROK_UPDATES_PIPE': str(self._device_updates_pipe[1])
      }
      cmds = ['/home/ubuntu/repos/bbcmicrobit-micropython/build/x86-linux-native-32bit/source/microbit-micropython']
      if self._program_path:
        if self._interactive:
          cmds.append('-i')
        cmds.append(self._program_path)

      self._microbit_child_process = subprocess.Popen(args=cmds, env=env, close_fds=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

      # Non-blocking reads from the simulator -- just read whatever is available when we epoll.
      fcntl.fcntl(self._microbit_child_process.stdout, fcntl.F_SETFL, os.O_NONBLOCK)
      fcntl.fcntl(self._device_updates_pipe[0], fcntl.F_SETFL, os.O_NONBLOCK)

      # Create an epoll fd for simulator updates, simulator stdout, and local key events.
      self._ep = select.epoll()
      for fd in [self._device_updates_pipe[0], self._microbit_child_process.stdout, sys.stdin]:
        self._ep.register(fd)

    except Exception as e:
      print('Unable to run simulator: ' + str(e) + '.', file=sys.stderr)

  def running(self):
    # Returns True if the simulator is still running.
    return self._microbit_child_process.poll() is None

  def terminate(self):
    # Terminate the simulator (if it hasn't already exited).
    if not self._microbit_child_process or self._microbit_child_process.poll() is not None:
      return

    self._microbit_child_process.kill()

  def event(self, event):
    # Send an event to the simulator.
    event = '[' + json.dumps(event) + ']'
    os.write(self._client_events_pipe[1], (event + '\n').encode(encoding='utf-8'))

  def next_update(self):
    # The updates can be batched from the simulator, but only return one at a time from the queue.
    if self._updates_queue:
      return self._updates_queue.pop(0)

    #r_fds, w_fds, x_fds = select.select([self._device_updates_pipe[0], self._microbit_child_process.stdout, sys.stdin], [], [])
    events = self._ep.poll(timeout=2)
    for r, event_type in events:
      if r == self._device_updates_pipe[0]:
        lines = os.read(self._device_updates_pipe[0], 20000).decode().split('\n')
        for line in lines:
          if line:
            try:
              # Add the batch to the queue.
              self._updates_queue.extend(json.loads(line))
            except Exception as e:
              print('Invalid json: ' + str(e), file=sys.stderr)
      elif r == self._microbit_child_process.stdout.fileno():
        # Create a fake 'update' for stdout (serial) data.
        self._updates_queue.append({'type': 'stdout', 'data': self._microbit_child_process.stdout.read()})
      elif r == sys.stdin.fileno():
        # Create a fake 'update' for local key events.
        self._updates_queue.append({'type': 'input'})

    if self._updates_queue:
      # We got at least one update, return it.
      return self._updates_queue.pop(0)
    else:
      # Epoll timeout.
      return []

  def send_input(self, c):
    self._microbit_child_process.stdin.write(chr(c))
    self._microbit_child_process.stdin.flush()


class CursesUI:
  def __init__(self, stdscr, simulator):
    self._scr = stdscr
    self._sim = simulator

    # Allow Ctrl-C to be handled by us.
    curses.raw()

    # Off LED
    curses.init_pair(1, curses.COLOR_WHITE, curses.COLOR_WHITE)
    # On LED
    curses.init_pair(2, curses.COLOR_RED, curses.COLOR_RED)
    # Active border
    curses.init_pair(3, curses.COLOR_GREEN, curses.COLOR_BLACK)
    # Inactive border
    curses.init_pair(4, curses.COLOR_WHITE, curses.COLOR_BLUE)

    # Non-blocking getch().
    self._scr.nodelay(1)

    # Create a sub-window for the console, with a scrolling sub-sub-window.
    self._console = self._scr.subwin(16, 0)
    self._console_scroll = self._console.derwin(self._console.getmaxyx()[0]-2, self._console.getmaxyx()[1]-2, 1, 1)
    self._console_scroll.scrollok(True)

    # Create a sub-window for the micro:bit.
    self._microbit = self._scr.subwin(16, 34, 0, (self._scr.getmaxyx()[1]-34)//2)

    # micro:bit or console focus.
    self._microbit_focus = True

    # LED and button state.
    self._leds = [0]*25
    self._buttons = [0]*2

    # Draw initial UI.
    self.draw_buttons()
    self.draw_leds()
    self.focus_microbit()

  def write_console(self, c):
    # Writes serial output (or local echo) to the console window, scrolling as necessary.
    self._console_scroll.addstr(c)
    self._console_scroll.refresh()

  def draw_leds(self):
    for x in range(0, 5):
      for y in range(0, 5):
        if self._leds[y*5+x] >= 5:
          self._microbit.addstr(3 + y*2, 10 + x*3, '  ', curses.color_pair(2))
        else:
          self._microbit.addstr(3 + y*2, 10 + x*3, '  ', curses.color_pair(1))
    self._microbit.refresh()

  def update_leds(self, leds):
    self._leds = leds
    self.draw_leds()

  def draw_buttons(self):
    self._microbit.addstr(1, 12, 'micro:bit')

    if self._buttons[0]:
      self._microbit.attrset(curses.color_pair(4))
    self._microbit.addstr(7, 3, ' A ')
    self._microbit.attrset(curses.color_pair(0))

    if self._buttons[1]:
      self._microbit.attrset(curses.color_pair(4))
    self._microbit.addstr(7, 28, ' B ')
    self._microbit.attrset(curses.color_pair(0))

    self._microbit.addstr(13, 2, ' 0     1      2      3V    GND')
    self._microbit.addstr(14, 2, '_[]____[]_____[]_____[]____[]_')
    self._microbit.refresh()

  def focus_microbit(self):
    self._microbit_focus = True
    self._microbit.attrset(curses.color_pair(3))
    self._microbit.box()
    self._microbit.attrset(curses.color_pair(0))
    self._console.box()
    self._console.refresh()
    self._microbit.refresh()

  def focus_console(self):
    self._microbit_focus = False
    self._microbit.box()
    self._microbit.refresh()
    self._console.attrset(curses.color_pair(3))
    self._console.box()
    self._console.attrset(curses.color_pair(0))
    self._console.refresh()
    self._console_scroll.refresh()

  def push_button(self, b):
    self.toggle_button(b)
    time.sleep(0.2)
    self.toggle_button(b)

  def toggle_button(self, b):
    self._buttons[b] = 1 - self._buttons[b]
    self._sim.event({'type': 'microbit_button', 'data': {'id': b, 'state': self._buttons[b]}})
    self.draw_buttons()

  def show_help(self):
    self.write_console('\nKeyboard shortcuts:\n')
    self.write_console('  Ctrl-O  Toggle focus between micro:bit and console.\n')
    self.write_console('  Ctrl-Q  Quit simulator.\n')
    self.write_console('  Ctrl-C  Stop current micro:bit program.\n')
    self.write_console('Shortcuts when micro:bit has focus:\n')
    self.write_console('  A  Tap A button.\n')
    self.write_console('  B  Tap B button.\n')
    self.write_console('  Ctrl-A  Toggle A button.\n')
    self.write_console('  Ctrl-B  Toggle B button.\n')
    self.write_console('\n')

  def handle_key(self):
    k = self._scr.getch()
    if k < 0:
      return True

    # For debugging - show the key code in the top left corner.
    #self._scr.addstr(0, 0, str(k) + '   ')
    #self._scr.refresh()

    if k in (27, 17,):
      # In both modes, Escape and Ctrl-Q terminate.
      return False
    elif k == 3:
      # And Ctrl-C always gets sent.
      self._sim.send_input(3)
      return True
    elif k == 31:
      # Ctrl-?
      self.show_help()
      return True

    if self._microbit_focus:
      if k == 15:
        # Ctrl-O toggles console output.
        self.focus_console()
      if k == 1:
        # Ctrl-A
        self.toggle_button(0)
      elif k == 2:
        # Ctrl-B
        self.toggle_button(1)
      elif k == 97:
        # A
        self.push_button(0)
      elif k == 98:
        # B
        self.push_button(1)
    else:
      if k == 15:
        # Ctrl-O toggles console output.
        self.focus_microbit()
      elif k in (263, 330):
        # Delete and Backspace --> Ctrl-H
        self._sim.send_input(8)
      elif k in (258,):
        # Up arrow --> Ctrl-N (history up)
        self._sim.send_input(14)
      elif k in (259,):
        # Down arrow - Ctrl-P (history down)
        self._sim.send_input(16)
      elif k >= 0 and k <= 255:
        # Local echo.
        if k in (10, 13) or (k >= 32 and k<= 126):
          self.write_console(chr(k))
        # Anything else straight through.
        self._sim.send_input(k)

    return True


def main(stdscr):
  # Emulate CPython's behavior:
  #  - ./gui.py               - run REPL interactively.
  #  - ./gui.py program.py    - terminate when the program finishes (or Ctrl-C).
  #  - ./gui.py -i program.py - drop to REPL when program finishes (or Ctrl-C).
  program_path = None
  interactive = False
  if len(sys.argv) == 2:
    program_path = sys.argv[1]
  elif len(sys.argv) == 3:
    interactive = sys.argv[1] == '-i'
    program_path = sys.argv[2]

  s = MicrobitSimulator(program_path, interactive)
  s.start()

  ui = CursesUI(stdscr, s)

  ui.write_console('micro:bit simulator demo\n')
  ui.write_console('by Grok Learning -- https://groklearning.com/microbit\n')
  ui.write_console('Press Ctrl-? for help.\n\n')

  try:
    while s.running():
      # Keep waiting for:
      #  - Updates from the simulator
      #  - Stdout from the simulator
      #  - Keyboard events (stdin)
      update = s.next_update()
      if update:
        if update['type'] == 'microbit_leds':
          # Brightness array.
          ui.update_leds(update['data']['b'])
        elif update['type'] == 'microbit_pins':
          # Pin state/mode and PWM duty cycle & period.
          pass
        elif update['type'] == 'microbit_radio_tx':
          # Radio data sent from micro:bit.
          pass
        elif update['type'] == 'microbit_ack':
          # Acknowledgement of an event (e.g. button press).
          pass
        elif update['type'] == 'stdout':
          # Serial output.
          ui.write_console(update['data'])
        elif update['type'] == 'input':
          # stdin - use curses to get key event.
          if not ui.handle_key():
            break

  finally:
    # Send SIGKILL to the simulator.
    s.terminate()


if __name__ == '__main__':
  curses.wrapper(main)
