from evdev import InputDevice
from select import select



gamepad = InputDevice('/dev/input/event4')


while True:
    r, w, x = select([gamepad], [], [], 1.5)

    if r != []:
      for event in gamepad.read():
            print(event)



